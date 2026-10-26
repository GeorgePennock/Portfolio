using Main;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AddressableAssets;
using UnityEngine.ResourceManagement.AsyncOperations;
using UnityEngine.ResourceManagement.ResourceProviders;

namespace Main
{
	public class Chunks : Singleton<Chunks>
	{
        public delegate void ChunkEvent(Vector2Int chunk);

        private static Dictionary<Vector2Int, GameChunk> instances = new Dictionary<Vector2Int, GameChunk>();

        private static HashSet<long> sourcesViewed = new HashSet<long>();

        private static ChunksSaveData saveData;

        public static ChunkEvent OnChunkLoaded { get; set; }
        public static ChunkEvent OnChunkDeloaded { get; set; }


        public override void Awake()
        {
            base.Awake();

            Map.OnChunkSceneLoaded += ChunkLoad;
            Map.OnChunkSceneDeloaded += ChunkDeload;

            saveData = GetComponent<ChunksSaveData>();
        }

        // This should be loading persistant data from a save file
        // Should not be ran in map, should move its running to the saves system
        public static void RegisterChunk(Vector2Int position)
        {
            instances.Add(position, GameChunk.CreateDeloadedChunk(position));
        }

        public static void SaveChunks()
        {
            Debug.LogError("Saving chunks not implemented");
        }

        public static void LoadSeenEntitySources(SaveableBehaviour.Save save)
        {
            if (saveData == null)
            {
                saveData = Instance.gameObject.AddComponent<ChunksSaveData>();

                SaveableBehaviour.ForceDataLoad<ChunksSaveData>(saveData, save.Data);

                sourcesViewed = saveData.ViewedSources;
            }
        }

        private static void ChunkLoad(Vector2Int chunk, SceneInstance instance)
        {
            if (!instances.ContainsKey(chunk)) RegisterChunk(chunk);

            foreach (SaveableBehaviour.Save container in instances[chunk].DataPersistents)
            {
                LoadSaveData(container, chunk);
            }
            instances[chunk].DataPersistents.Clear();

            instances[chunk] = instances[chunk].CopyWithActive(instances[chunk], true);

            OnChunkLoaded?.Invoke(chunk);
        }

        private static void LoadSaveData(SaveableBehaviour.Save container, Vector2Int chunk)
        {
            if (container.AssetReference == null || container.AssetReference.RuntimeKeyIsValid() == false)
            {
                // Overrides for specific types of instantiated objects
                if (container.Type == typeof(ItemWrapper))
                {
                    bool success = ItemUtility.CreateItem(
                        GetFromSave<short>(container, "identifier"),
                        Vector3.zero, // Dont need to load as it will be done by ForceDataLoad 
                        Vector3.zero, // Dont need to load as it will be done by ForceDataLoad 
                        out ItemWrapper wrapper);

                    if (success)
                    {
                        SaveableBehaviour.ForceDataLoad<ItemWrapper>(wrapper, container.Data);
                    }
                    else Debug.Log("Failed to create item " + GetFromSave<short>(container, "identifier"));
                }
                else
                {
                    Debug.LogWarning("Unknown way of creating asset " + container.Type);
                    LoadEntityUsingAssetReference(AbsoluteReference.Instance.FallbackEntity, chunk, container.Data);
                }
            }
            else
            {
                LoadEntityUsingAssetReference(container.AssetReference, chunk, container.Data);
            }
        }

        private static void LoadEntityUsingAssetReference(AssetReferenceGameObject toLoad, Vector2Int chunk, Dictionary<string,SaveableBehaviour.Data> data = default)
        {
            AsyncOperationHandle<GameObject> operation = toLoad.LoadAssetAsync<GameObject>();
            operation.Completed += (AsyncOperationHandle<GameObject> asset) =>
            {
                if (asset.Status == AsyncOperationStatus.Failed)
                {
                    Debug.LogError("Faileds to load asset");
                    return;
                }

                SaveableBehaviour created;
                if (data == null)
                {
                    created = SaveableBehaviour.InstantiateNew(asset.Result.GetComponent<SaveableBehaviour>());
                }
                else
                {
                    created = SaveableBehaviour.InstantiateFrom(asset.Result.GetComponent<SaveableBehaviour>(),data);
                }

                if (created.GetType() == typeof(Entity))
                {
                    instances[chunk].WorldPersistents.Add((Entity)created);
                }
            };
            operation.ReleaseHandleOnCompletion();
        }

        private static T GetFromSave<T>(SaveableBehaviour.Save save, string id) => (T)save.Data[id].Value;

        private static void ChunkDeload(Vector2Int chunk)
        {
            foreach (Entity element in instances[chunk].WorldPersistents)
            {
                if (element == null) continue;
                DeloadEntity(element);
            }
            instances[chunk].WorldPersistents.Clear();

            instances[chunk] = instances[chunk].CopyWithActive(instances[chunk],false);

            OnChunkDeloaded?.Invoke(chunk);
        }

