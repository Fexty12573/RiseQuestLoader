using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.IO;
using Newtonsoft.Json;

namespace RiseQuestEditor
{
    public struct EntryInfo
    {
        public string Name { get; set; }
        public int Id { get; set; }
    }

    public struct IconInfo
    {
        public string Name { get; set; }
        public string Icon { get; set; }
        public int Id { get; set; }
    }

    public struct OtherInfo
    {
        public string Defense { get; set; }
        public string ElementA { get; set; }
        public string ElementB { get; set; }
        public string Stun { get; set; }
        public string Exhaust { get; set; }
        public string WyvernRide { get; set; }
    }

    public struct MultiInfo
    {
        public string[] Health { get; set; }
        public string[] Attack { get; set; }
        public string[] PartHP { get; set; }
        public string[] OtherParts { get; set; }
        public string[] MultiParts { get; set; }
        public string[] Defense { get; set; }
        public string[] ElementA { get; set; }
        public string[] ElementB { get; set; }
        public string[] Stun { get; set; }
        public string[] Exhaust { get; set; }
        public string[] WyvernRide { get; set; }
        public string[] Monster2MonsterAttack { get; set; }

        public MultiInfo()
        {
            Health = new string[3];
            Attack = new string[3];
            PartHP = new string[3];
            OtherParts = new string[3];
            MultiParts = new string[3];
            Defense = new string[3];
            ElementA = new string[3];
            ElementB = new string[3];
            Stun = new string[3];
            Exhaust = new string[3];
            WyvernRide = new string[3];
            Monster2MonsterAttack = new string[3];
        }
    }

    public struct Language
    {
        public int Id { get; set; }
        public string Identifier { get; set; }
        public string Name { get; set; }
    }

    public class EnumHelper
    {
        public static readonly Dictionary<int, string> EmsSetNo;
        public static readonly Dictionary<int, string> EnemyLevel;
        public static readonly Dictionary<int, string> Map;
        public static readonly Dictionary<int, string> MonsterIconName;
        public static readonly Dictionary<int, string> MonsterIconFile;
        public static readonly Dictionary<int, string> Monster;
        public static readonly Dictionary<int, string> NandoYuragi;
        public static readonly Dictionary<int, string> QuestCondition;
        public static readonly Dictionary<int, string> QuestLevel;
        public static readonly Dictionary<int, string> QuestObjective;
        public static readonly Dictionary<int, string> RampageCategory;
        public static readonly Dictionary<int, string> SpawnCondition;
        public static readonly Dictionary<int, string> SwapCondition;
        public static readonly Dictionary<int, string> SwapExecType;
        public static readonly Dictionary<int, string> SwapStopType;
        public static readonly Dictionary<int, string> BattleBGMType;
        public static readonly Dictionary<int, string> ClearBGMType;
        public static readonly Dictionary<int, string> Item;
        public static readonly Dictionary<int, string> IndividualType;

        public static readonly Dictionary<int, string> HealthRate;
        public static readonly Dictionary<int, string> AttackRate;
        public static readonly Dictionary<int, string> PartsRate;
        public static readonly Dictionary<int, OtherInfo> OtherRate;
        public static readonly Dictionary<int, MultiInfo> MultiRate;
        public static readonly Dictionary<int, Language> Languages;

