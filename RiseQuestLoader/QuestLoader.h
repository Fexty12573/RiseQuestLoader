#pragma once

#include "FunctionHook.h"
#include "QuestExporter.h"
#include "Quest.h"

#include <filesystem>
#include <memory>
#include <unordered_map>


class QuestLoader {
public:
    struct CustomQuest {
        std::string m_name;
        std::string m_client;
        std::string m_description;
        std::string m_target;

        reframework::API::ManagedObject* m_memory_object{};

        CustomQuest() = default;
        CustomQuest(const nlohmann::json& j, reframework::API::ManagedObject* quest)
            : m_memory_object(quest) {
            m_name = j["QuestText"]["Name"];
            m_client = j["QuestText"]["Client"];
            m_description = j["QuestText"]["Description"];
            m_target = j["QuestText"]["Target"];
        }
        ~CustomQuest() { m_memory_object->release(); }
    };

    QuestLoader();

    static std::shared_ptr<QuestLoader> get();

    bool initialize();
    void read_quests();

    void render_ui();

private:
    void parse_quest(const std::filesystem::path& path);

    static SystemString* get_quest_text_hook(void* vmctx, reframework::API::ManagedObject* this_, QuestText type, void* qi);
    static reframework::API::ManagedObject* make_questno_list_hook(
        void* vmctx, reframework::API::ManagedObject* this_, reframework::API::ManagedObject* src, bool is_quick_match);
    static reframework::API::ManagedObject* make_quest_list_hyakuryu_hook(
        void* vmctx, reframework::API::ManagedObject* this_, reframework::API::ManagedObject* src, bool is_village, const int rank);
    static void quest_counter_awake_hook(void* vmctx, reframework::API::ManagedObject* this_);
    static void init_quest_data_dict_hook(void* vmctx, reframework::API::ManagedObject* this_);

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

    std::unordered_map<int32_t, CustomQuest> m_custom_quests;
};
