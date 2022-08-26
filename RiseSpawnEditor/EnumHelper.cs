using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace RiseSpawnEditor
{
    class EnumHelper
    {
        public struct SpawnInfo
        {
            public double X { get; set; }
            public double Y { get; set; }
            public double Z { get; set; }
            public int Area { get; set; }
            public int UnqId { get; set; }
        }
        public struct MapSpawn
        {
            public int Map { get; set; }
            public List<SpawnInfo> Positions { get; set; }
        }
        
        public static readonly Dictionary<int, string> Map;
        public static readonly List<MapSpawn> Spawns;
        
        static EnumHelper()
        {
            Map = DeserializeDictionary<int, string>("Map.json");
            Spawns = DeserializeList<MapSpawn>("Assets/SpawnData.json");
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
