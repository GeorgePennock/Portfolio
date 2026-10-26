using Main;
using Main.DataParsing;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using System.Xml.Serialization;

// TODO Why am i using this dawg i wanna delete it
using Unity.VisualScripting;
using UnityEditor;
using UnityEngine;
using UnityEngine.AddressableAssets;

namespace Main
{
    public class SaveableBehaviour : MonoBehaviour
	{
        public delegate List<string> SaveMethodCallback(SaveableBehaviour.Save save);

        public virtual bool Saveable { get; } = true;

        public Save SaveData
        {
            get
            {
                AssetReferenceGameObject argo = null;
                SaveableBehaviourPrefab prefab = GetComponent<SaveableBehaviourPrefab>();
                if (prefab != null) argo = prefab.Prefab;
                return new Save()
                {
                    AssetReference = argo,
                    Type = GetType(),
                    Data = DataDeload()
                };
            }
        }

        // GUID is currently unused as i actually dont think i need it, but i might implement it at some point
        public long GUID { get; private set; } = -1;

        public static T InstantiateNew<T>(T behaviour, Transform parent = null) where T : SaveableBehaviour
        {
            T newObject = Instantiate(behaviour, parent);

            return newObject;
        }

        public static T InstantiateNewTemplateless<T>() where T : SaveableBehaviour
        {
            GameObject gameObject = new GameObject("New Instantiated Object");
            T newObject = gameObject.AddComponent<T>();

            return newObject;
        }

        public static T InstantiateFrom<T>(T behaviour, Dictionary<string, Data> source) where T : SaveableBehaviour
        {
            T newObject = Instantiate(behaviour);

            newObject.DataLoad(source);

            return newObject;
        }

        public static void ForceDataLoad<T>(T behaviour, Dictionary<string, Data> data) where T : SaveableBehaviour
        {
            behaviour.DataLoad(data);
        }
        public static Dictionary<string,Data> ForceDataDeload<T>(T behaviour) where T : SaveableBehaviour
        {
            return behaviour.DataDeload();
        }

        public static T InstantiateFromTemplateless<T>(Dictionary<string, Data> source) where T : SaveableBehaviour
        {
            GameObject gameObject = new GameObject("New Instantiated Object");
            T newObject = gameObject.AddComponent<T>();

            newObject.DataLoad(source);

            return newObject;
        }

        private void DataLoad(Dictionary<string, Data> data)
        {
            List<StoredInfo> info = GetSaveVariables();

            for (int i = 0; i < info.Count; i++)
            {
                string field = info[i].Name;
                if (data.ContainsKey(field))
                {
                    info[i].SetValue(this, data[field].Value);
                }
            }
        }

        private Dictionary<string, Data> DataDeload()
        {
            Dictionary<string, Data> data = new Dictionary<string, Data>();

            List<StoredInfo> info = GetSaveVariables();

            for (int i = 0; i < info.Count; i++)
            {
                string field = info[i].Name;
                data.Add(field, new Data(info[i].GetValue(this), info[i].Attribute.Persists));
            }

            return data;
        }

        private List<StoredInfo> GetSaveVariables()
        {
            FieldInfo[] fields = GetType().GetFields();
            PropertyInfo[] props = GetType().GetProperties();
            
            List<StoredInfo> stored = new List<StoredInfo>();

            foreach (FieldInfo field in fields)
            {
                SaveVarAttribute attribute = (SaveVarAttribute)Attribute.GetCustomAttribute(field,typeof(SaveVarAttribute));
                if (attribute == null) continue;

                stored.Add(new StoredInfo(field, attribute));
            }

            foreach (PropertyInfo prop in props)
            {
                SaveVarAttribute attribute = (SaveVarAttribute)Attribute.GetCustomAttribute(prop, typeof(SaveVarAttribute));
                if (attribute == null) continue;

                stored.Add(new StoredInfo(prop, attribute));
            }

            return stored;
        }

        private struct StoredInfo
        {
            public string Name;
            public object GetValue(object self)
            {
                return IsField ? Field.GetValue(self) : Property.GetValue(self);
            }
            public void SetValue(object self, object value)
            {
                if (IsField)
                {
                    Field.SetValue(self, value);
                }
                else
                {
                    Property.SetValue(self, value);
                }
            }
            public PropertyInfo Property;
            public FieldInfo Field;

            public bool IsField;

            public SaveVarAttribute Attribute;

            public StoredInfo(PropertyInfo property, SaveVarAttribute attribute)
            {
                Property = property;
                Field = null;
                Attribute = attribute;
                IsField = false;

                if (attribute.ForcedName == null)
                {
                    Name = Property.Name;
                }
                else Name = attribute.ForcedName;
            }

