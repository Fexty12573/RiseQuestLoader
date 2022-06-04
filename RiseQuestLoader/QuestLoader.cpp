#include "QuestLoader.h"

#include "reframework/API.hpp"

#include <imgui.h>
#include <MinHook.h>
#include <fmt/format.h>

#include <fstream>

using namespace reframework;
namespace fs = std::filesystem;

namespace utility {
void* get_class_info(const API::TypeDefinition* type) {
    const auto ptr = reinterpret_cast<uint64_t>(type->get_type_info()) + 0x58;
    return *reinterpret_cast<void**>(ptr);
}
int32_t& get_ref_count(const API::ManagedObject* obj) {
    const auto ptr = reinterpret_cast<uint64_t>(obj) + 0x8;
    return *reinterpret_cast<int32_t*>(ptr);
}
}

QuestLoader::QuestLoader() = default;

std::shared_ptr<QuestLoader> QuestLoader::get() {
    static auto instance = std::make_shared<QuestLoader>();
    return instance;
}

bool QuestLoader::initialize() {
    if (m_initialized) {
        return true;
    }

    MH_Initialize();

    const auto& api = API::get();

    m_quest_data = api->tdb()->find_type("snow.quest.QuestData");
    if (!m_quest_data) {
        return false;
    }

    m_normal_quest_data = api->tdb()->find_type("snow.quest.NormalQuestData.Param");
    if (!m_normal_quest_data) {
        return false;
    }

    m_normal_quest_data_for_enemy = api->tdb()->find_type("snow.quest.NormalQuestDataForEnemy.Param");
    if (!m_normal_quest_data_for_enemy) {
        return false;
    }

    m_rampage_data = api->tdb()->find_type("snow.quest.HyakuryuQuestData");
    if (!m_rampage_data) {
        return false;
    }

    if (!m_quest_exporter.initialize()) {
        return false;
    }

    if (!m_get_quest_text_hook) {
        if (const auto get_quest_text = m_quest_data->find_method("getQuestTextCore")) {
            const auto func = get_quest_text->get_function_raw();

            m_get_quest_text_hook = std::make_shared<utility::FunctionHook>(func, get_quest_text_hook);
            if (!m_get_quest_text_hook->is_valid()) {
                m_get_quest_text_hook.reset();
                return false;
            }

            m_get_quest_text_hook->create();
        } else {
            return false;
        }
    }

    const auto quest_manager = m_quest_exporter.get_quest_manager()->get_type_definition();

    if (!m_make_questno_list_hook) {
        if (const auto make_questno_list = quest_manager->find_method("makeQuestNoList")) {
            const auto func = make_questno_list->get_function_raw();

            m_make_questno_list_hook = std::make_shared<utility::FunctionHook>(func, make_questno_list_hook);
            if (!m_make_questno_list_hook->is_valid()) {
                m_make_questno_list_hook.reset();
                return false;
            }

            m_make_questno_list_hook->create();
        } else {
            return false;
        }
    }

    if (!m_make_quest_list_hyakuryu_hook) {
        if (const auto make_quest_list = quest_manager->find_method("makeQuestList_Hyakuryu")) {
            const auto func = make_quest_list->get_function_raw();

            m_make_quest_list_hyakuryu_hook = std::make_shared<utility::FunctionHook>(func, make_quest_list_hyakuryu_hook);
            if (!m_make_quest_list_hyakuryu_hook->is_valid()) {
                m_make_quest_list_hyakuryu_hook.reset();
                return false;
            }

            m_make_quest_list_hyakuryu_hook->create();
        } else {
            return false;
        }
    }

    if (!m_quest_counter_awake_hook) {
        const auto quest_counter = api->tdb()->find_type("snow.gui.fsm.questcounter.GuiQuestCounterFsmManager");
        if (!quest_counter) {
            return false;
        }

        if (const auto awake = quest_counter->find_method("awake")) {
            const auto func = awake->get_function_raw();

            m_quest_counter_awake_hook = std::make_shared<utility::FunctionHook>(func, quest_counter_awake_hook);
            if (!m_quest_counter_awake_hook->is_valid()) {
                m_quest_counter_awake_hook.reset();
                return false;
            }

            m_quest_counter_awake_hook->create();
        } else {
            return false;
        }
    }

    new_instance = reinterpret_cast<decltype(new_instance)>(0x142b87a30);

    m_initialized = true;
    return true;
}

