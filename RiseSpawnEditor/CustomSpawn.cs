using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RiseSpawnEditor
{
    public class CustomSpawn
    {
        public int QuestID { get; set; }
        public int Map { get; set; }
        public List<SpawnSetter> Setters { get; set; }

        public CustomSpawn()
        {
            QuestID = -1;
            Map = 0;
            Setters = new List<SpawnSetter>();
        }
    }

    public class SpawnSetter
    {
        public string? SetName;
        public IndividualSpawn[] Spawns { get; set; }
        
        public SpawnSetter(string name = "")
        {
            SetName = name;
            Spawns = new IndividualSpawn[3]
            {
                new IndividualSpawn(),
                new IndividualSpawn(),
                new IndividualSpawn()
            };
        }
    }

    public class IndividualSpawn
    {
        public class Position
        {
            public double X { get; set; }
            public double Y { get; set; }
            public double Z { get; set; }

            public Position()
            {
                X = Y = Z = 0.0;
            }
        }
        
        public int Chance { get; set; }
        public int Area { get; set; }
        public int SubSpawn { get; set; }
        public Position? Coordinates { get; set; }

        public IndividualSpawn()
        {
            Chance = 0;
            Area = 0;
            SubSpawn = 0;
            Coordinates = null;
        }
    }
}
