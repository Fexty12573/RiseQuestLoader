#include "QuestLoader.h"
#include "Language.h"
#include "Plugin.h"

#include "reframework/API.hpp"

#include <imgui.h>
#include <MinHook.h>
#include <fmt/format.h>

#include <fstream>

using namespace reframework;
namespace fs = std::filesystem;

using ManagedObject = reframework::API::ManagedObject;

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

    if (!m_init_quest_data_dict_hook) {
        if (const auto init = quest_manager->find_method("initQuestDataDictionary")) {
            const auto func = init->get_function_raw();

            m_init_quest_data_dict_hook = std::make_shared<utility::FunctionHook>(func, init_quest_data_dict_hook);
            if (!m_init_quest_data_dict_hook) {
                m_init_quest_data_dict_hook.reset();
                return false;
            }

            m_init_quest_data_dict_hook->create();
        } else {
            return false;
        }
    }
    // 0f 85 b0 00 00 00 f6 42 13
    auto results = utility::scanmem({0x0f, 0x85, 0xb0, 0x00, 0x00, 0x00, 0xf6, 0x42, 0x13, 0x01});
    if (results.empty()) {
        api->log_error("[QuestLoader] Failed to find object allocator");
        OutputDebugStringA("[QuestLoader] Failed to find object allocator");
        return false;
    }

    void* result = static_cast<byte*>(results[0]) - 0x16;
    api->log_info("[QuestLoader] Found object allocator at {:p}", result);
    utility::log(fmt::format("[QuestLoader] Found object allocator at {:p}", result));

    new_instance = static_cast<decltype(new_instance)>(result);

    if (!m_get_message_hook) {
        // 4d 8b 4c c8 38 85 d2 74 10 4d 8b 40 20 41 39 3c 80
        results = utility::scanmem({0x4D, 0x8B, 0x4C, 0xC8, 0x38, 0x85, 0xD2, 0x74, 0x10, 0x4D, 0x8B, 0x40, 0x20, 0x41, 0x39, 0x3C, 0x80});
        if (results.empty()) {
            const auto msg = fmt::format("Failed to find via::gui::MessageManager::getMessage");
            api->log_error(msg.c_str());
            utility::log(msg);

            return false;
        }

        result = static_cast<byte*>(results[0]) - 0x39;

        const auto msg = fmt::format("Found via::gui::MessageManager::getMessage at {:p}", result);
        api->log_error(msg.c_str());
        utility::log(msg);

        m_get_message_hook = std::make_shared<utility::FunctionHook>(result, get_message_hook);
        if (!m_get_message_hook) {
            m_get_message_hook.reset();
            return false;
        }

        m_get_message_hook->create();
    }

    m_initialized = true;
    return true;
}

void QuestLoader::read_quests() {
    const auto& api = API::get();
    if (!fs::exists("reframework/quests")) {
        fs::create_directories("reframework/quests");
    }

    for (const auto& entry : fs::directory_iterator("reframework/quests")) {
        if (entry.path().extension() == ".json") {
            try {
                parse_quest(entry);
            } catch (const std::exception& e) {
                api->log_error("C++ Exception Thrown: {}", e.what());
            }
        }
    }
}