void QuestLoader::read_quests() {
    for (const auto& entry : fs::directory_iterator("reframework/quests")) {
        if (entry.path().extension() == ".json") {
            parse_quest(entry);
        }
    }
}

void QuestLoader::render_ui() {
    if (!m_initialized) {
        if (!initialize()) {
            return;
        }
    }

    ImGui::Begin("QuestExporter");

    static int quest_id = 0;
    ImGui::InputInt("Quest ID", &quest_id);

    if (ImGui::Button("Export Quest")) {
        const auto q = m_quest_exporter.export_quest(quest_id);

        const std::filesystem::path path = "./reframework/plugins/quests";
        if (!exists(path)) {
            create_directories(path);
        }

        std::ofstream(fmt::format("./reframework/plugins/quests/q{}.json", quest_id)) << q.dump(4);
    }

    if (ImGui::Button("Export All Quests")) {
        const auto quests = m_quest_exporter.export_all_quests();

        const std::filesystem::path path = "./reframework/plugins/quests";
        if (!exists(path)) {
            create_directories(path);
        }

        for (const auto& q : quests) {
            std::ofstream(fmt::format("./reframework/plugins/quests/q{}.json", q.value("ID", 0))) << q.dump(4);
        }
    }

    if (ImGui::Button("Load Quests")) {
        read_quests();
    }

    ImGui::Checkbox("Skip Hook", &m_skip_hook);

    ImGui::End();
}

