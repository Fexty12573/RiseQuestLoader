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
            std::string m_failure_condition;
        };

        std::map<GameLanguage, QuestInfo> m_quest_infos;
        GameLanguage m_fallback_language{GameLanguage::NONE};

        int32_t m_quest_id{0};

        reframework::API::ManagedObject* m_memory_object{};
        reframework::API::ManagedObject* m_original_object{};
        bool m_is_replacement{false};
        bool m_enabled{true};

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

            const auto& info = m_quest_infos.at(language);
            if (info.m_name.empty() && info.m_client.empty() && info.m_description.empty() && info.m_target.empty()) {
                return m_quest_infos.at(m_fallback_language);
            }

            return info;
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
                    info["Target"],
                    info.value("Fail", "")
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

    struct CustomSpawn {
        struct SpawnInfo {
            struct Vec3 {
                float X, Y, Z;
            };

            int32_t m_chance;
            int32_t m_block;
            int32_t m_id;
            Vec3* m_coordinates;
            reframework::API::ManagedObject* m_object;

            [[nodiscard]] bool has_custom_coordinates() const {
                return m_coordinates != nullptr;
            }

            void set_object(reframework::API::ManagedObject* obj) {
                if (m_object != nullptr) {
                    m_object->release();
                }

                obj->add_ref();
                m_object = obj;
            }

            SpawnInfo(int chance, int block, int id, const nlohmann::json& coords)
                : m_chance(chance)
                , m_block(block)
                , m_id(id)
                , m_object(nullptr) {
                if (coords.type() != nlohmann::detail::value_t::null) {
                    m_coordinates = new Vec3 {
                        coords.value("X", 0.0f),
                        coords.value("Y", 0.0f),
                        coords.value("Z", 0.0f)
                    };
                } else {
                    m_coordinates = nullptr;
                }
            }
            SpawnInfo(const SpawnInfo& info)
                : m_chance(info.m_chance), m_block(info.m_block), m_id(info.m_id) {
                m_coordinates = new Vec3(*info.m_coordinates);
                set_object(info.m_object);
            }
            SpawnInfo(SpawnInfo&& info) noexcept
                : m_chance(info.m_chance), m_block(info.m_block), m_id(info.m_id) {
                m_object = info.m_object;
                m_coordinates = info.m_coordinates;

                info.m_chance = 0;
                info.m_block = 0;
                info.m_id = 0;
                info.m_object = nullptr;
                info.m_coordinates = nullptr;
            }
            SpawnInfo& operator=(const SpawnInfo& info) {
                if (this != &info) {
                    m_chance = info.m_chance;
                    m_block = info.m_block;
                    m_id = info.m_id;
                    m_coordinates = new Vec3(*info.m_coordinates);
                    set_object(info.m_object);
                }

                return *this;
            }
            SpawnInfo& operator=(SpawnInfo&& info) noexcept {
                if (this != &info) {
                    m_chance = info.m_chance;
                    m_block = info.m_block;
                    m_id = info.m_id;
                    m_coordinates = info.m_coordinates;
                    m_object = info.m_object;

                    info.m_chance = 0;
                    info.m_block = 0;
                    info.m_id = 0;
                    info.m_coordinates = nullptr;
                    info.m_object = nullptr;
                }

                return *this;
            }
            ~SpawnInfo() {
                delete m_coordinates;
            }
        };
        struct SpawnSetter {
            std::string m_set_name;
            std::vector<SpawnInfo> m_spawn_infos;
            reframework::API::ManagedObject* m_object{nullptr};

            void set_object(reframework::API::ManagedObject* obj) {
                if (m_object != nullptr) {
                    m_object->release();
                }


                obj->add_ref();
                m_object = obj;
            }

            SpawnSetter() = default;
            SpawnSetter(const SpawnSetter& setter) {
                m_set_name = setter.m_set_name;
                m_spawn_infos = setter.m_spawn_infos;
                set_object(setter.m_object);
            }
            SpawnSetter(SpawnSetter&& setter) noexcept
                : m_set_name(std::move(setter.m_set_name)), m_spawn_infos(std::move(setter.m_spawn_infos)) {
                m_object = setter.m_object;

                setter.m_set_name = "";
                setter.m_spawn_infos = {};
                setter.m_object = nullptr;
            }
            SpawnSetter& operator=(const SpawnSetter& setter) {
                if (this != &setter) {
                    m_set_name = setter.m_set_name;
                    m_spawn_infos = setter.m_spawn_infos;
                    set_object(setter.m_object);
                }
                
                return *this;
            }
            SpawnSetter& operator=(SpawnSetter&& setter) noexcept {
                if (this != &setter) {
                    m_set_name = std::move(setter.m_set_name);
                    m_spawn_infos = std::move(setter.m_spawn_infos);
                    set_object(setter.m_object);

                    setter.m_set_name = "";
                    setter.m_spawn_infos = {};
                    setter.m_object = nullptr;
                }

                return *this;
            }
            ~SpawnSetter() {
                if (m_object != nullptr) {
                    m_object->release();
                }
            }
        };

        MapNoType m_map;
        std::vector<SpawnSetter> m_setters;
        int m_quest_id_lock = -1;

        [[nodiscard]] const SpawnSetter* get_spawn_setter(std::string_view name) const {
            for (const auto& setter : m_setters) {
                if (setter.m_set_name == name) {
                    return &setter;
                }
            }

            return nullptr;
        }
        [[nodiscard]] bool matches_quest(int id) const {
            if (m_quest_id_lock == -1) {
                return true;
            }

            return m_quest_id_lock == id;
        }

        explicit CustomSpawn(const nlohmann::json& j) {
            m_map = j["Map"];

            if (j.contains("QuestID")) {
                const auto& qid = j["QuestID"];

                if (qid.type() == nlohmann::detail::value_t::null) {
                    m_quest_id_lock = -1;
                } else {
                    m_quest_id_lock = qid.get<int>();
                }
            } else {
                m_quest_id_lock = -1;
            }
            

            for (const auto& setter : j["Setters"]) {
                SpawnSetter spawn_setter{};

                spawn_setter.m_set_name = setter["SetName"];

                for (const auto& spawn : setter["Spawns"]) {
                    spawn_setter.m_spawn_infos.emplace_back(
                        spawn.value("Chance", 0), 
                        spawn.value("Area", 0), 
                        spawn.value("SubSpawn", 0), 
                        spawn["Coordinates"]
                    );
                }

                m_setters.push_back(std::move(spawn_setter));
            }
        }
    };

    struct ResourceManager {
        [[nodiscard]] GameLanguage get_language() const {
            return *reinterpret_cast<const GameLanguage*>(reinterpret_cast<const uint8_t*>(this) + 0x43C);
        }
    };

    QuestLoader();

    static std::shared_ptr<QuestLoader> get();

    bool initialize();
    void read_quests();

    void render_ui();

