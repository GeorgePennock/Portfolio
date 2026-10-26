using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;

namespace Main
{
	public class GameSave : MonoBehaviour
	{
        public delegate void TriggerEvent();

        public static TriggerEvent SaveGameTrigger { get; set; }
        public static TriggerEvent LoadGameTrigger { get; set; }


        private void Update()
        {
            /*
            if (Input.GetKeyDown(KeyCode.H))
            {
                // Save Game
                SaveWorld();
            }

            if (Input.GetKeyDown(KeyCode.L))
            {
                // Save Game
                LoadWorld();
            }
            */
        }

        public static void SaveWorld()
        {
            Chunks.QueueSave();

            SaveableBehaviour.QueueClassSaving(Player.Ref.Save);

            SaveableBehaviour.SaveGame(() =>
            {
                Debug.Log("Game Saved!");
            });
        }

        public static void DeloadGame()
        {
            Chunks.Masacre();
        }

        public static void LoadWorld()
        {
            SaveableBehaviour.LoadGame(SaveLocationAttribute.LocationType.World, (SaveableBehaviour.Save save) =>
            {
                Debug.Log("Found full save class");
                if (save.Type == typeof(PlayerSaveSystem))
                {
                    SaveableBehaviour.ForceDataLoad<PlayerSaveSystem>(Player.Ref.Save, save.Data);
                }
                else if (save.Type == typeof(ChunksSaveData))
                {
                    Chunks.LoadSeenEntitySources(save);
                }
            }, () => { });

            SaveableBehaviour.LoadGame(SaveLocationAttribute.LocationType.Chunk, Chunks.SaveRecieved, () =>
            {
                Debug.Log("Game Loaded!");
            });
        } 

        public static bool GetSaveData(long guid, out SaveableBehaviour.Save data)
        {
            data = new();

            return true;
        }
    }
}