            public StoredInfo(FieldInfo field, SaveVarAttribute attribute)
            {
                Property = null;
                Field = field;
                Attribute = attribute;
                IsField = true;

                if (attribute.ForcedName == null)
                {
                    Name = Field.Name;
                }
                else Name = attribute.ForcedName;
            }
        }

        [System.Serializable]
        public struct Save
        {
            public AssetReferenceGameObject AssetReference;
            public System.Type Type;
            public long GUID;
            public Dictionary<string, Data> Data;
        }

        public struct Data
        {
            public object Value { get; set; }

            public bool Persistant { get; set; }

            public Data(object value, bool persistant)
            {
                Value = value;
                Persistant = persistant;
            }

        }

        private const int SAVE_FILE_OFFSET = 3;

        public static bool LoadSaveFromFile(List<string> data, out Save save)
        {
            save = default;
            if (data == null || data.Count < SAVE_FILE_OFFSET)
            {
                Debug.LogError("Invalid save file data passed into LoadSaveFromFile().");
                return false;
            }

            if (!long.TryParse(data[0], out long guid))
            {
                Debug.LogError("Invalid GUID passed through save data.");
                return false;
            }

            Type type = Type.GetType(data[1]);
            if (type == null)
            {
                Debug.LogError("Invalid Type passed through save data.");
                return false;
            }

            AssetReferenceGameObject assetReference = null;
            // Currently you cannot save gameObject asset references as strings, solution inc but fornow it will produce error entities.

            Dictionary<string, Data> storedData = new Dictionary<string, Data>();

            for (int i = SAVE_FILE_OFFSET; i < data.Count; i++)
            {
                string[] split = data[i].Split(':', 3);
                
                if (split.Length != 3)
                {
                    Debug.LogError("Error loading data from save: " + data[i]);
                    continue;
                }
                System.Type valType = DataSerialization.ParseType(split[1]);

                object castData = serializer.ParseFromString(split[2], valType);

                storedData.Add(split[0], new Data(castData,true));
            }

            save = new Save()
            {
                GUID = guid,
                Type = type,
                AssetReference = assetReference,
                Data = storedData
            };
            return true;
        }
        public static string[] LoadFileFromSave(Save save)
        {
            string[] data = new string[save.Data.Count + SAVE_FILE_OFFSET];

            data[0] = save.GUID.ToString();
            data[1] = save.Type.FullName;
            data[2] = save.AssetReference == null ? "NO-ASSET-REFERENCE" : save.AssetReference.AssetGUID;

            int increment = 0;
            foreach (KeyValuePair<string,Data> pair in save.Data)
            {
                // : is my delimiter. Aslong as the key's name doesnt contain a : it should work (realistically keys cant contain : anyways unless you custom name them)
                // Players probably cant break this but who knows
                if (pair.Value.Persistant == false) continue;
                
                System.Type type = pair.Value.Value.GetType();

                string d = serializer.ParseToString(pair.Value.Value, type);
                string builtValue = pair.Key + ":" + type + ":" + d;

                data[SAVE_FILE_OFFSET + increment] = builtValue;

                increment++;
            }

            return data;
        }
        // TODO: Most of this should be moved out of saveable behaviour
        // Just because its saveable, doesnt mean it should be in charge of saving

        // TODO
        // This should be really changed to a modular system
        // Dictionary of catagories for saves

        // Saveables that should go to the global save file
        private static HashSet<SaveableBehaviour> queuedGlobalSaveables = new HashSet<SaveableBehaviour>();
        
        // Saveables that should go to the world save file
        private static HashSet<SaveableBehaviour> queuedWorldSaveables = new HashSet<SaveableBehaviour>();
        
        // Entities that should go to the chunk save file. Cannot store a SaveableBehaviour this way
        private static HashSet<Entity> queuedChunkSaveables = new HashSet<Entity>();

        // Entities which are not loaded but still want to be saved are stored as ghosts here
        private static HashSet<(Save save, Vector2Int chunk)> queuedGhostChunkSaveables = new HashSet<(Save save, Vector2Int chunk)>();

        // Queues the class to be saved upon the next save cycle
        public static void QueueClassSaving(SaveableBehaviour behaviour)
        {
            SaveLocationAttribute.LocationType location = GetSaveLocationOfClass(behaviour);

            switch (location)
            {
                case SaveLocationAttribute.LocationType.Global:
                    if (!queuedGlobalSaveables.Contains(behaviour)) queuedGlobalSaveables.Add(behaviour);
                    break;

                case SaveLocationAttribute.LocationType.World:
                    if (!queuedWorldSaveables.Contains(behaviour)) queuedWorldSaveables.Add(behaviour);
                    break;

                case SaveLocationAttribute.LocationType.Chunk:
                    if (behaviour is Entity entity && !queuedChunkSaveables.Contains(entity)) queuedChunkSaveables.Add(entity);
                    else Debug.LogError("Tried to save non entity " + behaviour.name + "/" + behaviour.GetType() + " into a chunk.");
                    break;
            }
        }