void QuestLoader::render_ui() {
    if (!m_initialized) {
        if (!initialize()) {
            return;
        }
    }

    if (API::get()->reframework()->is_drawing_ui()) {
        ImGui::Begin("Quest Loader");

        if (ImGui::TreeNode("Quest Exporter")) {
            static int quest_id = 0;
            ImGui::InputInt("Quest ID", &quest_id);

            ImGui::SameLine();
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
                    std::ofstream(fmt::format("./reframework/plugins/quests/q{}.json", q.value("QuestID", 0))) << q.dump(4);
                }
            }

            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Quest Loader")) {
            const auto quest_manager = m_quest_exporter.get_quest_manager();
            const auto questdict = *quest_manager->get_field<ManagedObject*>("_QuestDataDictionary");

            if (ImGui::Button("Reload Quests")) {
                for (auto& quest : m_custom_quests) {
                    quest.second.cleanup(questdict);
                }

                m_custom_quests.clear();
                read_quests();
            }

            if (ImGui::TreeNode("Replacement Quests")) {
                for (auto& [id, quest] : m_custom_quests) {
                    if (quest.m_is_replacement) {
                        if (ImGui::Checkbox(fmt::format("[{}] {}", id, quest.get_quest_info().m_name).c_str(), &quest.m_enabled)) {
                            if (quest.m_enabled) {
                                quest.enable(questdict);
                            } else {
                                quest.disable(questdict);
                            }
                        }
                    }
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Custom Quests")) {
                std::vector<int> to_erase{};
                for (auto& [id, quest] : m_custom_quests) {
                    if (!quest.m_is_replacement) {
                        ImGui::PushID(id);
                        if (ImGui::Button("Unload")) {
                            to_erase.emplace_back(id);
                        }

                        ImGui::SameLine();
                        ImGui::Text("[%d] %s", id, quest.get_quest_info().m_name.c_str());
                    }
                }

                for (const auto& id : to_erase) {
                    m_custom_quests[id].cleanup(questdict);
                    m_custom_quests.erase(id);
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Debug")) {
            ImGui::Checkbox("Skip Hook", &m_skip_hook);
            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void QuestLoader::parse_quest(const std::filesystem::path& path) {
    const auto& api = API::get();
    nlohmann::json j{};
    bool is_rampage_quest = false;
    const auto quest_manager = m_quest_exporter.get_quest_manager();
    const auto questdict = *quest_manager->get_field<ManagedObject*>("_QuestDataDictionary");

    try {
        std::ifstream(path) >> j;
    } catch (const std::exception& e) {
        api->log_error("Failed to load quest: {}", e.what());
    }

    if (!j.contains("QuestID")) {
        return;
    }

    const auto quest_id = j["QuestID"].get<int32_t>();

    utility::log(fmt::format("Loading Quest {}", quest_id));

    if (const auto it = m_custom_quests.find(quest_id); it != m_custom_quests.end()) {
        it->second.cleanup(questdict);
        m_custom_quests.erase(it);
    }
    
    const auto quest = new_instance(api->get_vm_context(), m_quest_data, 0);
    if (quest == nullptr) {
        api->log_error("Failed to create quest instance");
        return;
    }

    const auto qnormal = new_instance(api->get_vm_context(), m_normal_quest_data, 0);
    if (qnormal == nullptr) {
        api->log_error("Failed to create NormalQuestData instance");
        return;
    }

    const auto qenemy = new_instance(api->get_vm_context(), m_normal_quest_data_for_enemy, 0);
    if (qenemy == nullptr) {
        api->log_error("Failed to create NormalQuestDataForEnemy instance");
        return;
    }

    const auto qrampage = new_instance(api->get_vm_context(), m_rampage_data, 0);
    if (qrampage == nullptr) {
        api->log_error("Failed to create RampageData instance");
        return;
    }

    quest->add_ref();
    qnormal->add_ref();
    qenemy->add_ref();
    qrampage->add_ref();

    *qnormal->get_field<SystemString*>("_DbgName") = utility::create_managed_string(j["QuestText"]["DebugName"]);
    *qnormal->get_field<SystemString*>("_DbgClient") = utility::create_managed_string(j["QuestText"]["DebugClient"]);
    *qnormal->get_field<SystemString*>("_DbgContent") = utility::create_managed_string(j["QuestText"]["DebugDescription"]);

    if (j.contains("QuestData") && j["QuestData"].type() != nlohmann::detail::value_t::null) {
        const auto& normal = j["QuestData"];

        *qnormal->get_field<uint32_t>("_QuestNo") = quest_id;
        *qnormal->get_field<uint32_t>("_QuestType") = normal["QuestType"];
        *qnormal->get_field<int32_t>("_QuestLv") = normal["QuestLevel"];
        *qnormal->get_field<int32_t>("_EnemyLv") = normal["EnemyLevel"];
        *qnormal->get_field<int32_t>("_MapNo") = normal["Map"];
        *qnormal->get_field<uint32_t>("_BaseTime") = normal["BaseTime"];
        *qnormal->get_field<int32_t>("_TimeVariation") = normal["TimeVariation"];
        *qnormal->get_field<int32_t>("_TimeLimit") = normal["TimeLimit"];
        *qnormal->get_field<int32_t>("_QuestLife") = normal["Carts"];
        
        if (const auto conditions = qnormal->get_field<ManagedObject*>("_OrderType")) {
            *conditions = utility::create_managed_array("snow.quest.QuestOrderType", normal["QuestConditions"].size());
            (*conditions)->add_ref();

            for (auto i = 0u; i < normal["QuestConditions"].size(); ++i) {
                utility::call(*conditions, "Set", i, normal["QuestConditions"][i].get<int32_t>());
            }
        }

        if (const auto targets = qnormal->get_field<ManagedObject*>("_TargetType")) {
            *targets = utility::create_managed_array("snow.quest.QuestTargetType", normal["TargetTypes"].size());
            (*targets)->add_ref();

            for (auto i = 0u; i < normal["TargetTypes"].size(); ++i) {
                utility::call(*targets, "Set", i, normal["TargetTypes"][i].get<uint8_t>());
            }
        }

        if (const auto targets = qnormal->get_field<ManagedObject*>("_TgtEmType")) {
            *targets = utility::create_managed_array("snow.enemy.EnemyDef.EmTypes", normal["TargetMonsters"].size());
            (*targets)->add_ref();

            for (auto i = 0u; i < normal["TargetMonsters"].size(); ++i) {
                utility::call(*targets, "Set", i, normal["TargetMonsters"][i].get<uint32_t>());
            }
        }

        if (const auto items = qnormal->get_field<ManagedObject*>("_TgtItemId")) {
            *items = utility::create_managed_array("snow.data.ContentsIdSystem.ItemId", normal["TargetItemIds"].size());
            (*items)->add_ref();

            for (auto i = 0u; i < normal["TargetItemIds"].size(); ++i) {
                utility::call(*items, "Set", i, normal["TargetItemIds"][i].get<uint32_t>());
            }
        }

        if (const auto counts = qnormal->get_field<ManagedObject*>("_TgtNum")) {
            *counts = utility::create_managed_array("System.UInt32", normal["TargetAmounts"].size());
            (*counts)->add_ref();

            for (auto i = 0u; i < normal["TargetAmounts"].size(); ++i) {
                utility::call(*counts, "Set", i, normal["TargetAmounts"][i].get<uint32_t>());
            }
        }

        if (const auto monsters = qnormal->get_field<ManagedObject*>("_BossEmType")) {
            if (const auto conds = qnormal->get_field<ManagedObject*>("_BossSetCondition")) {
                if (const auto params = qnormal->get_field<ManagedObject*>("_BossSetParam")) {
                    constexpr uint32_t max_monster_count = 7;

                    *monsters = utility::create_managed_array("snow.enemy.EnemyDef.EmTypes", max_monster_count);
                    *conds = utility::create_managed_array("snow.QuestManager.BossSetCondition", max_monster_count);
                    *params = utility::create_managed_array("System.UInt32", max_monster_count);

                    (*monsters)->add_ref();
                    (*conds)->add_ref();
                    (*params)->add_ref();

                    for (auto i = 0u; i < max_monster_count; ++i) {
                        const auto& mon = normal["Monsters"][i];

                        utility::call(*monsters, "Set", i, mon["Id"].get<uint32_t>());
                        utility::call(*conds, "Set", i, mon["SpawnCondition"].get<uint32_t>());
                        utility::call(*params, "Set", i, mon["SpawnParam"].get<uint32_t>());
                    }
                }
            }
        }

        *qnormal->get_field<uint8_t>("_InitExtraEmNum") = normal["ExtraMonsterCount"];
        *qnormal->get_field<bool>("_IsSwapExitMarionette") = normal["SwapExitRide"];

        if (const auto rates = qnormal->get_field<ManagedObject*>("_SwapEmRate")) {
            *rates = utility::create_managed_array("System.Byte", normal["SwapFrequencies"].size());
            (*rates)->add_ref();

            for (auto i = 0u; i < normal["SwapFrequencies"].size(); ++i) {
                utility::call(*rates, "Set", i, normal["SwapFrequencies"][i].get<uint8_t>());
            }
        }

        if (const auto conds = qnormal->get_field<ManagedObject*>("_SwapSetCondition")) {
            *conds = utility::create_managed_array("snow.QuestManager.SwapSetCondition", normal["SwapConditions"].size());
            (*conds)->add_ref();

            for (auto i = 0u; i < normal["SwapConditions"].size(); ++i) {
                utility::call(*conds, "Set", i, normal["SwapConditions"][i].get<uint32_t>());
            }
        }

        if (const auto params = qnormal->get_field<ManagedObject*>("_SwapSetParam")) {
            *params = utility::create_managed_array("System.Byte", normal["SwapParams"].size());
            (*params)->add_ref();

            for (auto i = 0u; i < normal["SwapParams"].size(); ++i) {
                utility::call(*params, "Set", i, normal["SwapParams"][i].get<uint8_t>());
            }
        }

        if (const auto counts = qnormal->get_field<ManagedObject*>("_SwapExitTime")) {
            *counts = utility::create_managed_array("System.Byte", normal["SwapExitTimes"].size());
            (*counts)->add_ref();

            for (auto i = 0u; i < normal["SwapExitTimes"].size(); ++i) {
                utility::call(*counts, "Set", i, normal["SwapExitTimes"][i].get<uint8_t>());
            }
        }

        *qnormal->get_field<uint32_t>("_SwapStopType") = normal["SwapStopType"];
        *qnormal->get_field<uint8_t>("_SwapStopParam") = normal["SwapStopParam"];
        *qnormal->get_field<uint32_t>("_SwapExecType") = normal["SwapExecType"];

        *qnormal->get_field<uint32_t>("_RemMoney") = normal["Reward"]["Zenny"];
        *qnormal->get_field<uint32_t>("_RemVillagePoint") = normal["Reward"]["Points"];
        *qnormal->get_field<uint32_t>("_RemRankPoint") = normal["Reward"]["HRP"];

        *qnormal->get_field<uint32_t>("_SupplyTbl") = normal["SupplyTable"];

        if (const auto icons = qnormal->get_field<ManagedObject*>("_Icon")) {
            constexpr uint32_t quest_icon_count = 5;

            *icons = utility::create_managed_array("snow.gui.SnowGuiCommonUtility.Icon.EnemyIconFrameForQuestOrder", quest_icon_count);
            (*icons)->add_ref();

            for (auto i = 0u; i < quest_icon_count; ++i) {
                utility::call(*icons, "Set", i, normal["Icons"][i].get<int32_t>());
            }
        }

        *qnormal->get_field<bool>("_IsTutorial") = normal["Tutorial"];
        *qnormal->get_field<bool>("_IsFromNpc") = normal["FromNpc"];

        *qnormal->get_field<bool>("_FenceDefaultActive") = normal["ArenaParam"]["FenceDefaultActive"];
        *qnormal->get_field<uint16_t>("_FenceActiveSec") = normal["ArenaParam"]["FenceUptime"];
        *qnormal->get_field<uint16_t>("_FenceDefaultWaitSec") = normal["ArenaParam"]["FenceInitialDelay"];
        *qnormal->get_field<uint16_t>("_FenceReloadSec") = normal["ArenaParam"]["FenceCooldown"];

        if (const auto pillars = qnormal->get_field<ManagedObject*>("_IsUsePillar")) {
            constexpr uint32_t arena_pillar_count = 3;

            *pillars = utility::create_managed_array("System.Boolean", arena_pillar_count);
            (*pillars)->add_ref();

            for (auto i = 0u; i < arena_pillar_count; ++i) {
                utility::call(*pillars, "Set", i, normal["ArenaParam"]["Pillars"][i].get<bool>());
            }
        }

        *qnormal->get_field<uint16_t>("_AutoMatchHR") = normal["AutoMatchHR"];
        *qnormal->get_field<int32_t>("_BattleBGMType") = normal["BattleBGMType"];
        *qnormal->get_field<int32_t>("_ClearBGMType") = normal["ClearBGMType"];
    }

    if (j.contains("EnemyData") && j["EnemyData"].type() != nlohmann::detail::value_t::null) {
        const auto& enemy = j["EnemyData"];

        *qnormal->get_field<uint32_t>("_QuestNo") = quest_id;
        *qenemy->get_field<int32_t>("_EmsSetNo") = enemy["SmallMonsters"]["SpawnType"];
        *qenemy->get_field<uint8_t>("_ZakoVital") = enemy["SmallMonsters"]["HealthTable"];
        *qenemy->get_field<uint8_t>("_ZakoAttack") = enemy["SmallMonsters"]["AttackTable"];
        *qenemy->get_field<uint8_t>("_ZakoParts") = enemy["SmallMonsters"]["PartTable"];
        *qenemy->get_field<uint8_t>("_ZakoOther") = enemy["SmallMonsters"]["OtherTable"];
        *qenemy->get_field<uint8_t>("_ZakoMulti") = enemy["SmallMonsters"]["MultiTable"];

        std::vector tables = {qenemy->get_field<ManagedObject*>("_RouteNo"),
            qenemy->get_field<ManagedObject*>("_PartsTbl"), qenemy->get_field<ManagedObject*>("_InitSetName"),
            qenemy->get_field<ManagedObject*>("_SubType"), qenemy->get_field<ManagedObject*>("_VitalTbl"),
            qenemy->get_field<ManagedObject*>("_AttackTbl"), qenemy->get_field<ManagedObject*>("_OtherTbl"),
            qenemy->get_field<ManagedObject*>("_StaminaTbl"), qenemy->get_field<ManagedObject*>("_Scale"),
            qenemy->get_field<ManagedObject*>("_ScaleTbl"), qenemy->get_field<ManagedObject*>("_Difficulty"),
            qenemy->get_field<ManagedObject*>("_BossMulti"), qenemy->get_field<ManagedObject*>("_IndividualType")
        };

        if (std::all_of(tables.begin(), tables.end(), [](auto** obj) { return obj != nullptr; })) {
            constexpr uint32_t max_monster_count = 7;

            *tables[0] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[1] = utility::create_managed_array("System.UInt16", max_monster_count);
            *tables[2] = utility::create_managed_array("System.String", max_monster_count);
            *tables[3] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[4] = utility::create_managed_array("System.UInt16", max_monster_count);
            *tables[5] = utility::create_managed_array("System.UInt16", max_monster_count);
            *tables[6] = utility::create_managed_array("System.UInt16", max_monster_count);
            *tables[7] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[8] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[9] = utility::create_managed_array("snow.enemy.EnemyDef.BossScaleTblType", max_monster_count);
            *tables[10] = utility::create_managed_array("snow.enemy.EnemyDef.NandoYuragi", max_monster_count);
            *tables[11] = utility::create_managed_array("System.Byte", max_monster_count);
            *tables[12] = utility::create_managed_array("snow.enemy.EnemyDef.EnemyIndividualType", max_monster_count);

            std::for_each(tables.begin(), tables.end(), [](auto** obj) { (*obj)->add_ref(); });

            for (auto i = 0u; i < max_monster_count; ++i) {
                const auto& mon = enemy["Monsters"][i];

                utility::call(*tables[0], "Set", i, mon["PathId"].get<uint8_t>());
                utility::call(*tables[1], "Set", i, mon["PartTable"].get<uint16_t>());

                if (const auto str = utility::create_managed_string(mon["SetName"].get<std::string>())) {
                    reinterpret_cast<ManagedObject*>(str)->add_ref();
                    utility::call(*tables[2], "Set", i, str);
                }

                utility::call(*tables[3], "Set", i, mon["SubType"].get<uint8_t>());
                utility::call(*tables[4], "Set", i, mon["HealthTable"].get<uint16_t>());
                utility::call(*tables[5], "Set", i, mon["AttackTable"].get<uint16_t>());
                utility::call(*tables[6], "Set", i, mon["OtherTable"].get<uint16_t>());
                utility::call(*tables[7], "Set", i, mon["StaminaTable"].get<uint8_t>());
                utility::call(*tables[8], "Set", i, mon["Size"].get<uint8_t>());
                utility::call(*tables[9], "Set", i, mon["SizeTable"].get<int32_t>());
                utility::call(*tables[10], "Set", i, mon["Difficulty"].get<int32_t>());
                utility::call(*tables[11], "Set", i, mon["MultiTable"].get<uint8_t>());
                utility::call(*tables[12], "Set", i, mon["IndividualType"].get<int32_t>());
            }
        }
    }

    if (j.contains("RampageData") && j["RampageData"].type() != nlohmann::detail::value_t::null) {
        const auto& rampage = j["RampageData"];

        if (rampage.contains("Seed")) {
            is_rampage_quest = true;

            *qnormal->get_field<uint32_t>("_QuestNo") = quest_id;
            *qrampage->get_field<int32_t>("_RandomSeed") = rampage["Seed"];
            *qrampage->get_field<uint8_t>("_Attr") = rampage["QuestAttr"];

            if (const auto waves = qrampage->get_field<ManagedObject*>("_WaveData")) {
                *waves = utility::create_managed_array("snow.quest.HyakuryuQuestData.WaveData", rampage["Waves"].size());
                (*waves)->add_ref();

                for (auto i = 0u; i < rampage["Waves"].size(); ++i) {
                    const auto wave_ =
                        new_instance(api->get_vm_context(), api->tdb()->find_type("snow.quest.HyakuryuQuestData.WaveData"), 0);
                    wave_->add_ref();

                    utility::call(*waves, "Set", i, wave_);
                    if (const auto qwave = utility::call(*waves, "Get", i)) {
                        const auto& wave = rampage["Waves"][i];

                        *qwave->get_field<uint32_t>("_BossEm") = wave["BossMonster"];
                        *qwave->get_field<uint32_t>("_BossSubType") = wave["BossSubType"];
                        *qwave->get_field<int32_t>("_OrderTblNo") = wave["OrderTable"];
                        *qwave->get_field<int32_t>("_BossEmNandoTblNo") = wave["BossMonsterNandoTable"];
                        *qwave->get_field<int32_t>("_WaveEmNandoTblNo") = wave["WaveMonsterNandoTable"];

                        if (const auto monsters = qwave->get_field<ManagedObject*>("_EmTable")) {
                            *monsters = utility::create_managed_array("snow.enemy.EnemyDef.EmTypes", wave["Monsters"].size());
                            (*monsters)->add_ref();

                            for (auto k = 0u; k < wave["monsters"].size(); ++k) {
                                utility::call(*monsters, "Set", k, wave["Monsters"][k].get<uint32_t>());
                            }
                        }
                    }
                }
            }

            *qrampage->get_field<int32_t>("_QuestLv") = rampage["QuestLevel"];
            *qrampage->get_field<int32_t>("_MapNo") = rampage["Map"];
            *qrampage->get_field<uint8_t>("_Category") = rampage["Category"];
            *qrampage->get_field<bool>("_IsVillage") = rampage["IsVillage"];
            *qrampage->get_field<uint8_t>("_BaseTime") = rampage["BaseTime"];
            *qrampage->get_field<uint8_t>("_StartBlockNo") = rampage["StartBlock"];
            *qrampage->get_field<uint8_t>("_EndBlockNo") = rampage["EndBlock"];
            *qrampage->get_field<uint8_t>("_ExtraEmWaveNo") = rampage["ExtraWaveCount"];
            *qrampage->get_field<int8_t>("_ExtraEmNandoTblNo") = rampage["ExtraMonsterNandoTable"];
            *qrampage->get_field<uint8_t>("_NushiOrderTblNo") = rampage["ApexOrderTable"];
            *qrampage->get_field<uint8_t>("_HmUnlockTblNo") = rampage["WeaponUnlockTable"];

            if (const auto subtargets = qrampage->get_field<ManagedObject*>("_SubTarget")) {
                *subtargets = utility::create_managed_array("snow.quest.QuestTargetType", rampage["SubTargets"].size());
                (*subtargets)->add_ref();

                for (auto i = 0u; i < rampage["SubTargets"].size(); ++i) {
                    utility::call(*subtargets, "Set", i, rampage["SubTargets"][i].get<uint8_t>());
                }
            }

            *qrampage->get_field<uint8_t>("_SubTarget5WaveNo") = rampage["SubTarget5Wave"];
        }
    }

    utility::call(quest, "set_RawNormal", qnormal);
    utility::call(quest, "set_RawEnemy", qenemy);

    if (is_rampage_quest) {
        utility::call(quest, "set_Hyakuryu", qrampage);
    } else {
        qrampage->release();
    }

    const bool is_replacement = is_existing_quest(quest_id);
    const auto original = is_replacement ? utility::call(questdict, "get_Item", quest_id) : nullptr;
    const auto log_refcounts = [&](std::string_view id) {
        if (is_replacement) {
            utility::log(fmt::format("[{}] {}_Original RefCount: {}", quest_id, id, original->get_ref_count()));
        }
        utility::log(fmt::format("[{}] {}_Quest RefCount: {}", quest_id, id, quest->get_ref_count()));
    };

    log_refcounts("PreMap");
    m_custom_quests.emplace(std::piecewise_construct, std::forward_as_tuple(quest_id), std::forward_as_tuple(j, quest, original));
    log_refcounts("PostMap");
    utility::call(questdict, "Insert", quest_id, quest, false);
    log_refcounts("PostInsert");
}

bool QuestLoader::is_existing_quest(int32_t quest_id) const {
    const auto manager = m_quest_exporter.get_quest_manager();
    
    const auto quest_data = 
        *(*manager->get_field<ManagedObject*>("_normalQuestData"))->get_field<REArray<ManagedObject*>*>("_Param");
    const auto quest_data_kohaku =
        *(*manager->get_field<ManagedObject*>("_nomalQuestDataKohaku"))->get_field<REArray<ManagedObject*>*>("_Param");

    if (std::ranges::any_of(*quest_data, [quest_id](const ManagedObject* obj) 
        { return *obj->get_field<int32_t>("_QuestNo") == quest_id; })) {
        return true;
    }

    if (std::ranges::any_of(*quest_data_kohaku,
            [quest_id](const ManagedObject* obj) { return *obj->get_field<int32_t>("_QuestNo") == quest_id; })) {
        return true;
    }
    
    return false;
}

SystemString* QuestLoader::get_quest_text_hook(void* vmctx, ManagedObject* this_, QuestText type, void* qi) {
    const auto quest_id = utility::call<int32_t>(this_, "getQuestNo");
    const auto loader = get();


    if (loader->m_custom_quests.contains(quest_id)) {
        const auto& quest = loader->m_custom_quests[quest_id];

        if (!quest.m_is_replacement || quest.m_enabled) {
            const auto language = utility::call<GameLanguage>(loader->m_quest_exporter.get_message_manager(), "get_nowLanguage");
            const auto& info = quest.get_quest_info(language);

            switch (type) {
            case QuestText::TITLE:
                return utility::create_managed_string(info.m_name);
            case QuestText::CLIENT:
                return utility::create_managed_string(info.m_client);
            case QuestText::REQUEST:
                return utility::create_managed_string(info.m_description);
            case QuestText::TARGET:
                return utility::create_managed_string(info.m_target);
            default:
                return loader->m_get_quest_text_hook->call_original<SystemString*>(vmctx, this_, type, qi);
            }
        }
    }

    return loader->m_get_quest_text_hook->call_original<SystemString*>(vmctx, this_, type, qi);
}

ManagedObject* QuestLoader::make_questno_list_hook(void* vmctx, ManagedObject* this_, ManagedObject* src, bool is_quick_match) {
    const auto loader = get();
    const auto list = loader->m_make_questno_list_hook->call_original<ManagedObject*>(vmctx, this_, src, is_quick_match);

    if (loader->m_skip_hook) {
        return list;
    }

    const auto is_normal_quest = [](QuestType quest_type) {
        static constexpr std::array<QuestType, 6> normal_quests = {
            QuestType::HUNTING,
            QuestType::KILL,
            QuestType::CAPTURE,
            QuestType::BOSSRUSH,
            QuestType::COLLECTS,
            QuestType::SPECIAL
        };

        return std::ranges::any_of(normal_quests, [quest_type](auto type_) { return (quest_type & type_) != 0; });
    };

    const auto type = utility::call<QuestCounterTopMenuType>(loader->m_quest_counter, "getQuestCounterSelectedTopMenu()");
    const auto level = utility::call<QuestCounterLevelMenuType>(loader->m_quest_counter, "getQuestCounterSelectedLevelMenu");
    const auto quest_level = utility::call<QuestLevel>(loader->m_quest_counter, "convertLeveLMenuToQuestLevel", level);
    
    for (const auto& [id, quest] : loader->m_custom_quests) {
        if (!quest.m_is_replacement) { // Only explicitly add fully custom quests. Replacements are already in the list
            if (quest_level == utility::call<QuestLevel>(quest.m_memory_object, "getQuestLv")) {
                const auto enemy_level = utility::call<EnemyLv>(quest.m_memory_object, "getEnemyLv");
                const auto quest_type = utility::call<QuestType>(quest.m_memory_object, "getQuestType");
                bool add_quest = false;

                if (type == QuestCounterTopMenuType::Normal_Vil && enemy_level == EnemyLv::Village ||
                    type == QuestCounterTopMenuType::Normal_Hall_Low && enemy_level == EnemyLv::Low ||
                    type == QuestCounterTopMenuType::Normal_Hall_High && enemy_level == EnemyLv::High ||
                    type == QuestCounterTopMenuType::Normal_Hall_Master && enemy_level == EnemyLv::Master ||
                    type == QuestCounterTopMenuType::Normal_Hall_HighLow && 
                    (enemy_level == EnemyLv::Low || enemy_level == EnemyLv::High)) {
                    if (is_normal_quest(quest_type)) {
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
                } else if (type == QuestCounterTopMenuType::Kyousei) {
                    if (quest_type & QuestType::KYOUSEI) {
                        add_quest = true;
                    }
                }

                if (add_quest) {
                    utility::log(fmt::format("Adding [{}] {} to quest list", id, quest.get_quest_info().m_name));
                    utility::call(list, "Add", id);
                }
            }
        }
    }

    return list;
}

ManagedObject* QuestLoader::make_quest_list_hyakuryu_hook(void* vmctx, ManagedObject* this_, ManagedObject* src, bool is_village, const int rank) {
    const auto loader = get();
    const auto list = loader->m_make_quest_list_hyakuryu_hook->call_original<ManagedObject*>(vmctx, this_, src, is_village, rank);

    if (!is_village) {
        for (const auto& [id, quest] : loader->m_custom_quests) {
            if (!quest.m_is_replacement) {
                bool add_quest = false;
                if (utility::call<QuestType>(quest.m_memory_object, "getQuestType") & QuestType::HYAKURYU) {
                    const auto quest_level = utility::call<QuestLevel>(quest.m_memory_object, "getQuestLv");

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
    }

    return list;
}

void QuestLoader::quest_counter_awake_hook(void* vmctx, ManagedObject* this_) {
    const auto loader = get();
    loader->m_quest_counter = this_;

    return loader->m_quest_counter_awake_hook->call_original<void>(vmctx, this_);
}

void QuestLoader::init_quest_data_dict_hook(void* vmctx, ManagedObject* this_) {
    const auto loader = get();

    loader->m_init_quest_data_dict_hook->call_original<void>(vmctx, this_);
    loader->read_quests();
}

const wchar_t* QuestLoader::get_message_hook(void* this_, _GUID* guid, GameLanguage language) {
    const auto loader = get();

    const auto override = loader->m_quest_exporter.get_override_language();
    if (override != GameLanguage::NONE) {
        language = override;
    }

    return loader->m_get_message_hook->call_original<const wchar_t*>(this_, guid, language);
}
