#pragma once

#include "FunctionHook.h"
#include "Quest.h"
#include "QuestExporter.h"

#include <filesystem>
#include <memory>
#include <unordered_map>


class QuestLoader {
public:
    struct CustomQuest {
        struct QuestInfo {
            std::string m_name;
            std::string m_client;
            std::string m_description;
            std::string m_target;
        };

        std::map<GameLanguage, QuestInfo> m_quest_infos;
        GameLanguage m_fallback_language{GameLanguage::NONE};

        int32_t m_quest_id{0};

        reframework::API::ManagedObject* m_memory_object{};
        reframework::API::ManagedObject* m_original_object{};
        bool m_is_replacement;
        bool m_enabled;

        void enable(reframework::API::ManagedObject* questdict) {
            if (m_is_replacement) {
                m_enabled = true;
                utility::call(questdict, "set_Item", m_quest_id, m_memory_object);
            }
        }

        void disable(reframework::API::ManagedObject* questdict) {
            if (m_is_replacement) {
                m_enabled = false;
                utility::call(questdict, "set_Item", m_quest_id, m_original_object);
            }
        }

        // Only call right before object is destroyed
        void cleanup(reframework::API::ManagedObject* questdict) {
            if (m_is_replacement) {
                m_enabled = false;
                
                utility::call(questdict, "set_Item", m_quest_id, m_original_object);
            } else {
                utility::call(questdict, "Remove", m_quest_id);
            }
        }

        [[nodiscard]] const QuestInfo& get_quest_info(GameLanguage language = GameLanguage::NONE) const {
            if (language == GameLanguage::NONE || !m_quest_infos.contains(language)) {
                return m_quest_infos.at(m_fallback_language);
            }

            return m_quest_infos.at(language);
        }

        CustomQuest() = default;
        CustomQuest(const nlohmann::json& j, reframework::API::ManagedObject* quest, reframework::API::ManagedObject* original)
            : m_memory_object(quest), m_original_object(original) {
            m_memory_object->add_ref();
            if (m_original_object) {
                m_original_object->add_ref();
                m_is_replacement = true;
                m_enabled = true;
            } else {
                m_is_replacement = false;
                m_enabled = false;
            }

            m_quest_id = j.value("QuestID", 0);
            m_fallback_language = language::get_language(j["QuestText"]["FallbackLanguage"]);

            for (const auto& info : j["QuestText"]["QuestInfo"]) {
                m_quest_infos[language::get_language(info["Language"])] = {
                    info["Name"],
                    info["Client"],
                    info["Description"],
                    info["Target"]
                };
            }
        }
        ~CustomQuest() {
            m_memory_object->release();
            if (m_original_object) {
                m_original_object->release();
            }
        }
    };

    QuestLoader();

    static std::shared_ptr<QuestLoader> get();

    bool initialize();
    void read_quests();

    void render_ui();

private:
    void parse_quest(const std::filesystem::path& path);
    [[nodiscard]] bool is_existing_quest(int32_t quest_id) const;

    static SystemString* get_quest_text_hook(void* vmctx, reframework::API::ManagedObject* this_, QuestText type, void* qi);
    static reframework::API::ManagedObject* make_questno_list_hook(
        void* vmctx, reframework::API::ManagedObject* this_, reframework::API::ManagedObject* src, bool is_quick_match);
    static reframework::API::ManagedObject* make_quest_list_hyakuryu_hook(
        void* vmctx, reframework::API::ManagedObject* this_, reframework::API::ManagedObject* src, bool is_village, const int rank);
    static void quest_counter_awake_hook(void* vmctx, reframework::API::ManagedObject* this_);
    static void init_quest_data_dict_hook(void* vmctx, reframework::API::ManagedObject* this_);
    static const wchar_t* get_message_hook(void* this_, _GUID* guid, GameLanguage language);

private:
    bool m_initialized = false;
    QuestExporter m_quest_exporter{};

    reframework::API::TypeDefinition* m_quest_data{};
    reframework::API::TypeDefinition* m_normal_quest_data{};
    reframework::API::TypeDefinition* m_normal_quest_data_for_enemy{};
    reframework::API::TypeDefinition* m_rampage_data{};

    reframework::API::ManagedObject* m_quest_counter{};

    bool m_skip_hook = false;

    reframework::API::ManagedObject* (*new_instance)(void*, void*, uint32_t){};

    constexpr static void* QuestDataClassInfo = reinterpret_cast<void*>(0x14bf02e10); 
    constexpr static void* NormalQuestDataClassInfo = reinterpret_cast<void*>(0x14bf02e10); 
    constexpr static void* NormalQuestDataForEnemyClassInfo = reinterpret_cast<void*>(0x14bf02e10);

    std::shared_ptr<utility::FunctionHook> m_get_quest_text_hook{};
    std::shared_ptr<utility::FunctionHook> m_make_questno_list_hook{};
    std::shared_ptr<utility::FunctionHook> m_make_quest_list_hyakuryu_hook{};
    std::shared_ptr<utility::FunctionHook> m_quest_counter_awake_hook{};
    std::shared_ptr<utility::FunctionHook> m_init_quest_data_dict_hook{};
    std::shared_ptr<utility::FunctionHook> m_get_message_hook{};

    std::unordered_map<int32_t, CustomQuest> m_custom_quests;
};