        // Queues the SaveData into the system. Must be a chunk based save. Cannot be removed when enqueued!
        public static void QueueDataSavingIntoChunk(Save save, Vector2Int chunk)
        {
            queuedGhostChunkSaveables.Add((save,chunk));
        }

        public static void DequeueClassSaving(SaveableBehaviour behaviour)
        {
            SaveLocationAttribute.LocationType location = GetSaveLocationOfClass(behaviour);

            switch (location)
            {
                case SaveLocationAttribute.LocationType.Global:
                    if (queuedGlobalSaveables.Contains(behaviour)) queuedGlobalSaveables.Remove(behaviour);
                    break;

                case SaveLocationAttribute.LocationType.World:
                    if (queuedWorldSaveables.Contains(behaviour)) queuedWorldSaveables.Remove(behaviour);
                    break;

                case SaveLocationAttribute.LocationType.Chunk:
                    if (behaviour is Entity entity && queuedChunkSaveables.Contains(entity)) queuedChunkSaveables.Remove(entity);
                    else Debug.LogError("Tried to de-save non entity " + behaviour.name + "/" + behaviour.GetType() + " from a chunk.");
                    break;
            }
            
        }

        private static SaveLocationAttribute.LocationType GetSaveLocationOfClass(SaveableBehaviour behaviour)
        {
            if (behaviour.GetType().HasAttribute<SaveLocationAttribute>() == true)
            {
                return behaviour.GetType().GetAttribute<SaveLocationAttribute>().Location;
            }
            return SaveLocationAttribute.LocationType.Global;
        }

        public delegate void SaveGameEvent();

        public delegate void SaveDataLoadedEvent(Save save);

        public const string CHUNK_KEY = "CHUNK";

        private const string SAVE_DIR = "/Save/";

        private const string GLOBAL_SAVE_FILE = "global.tthh";

        // TODO: Replace with a world save choice system, for now there is only ever one world.
        private const string WORLD_SAVE_FILE = "world.tthh";

        // Replace it with whatever the worlds name is, plus the prefix of _chunks.
        private const string CHUNK_SAVE_FILE = "world_chunks.tthh";

        private static DataSerialization serializer = new DataSerialization();

        public static void InitSerializer()
        {
            serializer = new DataSerialization();
        }

        /// <summary>
        /// Warning! Intesive Operation. Saves any Queued classes into files and empties out the queues. Any class not queued will NOT be saved!
        /// </summary>
        public static void SaveGame(SaveGameEvent onSaveComplete = default)
        {
            // TODO: Make Async
            string globalPath = Application.persistentDataPath + SAVE_DIR + GLOBAL_SAVE_FILE;
            string worldPath = Application.persistentDataPath + SAVE_DIR + WORLD_SAVE_FILE;
            string chunkPath = Application.persistentDataPath + SAVE_DIR + CHUNK_SAVE_FILE;

            Debug.Log("Saving Global Data");

            if (!Directory.Exists(Application.persistentDataPath + SAVE_DIR))
            {
                Directory.CreateDirectory(Application.persistentDataPath + SAVE_DIR);
            }

            using (StreamWriter globalStream = new StreamWriter(globalPath, false))
            {
                foreach (SaveableBehaviour behaviour in queuedGlobalSaveables)
                {
                    if (!behaviour.Saveable)
                    {
                        Debug.LogWarning("Skipping saving of " + behaviour + " because it was not marked as Saveable.");
                        continue;
                    }
                    WriteSaveableBehaviourWithDelimiter(globalStream, behaviour);
                }
            }

            queuedGlobalSaveables.Clear();

            Debug.Log("Saving World Data");

            using (StreamWriter worldStream = new StreamWriter(worldPath, false))
            {
                foreach (SaveableBehaviour behaviour in queuedWorldSaveables)
                {
                    if (!behaviour.Saveable)
                    {
                        Debug.LogWarning("Skipping saving of " + behaviour + " because it was not marked as Saveable.");
                        continue;
                    }
                    WriteSaveableBehaviourWithDelimiter(worldStream, behaviour);
                }

            }

            queuedWorldSaveables.Clear();

            Debug.Log("Saving Chunk Data");

            using (StreamWriter chunkStream = new StreamWriter(chunkPath, false))
            {
                // Currently no logical difference for saving entities, but is split if needed
                foreach (Entity entity in queuedChunkSaveables)
                {
                    if (!entity.Saveable)
                    {
                        Debug.LogWarning("Skipping saving of " + entity + " because it was not marked as Saveable.");
                        continue;
                    }
                    WriteEntityWithDelimiter(chunkStream, entity);
                }

                foreach ((Save save, Vector2Int chunk) in queuedGhostChunkSaveables)
                {
                    WriteEntityWithDelimiter(chunkStream, save,chunk);
                }
            }

            queuedChunkSaveables.Clear();
            queuedGhostChunkSaveables.Clear();

            Debug.Log("Saved Game Data.");

            onSaveComplete?.Invoke();
        }

