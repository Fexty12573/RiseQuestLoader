#include "QuestExporter.h"
#include "Quest.h"

using namespace reframework;

QuestExporter::QuestExporter() {
    initialize();
}

bool QuestExporter::initialize() {
    if (m_initialized) {
        return true;
    }

    m_quest_manager = API::get()->get_managed_singleton("snow.QuestManager");
    if (!m_quest_manager) {
        return false;
    }

    m_get_quest_data = m_quest_manager->get_type_definition()->find_method("getQuestData(System.Int32)");
    if (!m_get_quest_data) {
        return false;
    }

    m_initialized = true;
    return true;
}

nlohmann::ordered_json QuestExporter::export_quest(int32_t quest_id) const {
    if (!m_initialized) {
        return {};
    }

    if (const auto quest = utility::call(m_get_quest_data, m_quest_manager, quest_id)) {
        const auto qnormal = utility::call(quest, "get_RawNormal");
        const auto qenemy = utility::call(quest, "get_RawEnemy");
        const auto qrampage = utility::call(quest, "get_Hyakuryu");


        nlohmann::ordered_json q = nlohmann::ordered_json::object();

        q["ID"] = quest_id;
        auto text = nlohmann::ordered_json::object();
        auto normal = nlohmann::ordered_json::object();
        auto enemy = nlohmann::ordered_json::object();
        auto rampage = nlohmann::ordered_json::object();

        // QuestText
        text["name"] = utility::str_call(quest, "getQuestText", QuestText::TITLE, nullptr);
        text["client"] = utility::str_call(quest, "getQuestText", QuestText::CLIENT, nullptr);
        text["description"] = utility::str_call(quest, "getQuestText", QuestText::REQUEST, nullptr);
        text["target"] = utility::str_call(quest, "getQuestText", QuestText::TARGET, nullptr);

        q["QuestText"] = text;

        // QuestData
        if (qnormal) {
            normal["type"] = *qnormal->get_field<uint32_t>("_QuestType");
            normal["level"] = *qnormal->get_field<int32_t>("_QuestLv");
            normal["enemylevel"] = *qnormal->get_field<int32_t>("_EnemyLv");
            normal["map"] = *qnormal->get_field<int32_t>("_MapNo");
            normal["quest_time"] = *qnormal->get_field<uint32_t>("_BaseTime");
            normal["time_variation"] = *qnormal->get_field<int32_t>("_TimeVariation");
            normal["time_limit"] = *qnormal->get_field<int32_t>("_TimeLimit");
            normal["carts"] = *qnormal->get_field<int32_t>("_QuestLife");

            if (const auto conditions = *qnormal->get_field<API::ManagedObject*>("_OrderType")) {
                normal["conditions"][0] = utility::call<int32_t>(conditions, "Get", 0);
                normal["conditions"][1] = utility::call<int32_t>(conditions, "Get", 1);
            }

            if (const auto targets = *qnormal->get_field<API::ManagedObject*>("_TargetType")) {
                normal["targets"][0] = utility::call<uint8_t>(targets, "Get", 0);
                normal["targets"][1] = utility::call<uint8_t>(targets, "Get", 1);
            }

            if (const auto targets = *qnormal->get_field<API::ManagedObject*>("_TgtEmType")) {
                normal["target_monsters"][0] = utility::call<uint32_t>(targets, "Get", 0);
                normal["target_monsters"][1] = utility::call<uint32_t>(targets, "Get", 1);
            }

            if (const auto items = *qnormal->get_field<API::ManagedObject*>("_TgtItemId")) {
                normal["target_items"][0] = utility::call<uint32_t>(items, "Get", 0);
                normal["target_items"][1] = utility::call<uint32_t>(items, "Get", 1);
            }

            if (const auto counts = *qnormal->get_field<API::ManagedObject*>("_TgtNum")) {
                normal["amount"][0] = utility::call<uint32_t>(counts, "Get", 0);
                normal["amount"][1] = utility::call<uint32_t>(counts, "Get", 1);
            }

            if (const auto monsters = *qnormal->get_field<API::ManagedObject*>("_BossEmType")) {
                if (const auto conds = *qnormal->get_field<API::ManagedObject*>("_BossSetCondition")) {
                    if (const auto params = *qnormal->get_field<API::ManagedObject*>("_BossSetParam")) {
                        for (int i = 0; i < 7; ++i) {
                            auto mon = nlohmann::ordered_json::object();

                            mon["id"] = utility::call<uint32_t>(monsters, "Get", i);
                            mon["spawn_condition"] = utility::call<uint32_t>(conds, "Get", i);
                            mon["param"] = utility::call<uint32_t>(params, "Get", i);

                            normal["monsters"][i] = mon;
                        }
                    }
                }
            }

            normal["extra_monster_count"] = *qnormal->get_field<uint8_t>("_InitExtraEmNum");
            normal["swap_exit_ride"] = *qnormal->get_field<bool>("_IsSwapExitMarionette");

            if (const auto rates = *qnormal->get_field<API::ManagedObject*>("_SwapEmRate")) {
                normal["swap_frequency"][0] = utility::call<uint8_t>(rates, "Get", 0);
                normal["swap_frequency"][1] = utility::call<uint8_t>(rates, "Get", 1);
            }

            if (const auto conds = *qnormal->get_field<API::ManagedObject*>("_SwapSetCondition")) {
                normal["swap_condition"][0] = utility::call<uint32_t>(conds, "Get", 0);
                normal["swap_condition"][1] = utility::call<uint32_t>(conds, "Get", 1);
            }

            if (const auto params = *qnormal->get_field<API::ManagedObject*>("_SwapSetParam")) {
                normal["swap_param"][0] = utility::call<uint8_t>(params, "Get", 0);
                normal["swap_param"][1] = utility::call<uint8_t>(params, "Get", 1);
            }

            if (const auto counts = *qnormal->get_field<API::ManagedObject*>("_SwapExitTime")) {
                normal["swap_exit_time"][0] = utility::call<uint8_t>(counts, "Get", 0);
                normal["swap_exit_time"][1] = utility::call<uint8_t>(counts, "Get", 1);
            }

            normal["swap_stop_type"] = *qnormal->get_field<uint32_t>("_SwapStopType");
            normal["swap_stop_param"] = *qnormal->get_field<uint8_t>("_SwapStopParam");
            normal["swap_exec_type"] = *qnormal->get_field<uint32_t>("_SwapExecType");

            normal["reward"]["zenny"] = *qnormal->get_field<uint32_t>("_RemMoney");
            normal["reward"]["points"] = *qnormal->get_field<uint32_t>("_RemVillagePoint");
            normal["reward"]["hrp"] = *qnormal->get_field<uint32_t>("_RemRankPoint");

            normal["supply_id"] = *qnormal->get_field<uint32_t>("_SupplyTbl");

            if (const auto icons = *qnormal->get_field<API::ManagedObject*>("_Icon")) {
                for (int i = 0; i < 5; ++i) {
                    normal["monster_icons"][i] = utility::call<int32_t>(icons, "Get", i);
                }
            }

            normal["tutorial"] = *qnormal->get_field<bool>("_IsTutorial");

            normal["arena_param"]["fence_default_active"] = *qnormal->get_field<bool>("_FenceDefaultActive");
            normal["arena_param"]["fence_uptime"] = *qnormal->get_field<uint16_t>("_FenceActiveSec");
            normal["arena_param"]["fence_initial_delay"] = *qnormal->get_field<uint16_t>("_FenceDefaultWaitSec");
            normal["arena_param"]["fence_cooldown"] = *qnormal->get_field<uint16_t>("_FenceReloadSec");

            if (const auto pillars = *qnormal->get_field<API::ManagedObject*>("_IsUsePillar")) {
                for (int i = 0; i < 3; ++i) {
                    normal["arena_param"]["pillars_active"][i] = utility::call<bool>(pillars, "Get", i);
                }
            }

            normal["auto_match_hr"] = *qnormal->get_field<uint16_t>("_AutoMatchHR");
            normal["battle_bgm_type"] = *qnormal->get_field<int32_t>("_BattleBGMType");
            normal["clear_bgm_type"] = *qnormal->get_field<int32_t>("_ClearBGMType");
        }

        q["QuestData"] = normal;

        // EnemyData
        if (qenemy) {
            enemy["small_monsters"]["spawn_type"] = *qenemy->get_field<int32_t>("_EmsSetNo");
            enemy["small_monsters"]["hp"] = *qenemy->get_field<uint8_t>("_ZakoVital");
            enemy["small_monsters"]["attack"] = *qenemy->get_field<uint8_t>("_ZakoAttack");
            enemy["small_monsters"]["part_hp"] = *qenemy->get_field<uint8_t>("_ZakoParts");
            enemy["small_monsters"]["other"] = *qenemy->get_field<uint8_t>("_ZakoOther");
            enemy["small_monsters"]["multi"] = *qenemy->get_field<uint8_t>("_ZakoMulti");

            std::vector tables = {*qenemy->get_field<API::ManagedObject*>("_RouteNo"),
                *qenemy->get_field<API::ManagedObject*>("_InitSetName"), *qenemy->get_field<API::ManagedObject*>("_SubType"),
                *qenemy->get_field<API::ManagedObject*>("_VitalTbl"), *qenemy->get_field<API::ManagedObject*>("_AttackTbl"),
                *qenemy->get_field<API::ManagedObject*>("_OtherTbl"), *qenemy->get_field<API::ManagedObject*>("_StaminaTbl"),
                *qenemy->get_field<API::ManagedObject*>("_Scale"), *qenemy->get_field<API::ManagedObject*>("_ScaleTbl"),
                *qenemy->get_field<API::ManagedObject*>("_Difficulty"), *qenemy->get_field<API::ManagedObject*>("_BossMulti")};

            if (std::all_of(tables.begin(), tables.end(), [](const API::ManagedObject* obj) { return obj != nullptr; })) {
                for (int i = 0; i < 7; ++i) {
                    auto mon = nlohmann::ordered_json::object();

                    mon["path_id"] = utility::call<uint8_t>(tables[0], "Get", i);
                    mon["set_name"] = utility::str_call(tables[1], "Get", i);
                    mon["sub_type"] = utility::call<uint8_t>(tables[2], "Get", i);
                    mon["hp"] = utility::call<uint8_t>(tables[3], "Get", i);
                    mon["attack"] = utility::call<uint8_t>(tables[4], "Get", i);
                    mon["other"] = utility::call<uint8_t>(tables[5], "Get", i);
                    mon["stamina"] = utility::call<uint8_t>(tables[6], "Get", i);
                    mon["size"] = utility::call<uint8_t>(tables[7], "Get", i);
                    mon["scale_type"] = utility::call<int32_t>(tables[8], "Get", i);
                    mon["difficulty"] = utility::call<int32_t>(tables[9], "Get", i);
                    mon["multi"] = utility::call<uint8_t>(tables[10], "Get", i);

                    enemy["monsters"][i] = mon;
                }
            } else {
                enemy["monsters"] = nlohmann::ordered_json::array();
            }
        }

        q["EnemyData"] = enemy;

        // RampageData
        if (qrampage) {
            rampage["seed"] = *qrampage->get_field<int32_t>("_RandomSeed");
            rampage["attributes"] = *qrampage->get_field<uint8_t>("_Attr");

            if (const auto waves = *qrampage->get_field<API::ManagedObject*>("_WaveData")) {
                for (int i = 0; i < 3; ++i) {
                    if (const auto qwave = utility::call(waves, "Get", i)) {
                        auto wave = nlohmann::ordered_json::object();

                        wave["boss_monster"] = *qwave->get_field<uint32_t>("_BossEm");
                        wave["boss_subtype"] = *qwave->get_field<uint32_t>("_BossSubType");
                        wave["order_table"] = *qwave->get_field<int32_t>("_OrderTblNo");
                        wave["boss_em_nando_table"] = *qwave->get_field<int32_t>("_BossEmNandoTblNo");
                        wave["wave_em_nando_table"] = *qwave->get_field<int32_t>("_WaveEmNandoTblNo");

                        if (const auto monsters = *qwave->get_field<API::ManagedObject*>("_EmTable")) {
                            for (int j = 0; j < 4; ++j) {
                                wave["monsters"][j] = utility::call<uint32_t>(monsters, "Get", j);
                            }
                        }

                        rampage["waves"][i] = wave;
                    }
                }
            }

            rampage["level"] = *qrampage->get_field<int32_t>("_QuestLv");
            rampage["map"] = *qrampage->get_field<int32_t>("_MapNo");
            rampage["category"] = *qrampage->get_field<uint8_t>("_Category");
            rampage["village"] = *qrampage->get_field<bool>("_IsVillage");
            rampage["base_time"] = *qrampage->get_field<uint8_t>("_BaseTime");
            rampage["start_block"] = *qrampage->get_field<uint8_t>("_StartBlockNo");
            rampage["end_block"] = *qrampage->get_field<uint8_t>("_EndBlockNo");
            rampage["extra_wave_count"] = *qrampage->get_field<uint8_t>("_ExtraEmWaveNo");
            rampage["extra_em_nando_table"] = *qrampage->get_field<int8_t>("_ExtraEmNandoTblNo");
            rampage["apex_order_table"] = *qrampage->get_field<uint8_t>("_NushiOrderTblNo");
            rampage["weapon_unlock_table"] = *qrampage->get_field<uint8_t>("_HmUnlockTblNo");

            if (const auto subtargets = *qrampage->get_field<API::ManagedObject*>("_SubTarget")) {
                for (int i = 0; i < 6; ++i) {
                    rampage["sub_targets"][i] = utility::call<uint8_t>(subtargets, "Get", i);
                }
            } else {
                rampage["sub_targets"] = nlohmann::ordered_json::array();
            }

            rampage["subtarget_5_wave"] = *qrampage->get_field<uint8_t>("_SubTarget5WaveNo");

            q["RampageData"] = rampage;
        }

        return q;
    }

    return {};
}

std::vector<nlohmann::ordered_json> QuestExporter::export_all_quests() const {
    if (!m_initialized) {
        return {};
    }

    const auto questdict = *m_quest_manager->get_field<API::ManagedObject*>("_QuestDataDictionary");
    if (!questdict) {
        return {};
    }

    std::vector<nlohmann::ordered_json> quests;
    const auto& api = API::get();

    struct DictEntry {
        int32_t hash_code;
        int32_t next;
        int32_t key;
        void* value;
    };

    if (uint64_t* entries = *questdict->get_field<uint64_t*>("_entries")) {
        const auto list = reinterpret_cast<DictEntry*>(entries + 4);

        for (int i = 0; list[i].key != 0; ++i) {
            quests.push_back(export_quest(list[i].key));
        }
    }

    return quests;
}

