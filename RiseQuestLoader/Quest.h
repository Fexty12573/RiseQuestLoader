#pragma once

enum class QuestCounterTopMenuType : int32_t {
    Normal_Vil = 0,
    Normal_Hall_High = 1,
    Normal_Hall_Low = 2,
    Hyakuryu = 3,
    Training = 4,
    Event = 5,
    FreeSide = 6,
    Arena = 7,
    Challenge = 8,
    Talk = 9,
    Urgent = 10,
    Special = 11,
    Max = 12,
    None = 13,
};

enum class QuestText {
    TITLE = 0x0,
    CLIENT = 0x1,
    REQUEST = 0x2,
    TARGET = 0x3,
    MISS = 0x4,
    ENV_INFO1 = 0x5,
    ENV_INFO2 = 0x6,
    ECOLOGY1 = 0x7,
    ECOLOGY2 = 0x8,
    ORDER_TYPE1 = 0x9,
    ORDER_TYPE2 = 0xA,
    EM_INFO = 0xB,
    MAP = 0xC,
    HYAKU_INFO = 0xD,
    HYAKU_INFO2 = 0xE,
    HYAKU_REWARD = 0xF,
};

enum class QuestType : uint32_t {
    INVALID     = 0,
    HUNTING     = 1 << 0, //   1
    KILL        = 1 << 1, //   2
    CAPTURE     = 1 << 2, //   4
    BOSSRUSH    = 1 << 3, //   8
    COLLECTS    = 1 << 4, //  16 (Gathering)
    TOUR        = 1 << 5, //  32
    ARENA       = 1 << 6, //  64
    SPECIAL     = 1 << 7, // 128
    HYAKURYU    = 1 << 8, // 256
    TRAINING    = 1 << 9, // 512
};

constexpr bool operator&(QuestType l, QuestType r) {
    return (static_cast<uint32_t>(l) & static_cast<uint32_t>(r)) > 0;
}

constexpr QuestType operator|(QuestType l, QuestType r) {
    return static_cast<QuestType>(static_cast<uint32_t>(l) | static_cast<uint32_t>(r));
}

enum class QuestLevel : int32_t {
    QL1 = 0,
    QL2 = 1,
    QL3 = 2,
    QL4 = 3,
    QL5 = 4,
    QL6 = 5,
    QL7 = 6,
    QL7EX = 7,
    MAX = 8,
    INVALID = -1,
    QL_V_LOW_START = 0,
    QL_V_LOW_END = 5,
    QL_H_LOW_START = 0,
    QL_H_LOW_END = 2,
    QL_H_HIGH_START = 3,
    QL_H_HIGH_END = 6,
    QL_H_HIGH_END_EX = 7,
};

enum class EnemyLv : uint32_t {
    Village = 0,
    Low = 1,
    High = 2,
    Max = 3,
};

enum class QuestCounterLevelMenuType : int32_t {
    Lv_01 = 0,
    Lv_02 = 1,
    Lv_03 = 2,
    Lv_04 = 3,
    Lv_05 = 4,
    Lv_06 = 5,
    Lv_07 = 6,
    Gathering = 7,
    Random = 8,
    Max = 9,
    None = 10,
};

enum class QuestRank : uint32_t {
    Low = 0,
    High = 1,
    Max = 2,
};