        private static void WriteSaveableBehaviourWithDelimiter(StreamWriter writer, SaveableBehaviour behaviour)
        {
            WriteSaveableBehaviourWithDelimiter(writer, behaviour.SaveData);
        }

        private static void WriteEntityWithDelimiter(StreamWriter writer, Entity entity)
        {
            Save save = entity.SaveData;
            save.Data.Add("CHUNK", new Data(entity.Chunk, true));
            WriteSaveableBehaviourWithDelimiter(writer, save);
        }

        private static void WriteSaveableBehaviourWithDelimiter(StreamWriter writer, Save data)
        {
            string[] save = LoadFileFromSave(data);

            foreach (string s in save) writer.WriteLine(s);

            writer.WriteLine('#');
        }

        private static void WriteEntityWithDelimiter(StreamWriter writer, Save data, Vector2Int chunk)
        {
            data.Data.Add("CHUNK", new Data(chunk,true));
            WriteSaveableBehaviourWithDelimiter(writer, data);
        }

        /// <summary>
        /// Warning! Intesive Operation. Reads the told save files data and runs the connect callbacks
        /// </summary>
        public static void LoadGame(SaveLocationAttribute.LocationType location, SaveDataLoadedEvent onSaveLoaded, SaveGameEvent onLoaded)
        {
            string path = Application.persistentDataPath + SAVE_DIR;
            switch (location)
            {
                case SaveLocationAttribute.LocationType.Global: path += GLOBAL_SAVE_FILE; break;
                case SaveLocationAttribute.LocationType.World: path += WORLD_SAVE_FILE; break;
                case SaveLocationAttribute.LocationType.Chunk: path += CHUNK_SAVE_FILE; break;
            }

            Debug.Log("Loading game data from file " + location + " : " + path);

            using (StreamReader fileReader = new StreamReader(path))
            {
                List<string> activeLines = new List<string>();
                while (fileReader.EndOfStream == false)
                {
                    string line = fileReader.ReadLine();
                    if (line == "#")
                    {
                        // Hit Delimiter, found full file
                        if (LoadSaveFromFile(activeLines, out Save save))
                        {
                            onSaveLoaded?.Invoke(save);
                        }
                        activeLines.Clear();
                    }
                    else activeLines.Add(line);
                }
            }

            Debug.Log("All save data loaded");
            onLoaded?.Invoke();

        }
    }

    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class SaveVarAttribute : Attribute
    {
        public bool Persists { get; set; }

        public string ForcedName { get; set; }

        /// <summary>
        /// This field's state will be saved when de/reloaded
        /// </summary>
        /// <param name="persists">Whether the save will persist through gameloading. If true closing the game will write it to a save file, otherwise it will only be saved in runtime.</param>
        public SaveVarAttribute(bool persists = true, string forcedName = default)
        {
            Persists = persists;
            ForcedName = forcedName;
        }
    }

    [AttributeUsage(AttributeTargets.Class)]
    public class SaveLocationAttribute : Attribute
    {
        public LocationType Location { get; set; }

        /// <summary>
        /// The classes save location when its being serialized into data. If a save location is not specificed the file will default to global mode.
        /// </summary>
        /// <param name="location">The save location of the data</param>
        public SaveLocationAttribute(LocationType location)
        {
            Location = location;
        }

        public enum LocationType
        { 
            /// <summary>
            /// Stores the save values completely seperate to any worlds, is the same for every instance of the game.
            /// </summary>
            Global, 

            /// <summary>
            /// Stores the save values only connected to the specific world file.
            /// </summary>
            World,

            /// <summary>
            /// Stores the save values in a specific chunk when the map is deloaded. Will not save if not ran from an entity.
            /// </summary>
            Chunk
        }
    }
}


// yo thanks bro https://stackoverflow.com/questions/2434534/serialize-an-object-to-string