        static EnumHelper()
        {
            EmsSetNo = DeserializeDictionary<int, string>("Assets/EmsSetNo.json");
            EnemyLevel = DeserializeDictionary<int, string>("Assets/EnemyLevel.json");
            Map = DeserializeDictionary<int, string>("Assets/Map.json");
            NandoYuragi = DeserializeDictionary<int, string>("Assets/NandoYuragi.json");
            QuestCondition = DeserializeDictionary<int, string>("Assets/QuestCondition.json");
            QuestLevel = DeserializeDictionary<int, string>("Assets/QuestLevel.json");
            QuestObjective = DeserializeDictionary<int, string>("Assets/QuestObjective.json");
            RampageCategory = DeserializeDictionary<int, string>("Assets/RampageCategory.json");
            SpawnCondition = DeserializeDictionary<int, string>("Assets/SpawnCondition.json");
            SwapCondition = DeserializeDictionary<int, string>("Assets/SwapCondition.json");
            SwapExecType = DeserializeDictionary<int, string>("Assets/SwapExecType.json");
            SwapStopType = DeserializeDictionary<int, string>("Assets/SwapStopType.json");
            BattleBGMType = DeserializeDictionary<int, string>("Assets/BattleBGMTypes.json");
            ClearBGMType = DeserializeDictionary<int, string>("Assets/QuestClearBGMTypes.json");
            IndividualType = DeserializeDictionary<int, string>("Assets/IndividualType.json");

            MonsterIconName = new Dictionary<int, string>();
            MonsterIconFile = new Dictionary<int, string>();
            Monster = new Dictionary<int, string>();
            Item = new Dictionary<int, string>();
            Languages = new Dictionary<int, Language>();

            HealthRate = DeserializeDictionary<int, string>("Assets/VitalRate.json");
            AttackRate = DeserializeDictionary<int, string>("Assets/AttackRate.json");
            PartsRate = DeserializeDictionary<int, string>("Assets/PartsRate.json");
            OtherRate = DeserializeDictionary<int, OtherInfo>("Assets/OtherRate.json");
            MultiRate = DeserializeDictionary<int, MultiInfo>("Assets/MultiRate.json");


            ParseMonsterIcons();
            ParseMonsters();
            ParseItems();
            ParseLanguages();
        }

        public static int GetLanguageId(string identifier)
        {
            return GetLanguage(identifier).Id;
        }

        public static int GetLanguageIndex(string identifier)
        {
            foreach (var language in Languages)
            {
                if (language.Value.Identifier == identifier)
                {
                    return language.Key;
                }
            }

            throw new Exception($"Unknown language identifier '{identifier}'");
        }

        public static Language GetLanguage(string identifier)
        {
            foreach (var language in Languages)
            {
                if (language.Value.Identifier == identifier)
                {
                    return language.Value;
                }
            }

            throw new Exception($"Unknown language identifier '{identifier}'");
        }

        private static void ParseMonsterIcons()
        {
            var rawIcons = DeserializeList<IconInfo>("Assets/MonsterIcons.json");

            foreach (IconInfo icon in rawIcons)
            {
                MonsterIconName[icon.Id] = icon.Name;
                MonsterIconFile[icon.Id] = $"Assets/Icons/{icon.Icon}_IAM.png";
            }
        }

        private static void ParseMonsters()
        {
            var rawMonsters = DeserializeList<EntryInfo>("Assets/Monsters.json");

            foreach (EntryInfo monster in rawMonsters)
            {
                Monster[monster.Id] = monster.Name;
            }
        }

        private static void ParseItems()
        {
            var rawItems = DeserializeList<EntryInfo>("Assets/Items.json");

            foreach (EntryInfo item in rawItems)
            {
                Item[item.Id] = item.Name;
            }
        }

        private static void ParseLanguages()
        {
            var rawLangs = DeserializeList<Language>("Assets/Languages.json");

            int i = 0;
            foreach (Language lang in rawLangs)
            {
                Languages[i] = lang;
                ++i;
            }
        }

        private static Dictionary<K, V> DeserializeDictionary<K, V>(string file) where K : notnull
        {
            using StreamReader sr = GetAsset(file);
            return JsonConvert.DeserializeObject<Dictionary<K, V>>(sr.ReadToEnd())!;
        }

        private static List<T> DeserializeList<T>(string file)
        {
            using StreamReader sr = GetAsset(file);
            return JsonConvert.DeserializeObject<List<T>>(sr.ReadToEnd())!;
        }

        private static StreamReader GetAsset(string path)
        {
            Uri uri = new(path, UriKind.Relative);
            return new StreamReader(Application.GetResourceStream(uri).Stream, Encoding.UTF8);
        }
    }
}