        public static void EntityHasChanged(Entity entity, Vector2Int oldChunk, Vector2Int newChunk)
        {
            // Only change chunk if it can find the new chunk, otherwise stay in the old one as a fallback
            if (instances.ContainsKey(newChunk))
            {
                if (!instances[newChunk].Active)
                {
                    DeloadEntity(entity);
                }
                else
                {
                    instances[newChunk].WorldPersistents.Add(entity);
                }

                if (instances.ContainsKey(oldChunk))
                {
                    instances[oldChunk].WorldPersistents.Remove(entity);
                }
            }
            else if (instances.ContainsKey(oldChunk))
            {
                // Fallback for if an entity leaves a chunk, take it to its closes chunks origin
                entity.SetPosition(Map.ChunkToWorld(oldChunk, 10, true));
                entity.OnChunkFallbackPosition?.Invoke();
            }
            else
            {
                // Final fallback, tp to 0 0 0
                entity.SetPosition(0, 0, 0);
                entity.OnChunkFallbackPosition?.Invoke();
            }
        }

        public static bool IsChunkLoaded(Vector2Int chunk)
        {
            if (!instances.ContainsKey(chunk)) return false;

            return instances[chunk].Active;
        }

        public static bool EntitySourceSeen(long guid)
        {
            return sourcesViewed.Contains(guid);
        }

        public static void EntitySourceView(long guid)
        {
            sourcesViewed.Add(guid);
        }


        public static void EntityHasChanged(Entity element, Vector2Int newChunk)
        {
            if (instances.ContainsKey(newChunk))
            {
                instances[newChunk].WorldPersistents.Add(element);
            }
        }

        public static void DeloadEntity(Entity entity)
        {
            if (entity.Saveable)
            {
                if (!instances.ContainsKey(entity.Chunk)) return;
                instances[entity.Chunk].DataPersistents.Add(entity.SaveData);
                Destroy(entity.gameObject);
            }
        }

        /// <summary>
        /// Warning! Intensive operation. Queues all entities whether persistant or not into the save system
        /// </summary>
        public static void QueueSave()
        {
            if (saveData != null)
            {
                saveData.ViewedSources = sourcesViewed;
                SaveableBehaviour.QueueClassSaving(saveData);
            }

            foreach (KeyValuePair<Vector2Int, GameChunk> pair in instances)
            {
                GameChunk chunk = pair.Value;

                if (chunk.Active)
                {
                    foreach (Entity entity in chunk.WorldPersistents)
                    {
                        SaveableBehaviour.QueueClassSaving(entity);
                    }
                }
                else
                {
                    foreach (SaveableBehaviour.Save save in chunk.DataPersistents)
                    {
                        SaveableBehaviour.QueueDataSavingIntoChunk(save, pair.Key);
                    }
                }
            }
        }

        /// <summary>
        /// Warning! Will wipe out all existing chunk data from the world. Also Intensive Operation
        /// ENSURE THAT YOU HAVE SAVED BEFORE RUNNING
        /// </summary>
        public static void Masacre()
        {
            instances.Clear();
            sourcesViewed.Clear();
        }

        public static void SaveRecieved(SaveableBehaviour.Save save)
        { 
            if (save.Data.ContainsKey(SaveableBehaviour.CHUNK_KEY) == false)
            {
                Debug.LogError("No chunk to read for save data");
                return;
            }
            Vector2Int chunk = (Vector2Int)save.Data[SaveableBehaviour.CHUNK_KEY].Value;
            LoadSaveData(save, chunk);
        }

        public struct GameChunk
        {
            public Vector2Int Location;

            public bool Active;

            public SceneInstance Instance;

            // Could be replaced with a HashSet<PersistentElementStored>
            // PersistentElementStored is a class not a monobehaviour

            // Should only be used when a scene is in the world
            public HashSet<Entity> WorldPersistents;

            // Should only be used when a scene is unloaded
            public HashSet<SaveableBehaviour.Save> DataPersistents;

            public static GameChunk CreateDeloadedChunk(Vector2Int position)
            {
                return new GameChunk()
                {
                    Location = position,
                    Active = false,
                    Instance = default,
                    WorldPersistents = new HashSet<Entity>(),
                    DataPersistents = new HashSet<SaveableBehaviour.Save>()
                };
            }

            public GameChunk CopyWithActive(GameChunk old, bool state)
            {
                return new GameChunk()
                {
                    Location = old.Location,
                    Active = state,
                    Instance = old.Instance,
                    WorldPersistents = old.WorldPersistents,
                    DataPersistents = old.DataPersistents
                };
            }
        }

    }
}