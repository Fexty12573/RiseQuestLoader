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

        q["QuestID"] = quest_id;
        auto text = nlohmann::ordered_json::object();
        auto normal = nlohmann::ordered_json::object();
        auto enemy = nlohmann::ordered_json::object();
        auto rampage = nlohmann::ordered_json::object();

        // QuestText
        text["Name"] = utility::str_call(quest, "getQuestText", QuestText::TITLE, nullptr);
        text["Client"] = utility::str_call(quest, "getQuestText", QuestText::CLIENT, nullptr);
        text["Description"] = utility::str_call(quest, "getQuestText", QuestText::REQUEST, nullptr);
        text["Target"] = utility::str_call(quest, "getQuestText", QuestText::TARGET, nullptr);

        q["QuestText"] = text;

        // QuestData
        if (qnormal) {
            q["QuestText"]["DebugName"] = utility::narrow((*qnormal->get_field<SystemString*>("_DbgName"))->data);
            q["QuestText"]["DebugClient"] = utility::narrow((*qnormal->get_field<SystemString*>("_DbgClient"))->data);
            q["QuestText"]["DebugDescription"] = utility::narrow((*qnormal->get_field<SystemString*>("_DbgContent"))->data);

            normal["QuestType"] = *qnormal->get_field<uint32_t>("_QuestType");
            normal["QuestLevel"] = *qnormal->get_field<int32_t>("_QuestLv");
            normal["EnemyLevel"] = *qnormal->get_field<int32_t>("_EnemyLv");
            normal["Map"] = *qnormal->get_field<int32_t>("_MapNo");
            normal["BaseTime"] = *qnormal->get_field<uint32_t>("_BaseTime");
            normal["TimeVariation"] = *qnormal->get_field<int32_t>("_TimeVariation");
            normal["TimeLimit"] = *qnormal->get_field<int32_t>("_TimeLimit");
            normal["Carts"] = *qnormal->get_field<int32_t>("_QuestLife");

            if (const auto conditions = *qnormal->get_field<API::ManagedObject*>("_OrderType")) {
                normal["QuestConditions"][0] = utility::call<int32_t>(conditions, "Get", 0);
                normal["QuestConditions"][1] = utility::call<int32_t>(conditions, "Get", 1);
            }

            if (const auto targets = *qnormal->get_field<API::ManagedObject*>("_TargetType")) {
                normal["TargetTypes"][0] = utility::call<uint8_t>(targets, "Get", 0);
                normal["TargetTypes"][1] = utility::call<uint8_t>(targets, "Get", 1);
            }

            if (const auto targets = *qnormal->get_field<API::ManagedObject*>("_TgtEmType")) {
                normal["TargetMonsters"][0] = utility::call<uint32_t>(targets, "Get", 0);
                normal["TargetMonsters"][1] = utility::call<uint32_t>(targets, "Get", 1);
            }

            if (const auto items = *qnormal->get_field<API::ManagedObject*>("_TgtItemId")) {
                normal["TargetItemIds"][0] = utility::call<uint32_t>(items, "Get", 0);
                normal["TargetItemIds"][1] = utility::call<uint32_t>(items, "Get", 1);
            }

            if (const auto counts = *qnormal->get_field<API::ManagedObject*>("_TgtNum")) {
                normal["TargetAmounts"][0] = utility::call<uint32_t>(counts, "Get", 0);
                normal["TargetAmounts"][1] = utility::call<uint32_t>(counts, "Get", 1);
            }

            if (const auto monsters = *qnormal->get_field<API::ManagedObject*>("_BossEmType")) {
                if (const auto conds = *qnormal->get_field<API::ManagedObject*>("_BossSetCondition")) {
                    if (const auto params = *qnormal->get_field<API::ManagedObject*>("_BossSetParam")) {
                        for (int i = 0; i < 7; ++i) {
                            auto mon = nlohmann::ordered_json::object();

                            mon["Id"] = utility::call<uint32_t>(monsters, "Get", i);
                            mon["SpawnCondition"] = utility::call<uint32_t>(conds, "Get", i);
                            mon["SpawnParam"] = utility::call<uint32_t>(params, "Get", i);

                            normal["Monsters"][i] = mon;
                        }
                    }
                }
            }

            normal["ExtraMonsterCount"] = *qnormal->get_field<uint8_t>("_InitExtraEmNum");
            normal["SwapExitRide"] = *qnormal->get_field<bool>("_IsSwapExitMarionette");

            if (const auto rates = *qnormal->get_field<API::ManagedObject*>("_SwapEmRate")) {
                normal["SwapFrequencies"][0] = utility::call<uint8_t>(rates, "Get", 0);
                normal["SwapFrequencies"][1] = utility::call<uint8_t>(rates, "Get", 1);
            }

            if (const auto conds = *qnormal->get_field<API::ManagedObject*>("_SwapSetCondition")) {
                normal["SwapConditions"][0] = utility::call<uint32_t>(conds, "Get", 0);
                normal["SwapConditions"][1] = utility::call<uint32_t>(conds, "Get", 1);
            }

            if (const auto params = *qnormal->get_field<API::ManagedObject*>("_SwapSetParam")) {
                normal["SwapParams"][0] = utility::call<uint8_t>(params, "Get", 0);
                normal["SwapParams"][1] = utility::call<uint8_t>(params, "Get", 1);
            }

            if (const auto counts = *qnormal->get_field<API::ManagedObject*>("_SwapExitTime")) {
                normal["SwapExitTimes"][0] = utility::call<uint8_t>(counts, "Get", 0);
                normal["SwapExitTimes"][1] = utility::call<uint8_t>(counts, "Get", 1);
            }

            normal["SwapStopType"] = *qnormal->get_field<uint32_t>("_SwapStopType");
            normal["SwapStopParam"] = *qnormal->get_field<uint8_t>("_SwapStopParam");
            normal["SwapExecType"] = *qnormal->get_field<uint32_t>("_SwapExecType");

            normal["Reward"]["Zenny"] = *qnormal->get_field<uint32_t>("_RemMoney");
            normal["Reward"]["Points"] = *qnormal->get_field<uint32_t>("_RemVillagePoint");
            normal["Reward"]["HRP"] = *qnormal->get_field<uint32_t>("_RemRankPoint");

            normal["SupplyTable"] = *qnormal->get_field<uint32_t>("_SupplyTbl");

            if (const auto icons = *qnormal->get_field<API::ManagedObject*>("_Icon")) {
                for (int i = 0; i < 5; ++i) {
                    normal["Icons"][i] = utility::call<int32_t>(icons, "Get", i);
                }
            }

            normal["Tutorial"] = *qnormal->get_field<bool>("_IsTutorial");
            normal["FromNpc"] = *qnormal->get_field<bool>("_IsFromNpc");

            normal["ArenaParam"]["FenceDefaultActive"] = *qnormal->get_field<bool>("_FenceDefaultActive");
            normal["ArenaParam"]["FenceUptime"] = *qnormal->get_field<uint16_t>("_FenceActiveSec");
            normal["ArenaParam"]["FenceInitialDelay"] = *qnormal->get_field<uint16_t>("_FenceDefaultWaitSec");
            normal["ArenaParam"]["FenceCooldown"] = *qnormal->get_field<uint16_t>("_FenceReloadSec");

            if (const auto pillars = *qnormal->get_field<API::ManagedObject*>("_IsUsePillar")) {
                for (int i = 0; i < 3; ++i) {
                    normal["ArenaParam"]["Pillars"][i] = utility::call<bool>(pillars, "Get", i);
                }
            }

            normal["AutoMatchHR"] = *qnormal->get_field<uint16_t>("_AutoMatchHR");
            normal["BattleBGMType"] = *qnormal->get_field<int32_t>("_BattleBGMType");
            normal["ClearBGMType"] = *qnormal->get_field<int32_t>("_ClearBGMType");
        }

        q["QuestData"] = normal;

        // EnemyData
        if (qenemy) {
            enemy["SmallMonsters"]["SpawnType"] = *qenemy->get_field<int32_t>("_EmsSetNo");
            enemy["SmallMonsters"]["HealthTable"] = *qenemy->get_field<uint8_t>("_ZakoVital");
            enemy["SmallMonsters"]["AttackTable"] = *qenemy->get_field<uint8_t>("_ZakoAttack");
            enemy["SmallMonsters"]["PartTable"] = *qenemy->get_field<uint8_t>("_ZakoParts");
            enemy["SmallMonsters"]["OtherTable"] = *qenemy->get_field<uint8_t>("_ZakoOther");
            enemy["SmallMonsters"]["MultiTable"] = *qenemy->get_field<uint8_t>("_ZakoMulti");

            std::vector tables = {
                *qenemy->get_field<API::ManagedObject*>("_RouteNo"), *qenemy->get_field<API::ManagedObject*>("_PartsTbl"),
                *qenemy->get_field<API::ManagedObject*>("_InitSetName"), *qenemy->get_field<API::ManagedObject*>("_SubType"),
                *qenemy->get_field<API::ManagedObject*>("_VitalTbl"), *qenemy->get_field<API::ManagedObject*>("_AttackTbl"),
                *qenemy->get_field<API::ManagedObject*>("_OtherTbl"), *qenemy->get_field<API::ManagedObject*>("_StaminaTbl"),
                *qenemy->get_field<API::ManagedObject*>("_Scale"), *qenemy->get_field<API::ManagedObject*>("_ScaleTbl"),
                *qenemy->get_field<API::ManagedObject*>("_Difficulty"), *qenemy->get_field<API::ManagedObject*>("_BossMulti"),
                *qenemy->get_field<API::ManagedObject*>("_IndividualType")
            };

            if (std::all_of(tables.begin(), tables.end(), [](const API::ManagedObject* obj) { return obj != nullptr; })) {
                for (int i = 0; i < 7; ++i) {
                    auto mon = nlohmann::ordered_json::object();

                    mon["PathId"] = utility::call<uint8_t>(tables[0], "Get", i);
                    mon["PartTable"] = utility::call<uint16_t>(tables[1], "Get", i);
                    mon["SetName"] = utility::str_call(tables[2], "Get", i);
                    mon["SubType"] = utility::call<uint8_t>(tables[3], "Get", i);
                    mon["HealthTable"] = utility::call<uint16_t>(tables[4], "Get", i);
                    mon["AttackTable"] = utility::call<uint16_t>(tables[5], "Get", i);
                    mon["OtherTable"] = utility::call<uint16_t>(tables[6], "Get", i);
                    mon["StaminaTable"] = utility::call<uint8_t>(tables[7], "Get", i);
                    mon["Size"] = utility::call<uint8_t>(tables[8], "Get", i);
                    mon["SizeTable"] = utility::call<int32_t>(tables[9], "Get", i);
                    mon["Difficulty"] = utility::call<int32_t>(tables[10], "Get", i);
                    mon["MultiTable"] = utility::call<uint8_t>(tables[11], "Get", i);
                    mon["IndividualType"] = utility::call<int32_t>(tables[12], "Get", i);

                    enemy["Monsters"][i] = mon;
                }
            } else {
                enemy["Monsters"] = nlohmann::ordered_json::array();
            }
        }

        q["EnemyData"] = enemy;

        // RampageData
        if (qrampage) {
            rampage["Seed"] = *qrampage->get_field<int32_t>("_RandomSeed");
            rampage["QuestAttr"] = *qrampage->get_field<uint8_t>("_Attr");

            if (const auto waves = *qrampage->get_field<API::ManagedObject*>("_WaveData")) {
                for (int i = 0; i < 3; ++i) {
                    if (const auto qwave = utility::call(waves, "Get", i)) {
                        auto wave = nlohmann::ordered_json::object();

                        wave["BossMonster"] = *qwave->get_field<uint32_t>("_BossEm");
                        wave["BossSubType"] = *qwave->get_field<uint32_t>("_BossSubType");
                        wave["OrderTable"] = *qwave->get_field<int32_t>("_OrderTblNo");
                        wave["BossMonsterNandoTable"] = *qwave->get_field<int32_t>("_BossEmNandoTblNo");
                        wave["WaveMonsterNandoTable"] = *qwave->get_field<int32_t>("_WaveEmNandoTblNo");

                        if (const auto monsters = *qwave->get_field<API::ManagedObject*>("_EmTable")) {
                            for (int j = 0; j < 4; ++j) {
                                wave["Monsters"][j] = utility::call<uint32_t>(monsters, "Get", j);
                            }
                        }

                        rampage["Waves"][i] = wave;
                    }
                }
            }

            rampage["QuestLevel"] = *qrampage->get_field<int32_t>("_QuestLv");
            rampage["Map"] = *qrampage->get_field<int32_t>("_MapNo");
            rampage["Category"] = *qrampage->get_field<uint8_t>("_Category");
            rampage["IsVillage"] = *qrampage->get_field<bool>("_IsVillage");
            rampage["BaseTime"] = *qrampage->get_field<uint8_t>("_BaseTime");
            rampage["StartBlock"] = *qrampage->get_field<uint8_t>("_StartBlockNo");
            rampage["EndBlock"] = *qrampage->get_field<uint8_t>("_EndBlockNo");
            rampage["ExtraWaveCount"] = *qrampage->get_field<uint8_t>("_ExtraEmWaveNo");
            rampage["ExtraMonsterNandoTable"] = *qrampage->get_field<int8_t>("_ExtraEmNandoTblNo");
            rampage["ApexOrderTable"] = *qrampage->get_field<uint8_t>("_NushiOrderTblNo");
            rampage["WeaponUnlockTable"] = *qrampage->get_field<uint8_t>("_HmUnlockTblNo");

            if (const auto subtargets = *qrampage->get_field<API::ManagedObject*>("_SubTarget")) {
                for (int i = 0; i < 6; ++i) {
                    rampage["SubTargets"][i] = utility::call<uint8_t>(subtargets, "Get", i);
                }
            } else {
                rampage["SubTargets"] = nlohmann::ordered_json::array();
            }

            rampage["SubTarget5Wave"] = *qrampage->get_field<uint8_t>("_SubTarget5WaveNo");

            q["RampageData"] = rampage;
        } else {
            q["RampageData"] = nullptr;
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

