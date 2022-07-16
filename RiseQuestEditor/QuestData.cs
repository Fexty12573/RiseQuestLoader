using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RiseQuestEditor
{
    [Flags]
    public enum QuestType
    {
        INVALID = 0,
        HUNTING = 1 << 0, //   1
        KILL = 1 << 1, //   2
        CAPTURE = 1 << 2, //   4
        BOSSRUSH = 1 << 3, //   8
        COLLECTS = 1 << 4, //  16 (Gathering)
        TOUR = 1 << 5, //  32
        ARENA = 1 << 6, //  64
        SPECIAL = 1 << 7, // 128
        HYAKURYU = 1 << 8, // 256
        TRAINING = 1 << 9, // 512
        KYOUSEI = 1 << 10  // 1024
    }

    public enum QuestLevel
    {
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
    }

    public enum EnemyLv
    {
        Village = 0,
        Low = 1,
        High = 2,
        Master = 3,
        Max = 4
    }

    public enum MapNoType : int
    {
        None = -1,
        No00 = 0,
        No01 = 1,
        No02 = 2,
        No03 = 3,
        No04 = 4,
        No05 = 5,
        No06 = 6,
        No07 = 7,
        No08 = 8,
        No09 = 9,
        No10 = 10,
        No11 = 11,
        No31 = 12,
        No32 = 13,
        No41 = 14,
        No42 = 15,
        No42_A = 16,
        Max = 17,
    }

    public enum QuestOrderType
    {
        None = 0,
        Under_2 = 1,
        H2 = 2,
        H3 = 3,
        H4 = 4,
        H5 = 5,
        H6 = 6,
        H7 = 7,
        H8 = 8,
        H20 = 9,
        H30 = 10,
        H40 = 11,
        H45 = 12,
        H50 = 13,
        H90 = 14,
        H100 = 15,
        M1 = 16,
        M2 = 17,
        M3 = 18,
        M4 = 19,
        M5 = 20,
        M6 = 21,
        M10 = 22,
        M20 = 23,
        M30 = 24,
        M40 = 25,
        M50 = 26,
        M60 = 27,
        M100 = 28,
        Only_1 = 29,
        MAX = 30,
    }

    public enum QuestTargetType
    {
        None = 0,
        ItemGet = 1,
        Hunting = 2,
        Kill = 3,
        Capture = 4,
        AllMainEnemy = 5,
        EmTotal = 6,
        FinalBarrierDefense = 7,
        FortLevelUp = 8,
        PlayerDown = 9,
        FinalBoss = 10,
        HuntingMachine = 11,
        DropItem = 12,
        EmStun = 13,
        EmElement = 14,
        EmCondition = 15,
        EmCnt_Weapon = 16,
        EmCnt_HmBallista = 17,
        EmCnt_HmCannon = 18,
        EmCnt_HmGatling = 19,
        EmCnt_HmTrap = 20,
        EmCnt_HmFlameThrower = 21,
        EmCnt_HmNpc = 22,
        EmCnt_HmDragnator = 23,
        ExtraEmRunaway = 24,
        Max = 25,
    }

    public enum SwapSetCondition : uint
    {
        None = 0,
        QuestTimer = 1,
    }

    public enum SwapStopType
    {
        None = 0,
        LowerHP = 1,
    }

    public enum SwapExecType
    {
        None = 0,
        FreeExtra = 1,
    }

    public enum BossSetCondition
    {
        NONE = 0,
        DEFAULT = 1,
        FREE_1 = 2,
        FREE_2 = 3,
        FREE_3 = 4,
        TIMER1 = 5,
        TIMER2 = 6,
        EM1_HP = 7,
        EM2_HP = 8,
        EM3_HP = 9,
        EM4_HP = 10,
        EM5_HP = 11,
        HP_EMx1 = 12,
        HP_EMx2 = 13,
        INIT_RANDOM = 14,
        SWAP_RANDOM = 15,
        FSM_CONTROL = 16,
        ENTRY_TIME = 17,
    }

    public enum EmsSetNo
    {
        ems_set_none = 0,
        m01_ems_set_base = 1,
        m01_ems_set_tour = 2,
        m01_ems_set_tutorial = 3,
        m02_ems_set_base = 4,
        m02_ems_set_tour = 5,
        m02_ems_set_QN000313 = 6,
        m02_ems_set_QN000412 = 7,
        m02_ems_set_QN010212 = 8,
        m02_ems_set_QN010516 = 9,
        m03_ems_set_base = 10,
        m03_ems_set_tour = 11,
        m03_ems_set_QN000315 = 12,
        m03_ems_set_QN010213 = 13,
        m03_ems_set_QN010517 = 14,
        m04_ems_set_base = 15,
        m04_ems_set_tour = 16,
        m04_ems_set_QN000205 = 17,
        m04_ems_set_QN000209 = 18,
        m04_ems_set_QN010112 = 19,
        m04_ems_set_QN010412 = 20,
        m05_ems_set_base = 21,
        m05_ems_set_tour = 22,
        m01_ems_set_QN000104 = 23,
        m01_ems_set_QN000105 = 24,
        m01_ems_set_QN000208 = 25,
        m01_ems_set_QN000310 = 26,
        m05_ems_set_QN000410 = 27,
        m05_ems_set_QN000414 = 28,
        m01_ems_set_QN010111 = 29,
        m01_ems_set_QN010418 = 30,
        m05_ems_set_QN010617 = 31,
        m21_ems_set_base = 32,
        m31_ems_set_base = 33,
        m32_ems_set_base = 34,
        m01_ems_set_base_MR = 35,
        m01_ems_set_tour_MR = 36,
        m02_ems_set_base_MR = 37,
        m02_ems_set_tour_MR = 38,
        m03_ems_set_base_MR = 39,
        m03_ems_set_tour_MR = 40,
        m03_ems_set_QN315211 = 41,
        m03_ems_set_QN315300 = 42,
        m05_ems_set_base_MR = 43,
        m05_ems_set_tour_MR = 44,
        m05_ems_set_QN315412 = 45,
        m31_ems_set_tour = 46,
        m31_ems_set_QN315101 = 47,
        m31_ems_set_QN315107 = 48,
        m32_ems_set_tour = 49,
        m32_ems_set_QN315314 = 50,
    }

    public enum NandoYuragi : int
    {
        False = 0,
        True1 = 1,
        True2 = 2,
    }

    [Flags]
    public enum RampageAttr
    {
        None = 0,
        FixWaveOrder = 1,
        Lot_HighEx = 2,
        Lot_TrueED = 4,
        FinalBossKill = 8,
    }

    public enum RampageCategory
    {
        Normal = 0,
        Nushi = 1,
        Max = 2,
    }

    public enum EnemyIndividualType : int
    {
        Normal = 0,
        Mystery = 1
    }

    namespace via.gui.MessageTag
    {
        public enum Type
        {
            Unknown = 0,
            Page = 1,               // <PAGE>
            Line = 2,               // <LINE>
            Size = 3,               // <SIZE #></SIZE>
            Font = 4,               // <FONT ...></FONT>
            Color = 5,              // <COLOR></COLOR>
            Char = 6,               // <CHAR ???>
            Space = 7,              // <BLANK>
            Wordwrap = 8,           // <WRAP>
            Center = 9,             // <CENTER></CENTER>
            Left = 10,              // <LEFT></LEFT>
            Right = 11,             // <RIGHT></RIGHT>
            Top = 12,               // <TOP></TOP>
            Bottom = 13,            // <BOTTOM></BOTTOM>
            Time = 14,              // <TIME>
            Ruby = 15,              // <RUBY></RUBY>
            RubyB = 16,             // <RUBYB></RUBYB>
            RubyRB = 17,            // <RB></RB>
            RubyRT = 18,            // <RT></RT>
            Event = 19,             // <EVENT ???>
            TCU = 20,               // <TCU></TCU>
            Draw = 21,              // ???
            GlowDraw = 22,          // ???
            ShadowDraw = 23,        // ???
            DistanceFieldDraw = 24, // ???
            Sprite = 25,            // <SPRITE ???>
            Baseline = 26,          // <BSL></BSL>
            Glow = 27,              // <GLOW></GLOW>
            Shadow = 28,            // <SHADOW></SHADOW>
            DistanceField = 29,     // ???
            Italic = 30,            // <ITALIC></ITALIC>
            Bold = 31,              // <BOLD></BOLD>
        }
        
        // Other:
        // <LSNR {M}{F}>
        // <SPKR {M}{F}>
        // <REF ???/>
        // <ICON NAME>
    }
}
