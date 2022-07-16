using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.IO;
using System.Windows;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace RiseQuestEditor
{
    public class CustomQuest
    {
        public int QuestID { get; set; }
        public QuestText? QuestText { get; set; }
        public QuestData? QuestData { get; set; }
        public EnemyData? EnemyData { get; set; }
        public RampageData? RampageData { get; set; }

        public bool IsRampageQuest()
        {
            return RampageData != null;
        }

        public CustomQuest()
        {
            QuestID = 0;
            QuestText = new QuestText();
            QuestData = new QuestData();
            EnemyData = new EnemyData();
            RampageData = null;
        }
    }

    public class ByteArrayJsonConverter : JsonConverter
    {
        public override void WriteJson(JsonWriter writer, object? value, Newtonsoft.Json.JsonSerializer serializer)
        {
            if (value != null)
            {
                byte[] bytes = (byte[])value;
                writer.WriteStartArray();

                foreach (byte b in bytes)
                {
                    writer.WriteValue((int)b);
                }

                writer.WriteEndArray();
            }
        }

        public override object? ReadJson(JsonReader reader, Type objectType, object? existingValue, Newtonsoft.Json.JsonSerializer serializer)
        {
            throw new NotImplementedException();
        }

        public override bool CanRead
        {
            get { return false; }
        }

        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(byte[]);
        }
    }

    public class QuestText
    {
        public struct QuestInfo_
        {
            public string? Language;
            public string? Name;
            public string? Client;
            public string? Description;
            public string? Target;

            public QuestInfo_(string indentifier)
            {
                Language = indentifier;
                Name = "";
                Client = "";
                Description = "";
                Target = "";
            }
        }

        public List<QuestInfo_> QuestInfo;
        public string? FallbackLanguage;
        public string? DebugName;
        public string? DebugClient;
        public string? DebugDescription;

        public QuestText()
        {
            QuestInfo = new List<QuestInfo_>();

            FallbackLanguage = "ENG";
            DebugName = "";
            DebugClient = "";
            DebugDescription = "";
        }
    }

    public class QuestData
    {
        public struct Monster
        {
            public uint Id;
            public BossSetCondition SpawnCondition;
            public uint SpawnParam;

            public Monster()
            {
                Id = 0;
                SpawnCondition = BossSetCondition.NONE;
                SpawnParam = 0;
            }
        }
        public struct Rewards
        {
            public uint Zenny;
            public uint Points;
            public uint HRP;

            public Rewards()
            {
                Zenny = Points = HRP = 0;
            }
        }
        public struct ArenaParameters
        {
            public bool FenceDefaultActive;
            public ushort FenceUptime;
            public ushort FenceInitialDelay;
            public ushort FenceCooldown;
            public bool[] Pillars;

            public ArenaParameters()
            {
                FenceDefaultActive = false;
                FenceUptime = FenceInitialDelay = FenceCooldown = 0;
                Pillars = new bool[3] { false, false, false };
            }
        }

        public QuestType QuestType { get; set; }
        public QuestLevel QuestLevel { get; set; }
        public EnemyLv EnemyLevel { get; set; }
        public MapNoType Map { get; set; }
        public uint BaseTime { get; set; }
        public uint TimeVariation { get; set; }
        public uint TimeLimit { get; set; }
        public uint Carts { get; set; }
        public QuestOrderType[] QuestConditions { get; set; }
        public QuestTargetType[] TargetTypes { get; set; }
        public uint[] TargetMonsters { get; set; }
        public uint[] TargetItemIds { get; set; }
        public uint[] TargetAmounts { get; set; }
        public Monster[] Monsters { get; set; }
        public uint ExtraMonsterCount { get; set; }
        public bool SwapExitRide { get; set; }
        public byte[] SwapFrequencies { get; set; }
        public SwapSetCondition[] SwapConditions { get; set; }
        public byte[] SwapParams { get; set; }
        public byte[] SwapExitTimes { get; set; }
        public SwapStopType SwapStopType { get; set; }
        public byte SwapStopParam { get; set; }
        public SwapExecType SwapExecType { get; set; }
        public Rewards Reward { get; set; }
        public uint SupplyTable { get; set; }
        public int[] Icons { get; set; }
        public bool Tutorial { get; set; }
        public bool FromNpc { get; set; }
        public ArenaParameters ArenaParam { get; set; }
        public ushort AutoMatchHR { get; set; }
        public int BattleBGMType { get; set; }
        public int ClearBGMType { get; set; }

        public QuestData()
        {
            Reward = new Rewards();
            ArenaParam = new ArenaParameters();
            QuestConditions = new QuestOrderType[2];
            TargetTypes = new QuestTargetType[2];
            TargetMonsters = new uint[2];
            TargetItemIds = new uint[2];
            TargetAmounts = new uint[2];
            Monsters = new Monster[7]
            {
                new Monster(),
                new Monster(),
                new Monster(),
                new Monster(),
                new Monster(),
                new Monster(),
                new Monster()
            };
            SwapFrequencies = new byte[2];
            SwapConditions = new SwapSetCondition[2];
            SwapParams = new byte[2];
            SwapExitTimes = new byte[2];
            Icons = new int[5] { 999, 999, 999, 999, 999 };
        }
    }

    public class EnemyData
    {
        public struct SmallMonsterData
        {
            public EmsSetNo SpawnType { get; set; }
            public byte HealthTable { get; set; }
            public byte AttackTable { get; set; }
            public byte PartTable { get; set; }
            public byte OtherTable { get; set; }
            public byte MultiTable { get; set; }

            public SmallMonsterData()
            {
                SpawnType = EmsSetNo.ems_set_none;
                HealthTable = 0;
                AttackTable = 0;
                PartTable = 0;
                OtherTable = 0;
                MultiTable = 0;
            }
        }
        public struct MonsterData
        {
            public byte PathId { get; set; }
            public string SetName { get; set; }
            public byte SubType { get; set; }
            public EnemyIndividualType IndividualType { get; set; }
            public ushort HealthTable { get; set; }
            public ushort AttackTable { get; set; }
            public ushort PartTable { get; set; }
            public ushort OtherTable { get; set; }
            public byte StaminaTable { get; set; }
            public byte Size { get; set; }
            public int SizeTable { get; set; }
            public NandoYuragi Difficulty { get; set; }
            public byte MultiTable { get; set; }

            public MonsterData()
            {
                PathId = 0;
                SetName = "";
                SubType = 0;
                IndividualType = EnemyIndividualType.Normal;
                HealthTable = 0;
                AttackTable = 0;
                PartTable = 0;
                OtherTable = 0;
                StaminaTable = 0;
                Size = 100;
                SizeTable = 0;
                Difficulty = NandoYuragi.False;
                MultiTable = 0;
            }
        }

        public SmallMonsterData SmallMonsters { get; set; }
        public MonsterData[] Monsters { get; set; }

        public EnemyData()
        {
            Monsters = new MonsterData[7]
            {
                new MonsterData(),
                new MonsterData(),
                new MonsterData(),
                new MonsterData(),
                new MonsterData(),
                new MonsterData(),
                new MonsterData()
            };
        }
    }

    public class RampageData
    {
        public struct WaveData
        {
            public uint BossMonster;
            public uint BossSubType;
            public int OrderTable;
            public int BossMonsterNandoTable;
            public int WaveMonsterNandoTable;
            public uint[] Monsters;

            public WaveData()
            {
                BossMonster = 0;
                BossSubType = 0;
                OrderTable = 0;
                BossMonsterNandoTable = -1;
                WaveMonsterNandoTable = -1;
                Monsters = new uint[4];
            }
        }

        public int Seed { get; set; }
        public RampageAttr QuestAttr { get; set; }
        public WaveData[] Waves { get; set; }
        public QuestLevel QuestLevel { get; set; }
        public MapNoType Map { get; set; }
        public RampageCategory Category { get; set; }
        public bool IsVillage { get; set; }
        public uint BaseTime { get; set; }
        public byte StartBlock { get; set; }
        public byte EndBlock { get; set; }
        public byte ExtraWaveCount { get; set; }
        public sbyte ExtraMonsterNandoTable { get; set; }
        public byte ApexOrderTable { get; set; }
        public byte WeaponUnlockTable { get; set; }
        public QuestTargetType[] SubTargets { get; set; }
        public byte SubTarget5Wave { get; set; }

        public RampageData()
        {
            Waves = new WaveData[3]
            {
                new WaveData(),
                new WaveData(),
                new WaveData()
            };
            SubTargets = new QuestTargetType[6];
        }
    }
}