void QuestLoader::parse_quest(const std::filesystem::path& path) {
    const auto& api = API::get();
    nlohmann::json j{};
    bool is_rampage_quest = false;

    std::ifstream(path) >> j;


    if (!j.contains("ID")) {
        return;
    }

    const auto quest_id = j["ID"].get<int32_t>();
    
    const auto quest = new_instance(api->get_vm_context(), utility::get_class_info(m_quest_data), 0);
    if (quest == nullptr) {
        api->log_error("Failed to create quest instance");
        return;
    }

    const auto qnormal = new_instance(api->get_vm_context(), utility::get_class_info(m_normal_quest_data), 0);
    if (qnormal == nullptr) {
        api->log_error("Failed to create NormalQuestData instance");
        return;
    }

    const auto qenemy = new_instance(api->get_vm_context(), utility::get_class_info(m_normal_quest_data_for_enemy), 0);
    if (qenemy == nullptr) {
        api->log_error("Failed to create NormalQuestDataForEnemy instance");
        return;
    }

    const auto qrampage = new_instance(api->get_vm_context(), utility::get_class_info(m_rampage_data), 0);
    if (qrampage == nullptr) {
        api->log_error("Failed to create RampageData instance");
        return;
    }

    quest->add_ref();
    qnormal->add_ref();
    qenemy->add_ref();
    qrampage->add_ref();

    if (j.contains("QuestData")) {
        const auto& normal = j["QuestData"];

        *qnormal->get_field<uint32_t>("_QuestNo") = quest_id;
        *qnormal->get_field<uint32_t>("_QuestType") = normal["type"];
        *qnormal->get_field<int32_t>("_QuestLv") = normal["level"];
        *qnormal->get_field<int32_t>("_EnemyLv") = normal["enemylevel"];
        *qnormal->get_field<int32_t>("_MapNo") = normal["map"];
        *qnormal->get_field<uint32_t>("_BaseTime") = normal["quest_time"];
        *qnormal->get_field<int32_t>("_TimeVariation") = normal["time_variation"];
        *qnormal->get_field<int32_t>("_TimeLimit") = normal["time_limit"];
        *qnormal->get_field<int32_t>("_QuestLife") = normal["carts"];
        
        if (const auto conditions = qnormal->get_field<API::ManagedObject*>("_OrderType")) {
            *conditions = utility::create_managed_array("snow.quest.QuestOrderType", normal["conditions"].size());
            (*conditions)->add_ref();

            for (auto i = 0u; i < normal["conditions"].size(); ++i) {
                utility::call(*conditions, "Set", i, normal["conditions"][i].get<int32_t>());
            }
        }

        if (const auto targets = qnormal->get_field<API::ManagedObject*>("_TargetType")) {
            *targets = utility::create_managed_array("snow.quest.QuestTargetType", normal["targets"].size());
            (*targets)->add_ref();

            for (auto i = 0u; i < normal["targets"].size(); ++i) {
                utility::call(*targets, "Set", i, normal["targets"][i].get<uint8_t>());
            }
        }

        if (const auto targets = qnormal->get_field<API::ManagedObject*>("_TgtEmType")) {
            *targets = utility::create_managed_array("snow.enemy.EnemyDef.EmTypes", normal["target_monsters"].size());
            (*targets)->add_ref();

            for (auto i = 0u; i < normal["target_monsters"].size(); ++i) {
                utility::call(*targets, "Set", i, normal["target_monsters"][i].get<uint32_t>());
            }
        }

        if (const auto items = qnormal->get_field<API::ManagedObject*>("_TgtItemId")) {
            *items = utility::create_managed_array("snow.data.ContentsIdSystem.ItemId", normal["target_items"].size());
            (*items)->add_ref();

            for (auto i = 0u; i < normal["target_items"].size(); ++i) {
                utility::call(*items, "Set", i, normal["target_items"][i].get<uint32_t>());
            }
        }

        if (const auto counts = qnormal->get_field<API::ManagedObject*>("_TgtNum")) {
            *counts = utility::create_managed_array("System.UInt32", normal["amount"].size());
            (*counts)->add_ref();

            for (auto i = 0u; i < normal["amount"].size(); ++i) {
                utility::call(*counts, "Set", i, normal["amount"][i].get<uint32_t>());
            }
        }

        if (const auto monsters = qnormal->get_field<API::ManagedObject*>("_BossEmType")) {
            if (const auto conds = qnormal->get_field<API::ManagedObject*>("_BossSetCondition")) {
                if (const auto params = qnormal->get_field<API::ManagedObject*>("_BossSetParam")) {
                    constexpr uint32_t max_monster_count = 7;

                    *monsters = utility::create_managed_array("snow.enemy.EnemyDef.EmTypes", max_monster_count);
                    *conds = utility::create_managed_array("snow.QuestManager.BossSetCondition", max_monster_count);
                    *params = utility::create_managed_array("System.UInt32", max_monster_count);

                    (*monsters)->add_ref();
                    (*conds)->add_ref();
                    (*params)->add_ref();

                    for (auto i = 0u; i < max_monster_count; ++i) {
                        const auto& mon = normal["monsters"][i];

                        utility::call(*monsters, "Set", i, mon["id"].get<uint32_t>());
                        utility::call(*conds, "Set", i, mon["spawn_condition"].get<uint32_t>());
                        utility::call(*params, "Set", i, mon["param"].get<uint32_t>());
                    }
                }
            }
        }

        *qnormal->get_field<uint8_t>("_InitExtraEmNum") = normal["extra_monster_count"];
        *qnormal->get_field<bool>("_IsSwapExitMarionette") = normal["swap_exit_ride"];

        if (const auto rates = qnormal->get_field<API::ManagedObject*>("_SwapEmRate")) {
            *rates = utility::create_managed_array("System.Byte", normal["swap_frequency"].size());
            (*rates)->add_ref();

            for (auto i = 0u; i < normal["swap_frequency"].size(); ++i) {
                utility::call(*rates, "Set", i, normal["swap_frequency"][i].get<uint8_t>());
            }
        }

        if (const auto conds = qnormal->get_field<API::ManagedObject*>("_SwapSetCondition")) {
            *conds = utility::create_managed_array("snow.QuestManager.SwapSetCondition", normal["swap_condition"].size());
            (*conds)->add_ref();

            for (auto i = 0u; i < normal["swap_condition"].size(); ++i) {
                utility::call(*conds, "Set", i, normal["swap_condition"][i].get<uint32_t>());
            }
        }

        if (const auto params = qnormal->get_field<API::ManagedObject*>("_SwapSetParam")) {
            *params = utility::create_managed_array("System.Byte", normal["swap_param"].size());
            (*params)->add_ref();

            for (auto i = 0u; i < normal["swap_param"].size(); ++i) {
                utility::call(*params, "Set", i, normal["swap_param"][i].get<uint8_t>());
            }
        }

        if (const auto counts = qnormal->get_field<API::ManagedObject*>("_SwapExitTime")) {
            *counts = utility::create_managed_array("System.Byte", normal["swap_exit_time"].size());
            (*counts)->add_ref();

            for (auto i = 0u; i < normal["swap_exit_time"].size(); ++i) {
                utility::call(*counts, "Set", i, normal["swap_exit_time"][i].get<uint8_t>());
            }
        }

        *qnormal->get_field<uint32_t>("_SwapStopType") = normal["swap_stop_type"];
        *qnormal->get_field<uint8_t>("_SwapStopParam") = normal["swap_stop_param"];
        *qnormal->get_field<uint32_t>("_SwapExecType") = normal["swap_exec_type"];

        *qnormal->get_field<uint32_t>("_RemMoney") = normal["reward"]["zenny"];
        *qnormal->get_field<uint32_t>("_RemVillagePoint") = normal["reward"]["points"];
        *qnormal->get_field<uint32_t>("_RemRankPoint") = normal["reward"]["hrp"];

        *qnormal->get_field<uint32_t>("_SupplyTbl") = normal["supply_id"];

        if (const auto icons = qnormal->get_field<API::ManagedObject*>("_Icon")) {
            constexpr uint32_t quest_icon_count = 5;

            *icons = utility::create_managed_array("snow.gui.SnowGuiCommonUtility.Icon.EnemyIconFrameForQuestOrder", quest_icon_count);
            (*icons)->add_ref();

            for (auto i = 0u; i < quest_icon_count; ++i) {
                utility::call(*icons, "Set", i, normal["monster_icons"][i].get<int32_t>());
            }
        }

        *qnormal->get_field<bool>("_IsTutorial") = normal["tutorial"];

        *qnormal->get_field<bool>("_FenceDefaultActive") = normal["arena_param"]["fence_default_active"];
        *qnormal->get_field<uint16_t>("_FenceActiveSec") = normal["arena_param"]["fence_uptime"];
        *qnormal->get_field<uint16_t>("_FenceDefaultWaitSec") = normal["arena_param"]["fence_initial_delay"];
        *qnormal->get_field<uint16_t>("_FenceReloadSec") = normal["arena_param"]["fence_cooldown"];

        if (const auto pillars = qnormal->get_field<API::ManagedObject*>("_IsUsePillar")) {
            constexpr uint32_t arena_pillar_count = 3;

            *pillars = utility::create_managed_array("System.Boolean", arena_pillar_count);
            (*pillars)->add_ref();

            for (auto i = 0u; i < arena_pillar_count; ++i) {
                utility::call(*pillars, "Set", i, normal["arena_param"]["pillars_active"][i].get<bool>());
            }
        }

        *qnormal->get_field<uint16_t>("_AutoMatchHR") = normal["auto_match_hr"];
        *qnormal->get_field<int32_t>("_BattleBGMType") = normal["battle_bgm_type"];
        *qnormal->get_field<int32_t>("_ClearBGMType") = normal["clear_bgm_type"];
    }

    if (j.contains("EnemyData")) {
        const auto& enemy = j["EnemyData"];

        *qnormal->get_field<uint32_t>("_QuestNo") = quest_id;
        *qenemy->get_field<int32_t>("_EmsSetNo") = enemy["small_monsters"]["spawn_type"];
        *qenemy->get_field<uint8_t>("_ZakoVital") = enemy["small_monsters"]["hp"];
        *qenemy->get_field<uint8_t>("_ZakoAttack") = enemy["small_monsters"]["attack"];
        *qenemy->get_field<uint8_t>("_ZakoParts") = enemy["small_monsters"]["part_hp"];
        *qenemy->get_field<uint8_t>("_ZakoOther") = enemy["small_monsters"]["other"];
        *qenemy->get_field<uint8_t>("_ZakoMulti") = enemy["small_monsters"]["multi"];

        std::vector tables = {
            qenemy->get_field<API::ManagedObject*>("_RouteNo"), qenemy->get_field<API::ManagedObject*>("_InitSetName"),
            qenemy->get_field<API::ManagedObject*>("_SubType"), qenemy->get_field<API::ManagedObject*>("_VitalTbl"),
            qenemy->get_field<API::ManagedObject*>("_AttackTbl"), qenemy->get_field<API::ManagedObject*>("_OtherTbl"),
            qenemy->get_field<API::ManagedObject*>("_StaminaTbl"), qenemy->get_field<API::ManagedObject*>("_Scale"),
            qenemy->get_field<API::ManagedObject*>("_ScaleTbl"), qenemy->get_field<API::ManagedObject*>("_Difficulty"),
            qenemy->get_field<API::ManagedObject*>("_BossMulti")
        };

        if (std::all_of(tables.begin(), tables.end(), [](auto** obj) { return obj != nullptr; })) {
            constexpr uint32_t max_monster_count = 7;

            *tables[0] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[1] = utility::create_managed_array("System.String", max_monster_count);
            *tables[2] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[3] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[4] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[5] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[6] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[7] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[8] = utility::create_managed_array("snow.enemy.EnemyDef.BossScaleTblType", max_monster_count);
            *tables[9] = utility::create_managed_array("snow.enemy.EnemyDef.NandoYuragi", max_monster_count);
            *tables[10] = utility::create_managed_array("System.Byte", max_monster_count);

            std::for_each(tables.begin(), tables.end(), [](auto** obj) { (*obj)->add_ref(); });

            for (auto i = 0u; i < max_monster_count; ++i) {
                const auto& mon = enemy["monsters"][i];

                utility::call(*tables[0], "Set", i, mon["path_id"].get<uint8_t>());

                if (const auto str = utility::create_managed_string(mon["set_name"].get<std::string>())) {
                    reinterpret_cast<API::ManagedObject*>(str)->add_ref();
                    utility::call(*tables[1], "Set", i, str);
                }

                utility::call(*tables[2], "Set", i, mon["sub_type"].get<uint8_t>());
                utility::call(*tables[3], "Set", i, mon["hp"].get<uint8_t>());
                utility::call(*tables[4], "Set", i, mon["attack"].get<uint8_t>());
                utility::call(*tables[5], "Set", i, mon["other"].get<uint8_t>());
                utility::call(*tables[6], "Set", i, mon["stamina"].get<uint8_t>());
                utility::call(*tables[7], "Set", i, mon["size"].get<uint8_t>());
                utility::call(*tables[8], "Set", i, mon["scale_type"].get<int32_t>());
                utility::call(*tables[9], "Set", i, mon["difficulty"].get<int32_t>());
                utility::call(*tables[10], "Set", i, mon["multi"].get<uint8_t>());
            }
        }
    }

    if (j.contains("RampageData")) {
        const auto& rampage = j["RampageData"];

        if (rampage.contains("seed")) {
            is_rampage_quest = true;

            *qnormal->get_field<uint32_t>("_QuestNo") = quest_id;
            *qrampage->get_field<int32_t>("_RandomSeed") = rampage["seed"];
            *qrampage->get_field<uint8_t>("_Attr") = rampage["attributes"];

            if (const auto waves = qrampage->get_field<API::ManagedObject*>("_WaveData")) {
                *waves = utility::create_managed_array("snow.quest.HyakuryuQuestData.WaveData", rampage["waves"].size());
                (*waves)->add_ref();

                for (auto i = 0u; i < rampage["waves"].size(); ++i) {
                    if (const auto qwave = utility::call(*waves, "Get", i)) {
                        const auto& wave = rampage["waves"][i];

                        *qwave->get_field<uint32_t>("_BossEm") = wave["boss_monster"];
                        *qwave->get_field<uint32_t>("_BossSubType") = wave["boss_subtype"];
                        *qwave->get_field<int32_t>("_OrderTblNo") = wave["order_table"];
                        *qwave->get_field<int32_t>("_BossEmNandoTblNo") = wave["boss_em_nando_table"];
                        *qwave->get_field<int32_t>("_WaveEmNandoTblNo") = wave["wave_em_nando_table"];

                        if (const auto monsters = qwave->get_field<API::ManagedObject*>("_EmTable")) {
                            *monsters = utility::create_managed_array("snow.enemy.EnemyDef.EmTypes", wave["monsters"].size());
                            (*monsters)->add_ref();

                            for (auto k = 0u; k < wave["monsters"].size(); ++k) {
                                utility::call(*monsters, "Set", k, wave["monsters"][k].get<uint32_t>());
                            }
                        }
                    }
                }
            }

            *qrampage->get_field<int32_t>("_QuestLv") = rampage["level"];
            *qrampage->get_field<int32_t>("_MapNo") = rampage["map"];
            *qrampage->get_field<uint8_t>("_Category") = rampage["category"];
            *qrampage->get_field<bool>("_IsVillage") = rampage["village"];
            *qrampage->get_field<uint8_t>("_BaseTime") = rampage["base_time"];
            *qrampage->get_field<uint8_t>("_StartBlockNo") = rampage["start_block"];
            *qrampage->get_field<uint8_t>("_EndBlockNo") = rampage["end_block"];
            *qrampage->get_field<uint8_t>("_ExtraEmWaveNo") = rampage["extra_wave_count"];
            *qrampage->get_field<int8_t>("_ExtraEmNandoTblNo") = rampage["extra_em_nando_table"];
            *qrampage->get_field<uint8_t>("_NushiOrderTblNo") = rampage["apex_order_table"];
            *qrampage->get_field<uint8_t>("_HmUnlockTblNo") = rampage["weapon_unlock_table"];

            if (const auto subtargets = qrampage->get_field<API::ManagedObject*>("_SubTarget")) {
                *subtargets = utility::create_managed_array("snow.quest.QuestTargetType", rampage["sub_targets"].size());
                (*subtargets)->add_ref();

                for (auto i = 0u; i < rampage["sub_targets"].size(); ++i) {
                    utility::call(*subtargets, "Set", i, rampage["sub_targets"][i].get<uint8_t>());
                }
            }

            *qrampage->get_field<uint8_t>("_SubTarget5WaveNo") = rampage["subtarget_5_wave"];
        }
    }

    utility::call(quest, "set_RawNormal", qnormal);
    utility::call(quest, "set_RawEnemy", qenemy);

    if (is_rampage_quest) {
        utility::call(quest, "set_Hyakuryu", qrampage);
    } else {
        qrampage->release();
    }

    const auto quest_manager = m_quest_exporter.get_quest_manager();
    const auto questdict = *quest_manager->get_field<API::ManagedObject*>("_QuestDataDictionary");
    if (utility::call<bool>(questdict, "ContainsKey", quest_id)) {
        utility::call(questdict, "Insert", quest_id, quest, false);
    } else {
        m_custom_quests[quest_id] = {j, quest};
        utility::call(questdict, "Add", quest_id, quest);
    }
}

