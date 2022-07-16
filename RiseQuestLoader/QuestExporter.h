#pragma once

#include "Language.h"
#include "Quest.h"

#include <vector>
#include <nlohmann/json.hpp>

#include "reframework/API.hpp"
#include "reframework/utility.h"


class QuestExporter {
public:
	QuestExporter();

	bool initialize();

	[[nodiscard]] nlohmann::ordered_json export_quest(int32_t quest_id);
    [[nodiscard]] std::vector<nlohmann::ordered_json> export_all_quests();

	[[nodiscard]] auto get_quest_manager() const { return m_quest_manager; }
    [[nodiscard]] auto get_message_manager() const { return m_message_manager; }
    [[nodiscard]] auto get_override_language() const { return m_override_language; }

private:
	bool m_initialized = false;
	reframework::API::ManagedObject* m_quest_manager{};
    reframework::API::ManagedObject* m_message_manager{};
	reframework::API::Method* m_get_quest_data{};

    GameLanguage m_override_language{GameLanguage::NONE};
};

