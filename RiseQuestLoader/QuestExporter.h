#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "reframework/API.hpp"
#include "reframework/utility.h"


class QuestExporter {
public:
	QuestExporter();

	bool initialize();

	[[nodiscard]] nlohmann::ordered_json export_quest(int32_t quest_id) const;
    [[nodiscard]] std::vector<nlohmann::ordered_json> export_all_quests() const;

	[[nodiscard]] auto get_quest_manager() const { return m_quest_manager; }

private:
	bool m_initialized = false;
	reframework::API::ManagedObject* m_quest_manager{};
	reframework::API::Method* m_get_quest_data{};
};