SystemString* QuestLoader::get_quest_text_hook(void* vmctx, API::ManagedObject* this_, QuestText type, void* qi) {
    const auto quest_id = utility::call<int32_t>(this_, "getQuestNo");
    const auto loader = get();


    if (loader->m_custom_quests.count(quest_id)) {
        switch (type) {
        case QuestText::TITLE:
            return utility::create_managed_string(loader->m_custom_quests[quest_id].m_name);
        case QuestText::CLIENT:
            return utility::create_managed_string(loader->m_custom_quests[quest_id].m_client); 
        case QuestText::REQUEST:
            return utility::create_managed_string(loader->m_custom_quests[quest_id].m_description);
        case QuestText::TARGET:
            return utility::create_managed_string(loader->m_custom_quests[quest_id].m_target);
        default: 
            return loader->m_get_quest_text_hook->call_original<SystemString*>(vmctx, this_, type, qi);
        }
    }

    return loader->m_get_quest_text_hook->call_original<SystemString*>(vmctx, this_, type, qi);
}

API::ManagedObject* QuestLoader::make_questno_list_hook(void* vmctx, API::ManagedObject* this_, API::ManagedObject* src, bool is_quick_match) {
    const auto loader = get();
    const auto list = loader->m_make_questno_list_hook->call_original<API::ManagedObject*>(vmctx, this_, src, is_quick_match);

    if (loader->m_skip_hook) {
        return list;
    }

    const auto type = utility::call<QuestCounterTopMenuType>(loader->m_quest_counter, "getQuestCounterSelectedTopMenu()");
    const auto level = utility::call<QuestCounterLevelMenuType>(loader->m_quest_counter, "getQuestCounterSelectedLevelMenu");
    const auto quest_level = utility::call<QuestLevel>(loader->m_quest_counter, "convertLeveLMenuToQuestLevel", level);

    for (const auto& [id, quest] : loader->m_custom_quests) {
        if (quest_level == utility::call<QuestLevel>(quest.m_memory_object, "getQuestLevel")) {
            const auto quest_type = utility::call<QuestType>(quest.m_memory_object, "getQuestType");
            bool add_quest = false;

            if (type == QuestCounterTopMenuType::Normal_Hall_Low || type == QuestCounterTopMenuType::Normal_Hall_High) {
                if (quest_type & QuestType::KILL || quest_type & QuestType::HUNTING || 
                    quest_type & QuestType::CAPTURE || quest_type & QuestType::COLLECTS) {
                    add_quest = true;
                }
            } else if (type == QuestCounterTopMenuType::Training) {
                if (quest_type & QuestType::TRAINING) {
                    add_quest = true;
                }
            } else if (type == QuestCounterTopMenuType::Arena) {
                if (quest_type & QuestType::ARENA) {
                    add_quest = true;
                }
            }

            if (add_quest) {
                utility::call(list, "Add", id);
            }
        }
    }

    return list;
}

