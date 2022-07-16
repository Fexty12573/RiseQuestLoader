#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class GameLanguage : int32_t {
    NONE = -1,
    JPN = 0,
    ENG = 1,
    FRE = 2,
    ITA = 3,
    GER = 4,
    SPA = 5,
    RUS = 6,
    POL = 7,
    POR = 10,
    KOR = 11,
    ZHT = 12,
    ZHS = 13,
    ARA = 21
};

namespace language {

const inline std::map<GameLanguage, std::string> s_language_identifiers = {
    { GameLanguage::NONE, "N/A" },
	{ GameLanguage::JPN, "JPN" },
    { GameLanguage::ENG, "ENG" },
    { GameLanguage::FRE, "FRE" },
    { GameLanguage::ITA, "ITA" },
    { GameLanguage::GER, "GER" },
    { GameLanguage::SPA, "SPA" },
    { GameLanguage::RUS, "RUS" },
    { GameLanguage::POL, "POL" },
    { GameLanguage::POR, "POR" },
    { GameLanguage::KOR, "KOR" },
    { GameLanguage::ZHT, "ZHT" },
    { GameLanguage::ZHS, "ZHS" },
    { GameLanguage::ARA, "ARA" }
};

inline std::string_view get_language_identifier(GameLanguage language) {
    return s_language_identifiers.at(language);
}

inline GameLanguage get_language(std::string_view identifier) {
    for (const auto& [lang, id] : s_language_identifiers) {
        if (id == identifier) {
            return lang;
        }
    }

    return GameLanguage::NONE;
}

}