private:
    void parse_quest(const std::filesystem::path& path);
    void parse_spawn(const std::filesystem::path& path);
    [[nodiscard]] bool is_existing_quest(int32_t quest_id) const;
    [[nodiscard]] reframework::API::ManagedObject* create_set_info_object(const CustomSpawn::SpawnSetter& setter) const;
    [[nodiscard]] reframework::API::ManagedObject* create_init_pos_object(const CustomSpawn::SpawnInfo& info) const;

    static SystemString* get_quest_text_hook(void* vmctx, reframework::API::ManagedObject* this_, QuestText type, void* qi);
    static reframework::API::ManagedObject* make_questno_list_hook(
        void* vmctx, reframework::API::ManagedObject* this_, reframework::API::ManagedObject* src, bool is_quick_match);
    static reframework::API::ManagedObject* make_quest_list_hyakuryu_hook(
        void* vmctx, reframework::API::ManagedObject* this_, reframework::API::ManagedObject* src, bool is_village, const int rank);
    static void quest_counter_awake_hook(void* vmctx, reframework::API::ManagedObject* this_);
    static void init_quest_data_dict_hook(void* vmctx, reframework::API::ManagedObject* this_);
    static const wchar_t* get_message_hook(void* this_, _GUID* guid, GameLanguage language);
    static bool is_single_quest_hook(void* vmctx, int32_t quest_id);
    static reframework::API::ManagedObject* find_boss_init_set_info_hook(
        void* vmctx, reframework::API::ManagedObject* this_, int em, MapNoType map, SystemString* set_name);
    static reframework::API::ManagedObject* find_boss_init_position_hook(
        void* vmctx, reframework::API::ManagedObject* this_, int block, int id, MapNoType map, bool is_safety);

private:
    bool m_initialized = false;
    QuestExporter m_quest_exporter{};
    ResourceManager* m_resource_manager{};

    reframework::API::TypeDefinition* m_quest_data{};
    reframework::API::TypeDefinition* m_normal_quest_data{};
    reframework::API::TypeDefinition* m_normal_quest_data_for_enemy{};
    reframework::API::TypeDefinition* m_rampage_data{};
    reframework::API::TypeDefinition* m_set_info{};
    reframework::API::TypeDefinition* m_lot_info{};
    reframework::API::TypeDefinition* m_init_pos{};

    reframework::API::ManagedObject* m_quest_counter{};
    CustomSpawn::SpawnSetter* m_last_spawn_setter{};

    bool m_skip_hook = false;

    reframework::API::ManagedObject* (*new_instance)(void*, void*, uint32_t){};

    constexpr static void* QuestDataClassInfo = reinterpret_cast<void*>(0x14bf02e10); 
    constexpr static void* NormalQuestDataClassInfo = reinterpret_cast<void*>(0x14bf02e10); 
    constexpr static void* NormalQuestDataForEnemyClassInfo = reinterpret_cast<void*>(0x14bf02e10);

    static constexpr const char* QuestExportPath = "./reframework/exported_quests";

    std::shared_ptr<utility::FunctionHook> m_get_quest_text_hook{};
    std::shared_ptr<utility::FunctionHook> m_make_questno_list_hook{};
    std::shared_ptr<utility::FunctionHook> m_make_quest_list_hyakuryu_hook{};
    std::shared_ptr<utility::FunctionHook> m_quest_counter_awake_hook{};
    std::shared_ptr<utility::FunctionHook> m_init_quest_data_dict_hook{};
    std::shared_ptr<utility::FunctionHook> m_get_message_hook{};
    std::shared_ptr<utility::FunctionHook> m_is_single_quest_hook{};
    std::shared_ptr<utility::FunctionHook> m_find_boss_init_set_info_hook{};
    std::shared_ptr<utility::FunctionHook> m_find_boss_init_position_hook{};

    std::unordered_map<int32_t, CustomQuest> m_custom_quests;
    std::vector<CustomSpawn> m_custom_spawns;
};