API::ManagedObject* QuestLoader::make_quest_list_hyakuryu_hook(void* vmctx, API::ManagedObject* this_, API::ManagedObject* src, bool is_village, const int rank) {
    const auto loader = get();
    const auto list = loader->m_make_quest_list_hyakuryu_hook->call_original<API::ManagedObject*>(vmctx, this_, src, is_village, rank);

    if (!is_village) {
        for (const auto& [id, quest] : loader->m_custom_quests) {
            bool add_quest = false;
            if (utility::call<QuestType>(quest.m_memory_object, "getQuestType") & QuestType::HYAKURYU) {
                const auto quest_level = utility::call<QuestLevel>(quest.m_memory_object, "getQuestLevel");

                if (rank == 0) { // High
                    if (quest_level >= QuestLevel::QL_H_HIGH_START) {
                        add_quest = true;
                    }
                } else { // Low
                    if (quest_level <= QuestLevel::QL_H_LOW_END) {
                        add_quest = true;
                    }
                }
            }

            if (add_quest) {
                utility::call(list, "Add", quest.m_memory_object);
            }
        }
    }

    return list;
}

void QuestLoader::quest_counter_awake_hook(void* vmctx, reframework::API::ManagedObject* this_) {
    const auto loader = get();
    loader->m_quest_counter = this_;

    return loader->m_quest_counter_awake_hook->call_original<void>(vmctx, this_);
}