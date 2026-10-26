using Codice.Client.BaseCommands;
using Codice.Client.BaseCommands.CheckIn;
using Codice.Client.Commands;
using Main;
using System;
using System.Collections.Generic;
using Unity.VisualScripting;
using Unity.VisualScripting.YamlDotNet.Core;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

public class MapWindowEditor : EditorWindow
{

    [MenuItem("Window/Map Editor")]
    public static void ShowWindow()
    {
        MapWindowEditor wnd = GetWindow<MapWindowEditor>();
        wnd.titleContent = new GUIContent("Map Editor");
        wnd.scrollPositionCells = new Vector2(wnd.position.width / 2, wnd.position.height / 2);

        MapDesignManager.ShowChunkOutlines = false;


        wnd.mapImage = new Texture2D(1, 1);
        wnd.mapImage.LoadImage(System.IO.File.ReadAllBytes(Application.dataPath + "/Textures/Concepts/map.png"));
        wnd.mapImage.Apply();
    }

    private MapData mapData;
    private Vector2 scrollPositionData;

    private Vector2 scrollPositionCells;

    private HashSet<Vector2Int> selectedCells = new HashSet<Vector2Int>();

    private HashSet<Vector2Int> loadedCells = new HashSet<Vector2Int>();

    private HashSet<Vector2Int> feasableCells = new HashSet<Vector2Int>();

    private Texture2D mapImage;

    private Vector2 dragStart = Vector2.zero;
    private Vector2 dragEnd = Vector2.zero;
    private bool isDragging = false;

    private bool isDrawing = false;

    private void CalculateFeasableCells()
    {
        feasableCells.Clear();
        HashSet<Vector2Int> cells = new HashSet<Vector2Int>();
        foreach (Vector2Int v in mapData.Keys) cells.Add(v);

        foreach (Vector2Int v in cells)
        {
            if (!cells.Contains(v + Vector2Int.up) && !feasableCells.Contains(v + Vector2Int.up)) 
                feasableCells.Add(v + Vector2Int.up);
            if (!cells.Contains(v + Vector2Int.down) && !feasableCells.Contains(v + Vector2Int.down)) 
                feasableCells.Add(v + Vector2Int.down);
            if (!cells.Contains(v + Vector2Int.left) && !feasableCells.Contains(v + Vector2Int.left)) 
                feasableCells.Add(v + Vector2Int.left);
            if (!cells.Contains(v + Vector2Int.right) && !feasableCells.Contains(v + Vector2Int.right)) 
                feasableCells.Add(v + Vector2Int.right);
        }
    }

    public void OnGUI()
    {

        GUIStyle headerStyle = new GUIStyle(GUI.skin.label);

        headerStyle.alignment = TextAnchor.MiddleCenter;
        headerStyle.fontSize *= 2;
        headerStyle.richText = true;

        GUIStyle labelStyle = new GUIStyle(GUI.skin.label);
        labelStyle.alignment = TextAnchor.MiddleCenter;
        labelStyle.richText = true;

        if (mapData == null)
        {
            GUILayout.FlexibleSpace();
            EditorGUILayout.BeginVertical();
            GUILayout.Label("Select Map Data", headerStyle, GUILayout.ExpandWidth(true));
            EditorGUILayout.BeginHorizontal();
            GUILayout.FlexibleSpace();
            mapData = (MapData)EditorGUILayout.ObjectField(mapData, typeof(MapData), false);
            GUILayout.FlexibleSpace();
            EditorGUILayout.EndHorizontal();
            EditorGUILayout.EndVertical();
            GUILayout.FlexibleSpace();
            return;
        }
        if (Event.current != null)
        {
            switch (Event.current.type)
            {
                case EventType.ScrollWheel:
                    if (Event.current.delta.y > 0) ScaleDown();
                    if (Event.current.delta.y < 0) ScaleUp();
                    break;
                case EventType.MouseDrag:
                    if (Event.current.button == 2)
                    {
                        scrollPositionCells += Event.current.delta;
                        Repaint();
                    }
                    if (Event.current.button == 0 && !isDrawing)
                    {
                        dragEnd = Event.current.mousePosition;

                        Repaint();
                    }
                    break;
                case EventType.MouseDown:
                    if (Event.current.button == 1)
                    {
                        scrollPositionCells = new Vector2(position.width / 2, position.height / 2);
                        ResetScale();
                        Repaint();
                    }
                    if (Event.current.button != 0) break;
                    if (!isDrawing)
                    {
                        dragStart = Event.current.mousePosition;
                        dragEnd = Event.current.mousePosition;
                        isDragging = true;
                    }
                    else
                    {
                        Vector2Int cell = TranslateInverse(Event.current.mousePosition);
                        if (feasableCells.Contains(cell))
                        {
                            CreateCell(cell);
                            Repaint();
                        }
                    }

                    break;

                case EventType.MouseUp:
                    if (Event.current.button != 0) break;
                    if (!isDrawing)
                    {
                        Vector2Int targetCellA = TranslateInverse(dragStart);
                        Vector2Int targetCellB = TranslateInverse(dragEnd);

                        isDragging = false;

                        Repaint();

                        if (targetCellA != targetCellB)
                        {
                            // Has Dragged

                            int startX = Mathf.Min(targetCellA.x, targetCellB.x);
                            int endX = Mathf.Max(targetCellA.x, targetCellB.x);

                            int startY = Mathf.Min(targetCellA.y, targetCellB.y);
                            int endY = Mathf.Max(targetCellA.y, targetCellB.y);

                            bool forceSelected = false;
                            bool forceDeselected = false;
                            if (Event.current.shift) forceSelected = true;
                            if (Event.current.control) forceDeselected = true;

                            for (int x = startX; x <= endX; x++)
                            {
                                for (int y = startY; y <= endY; y++)
                                {
                                    Vector2Int cell = new Vector2Int(x, y);
                                    if ((selectedCells.Contains(cell) || forceDeselected) && !forceSelected)
                                    {
                                        selectedCells.Remove(cell);
                                        Repaint();
                                    }
                                    else if (mapData.Keys.Contains(cell) || forceSelected)
                                    {
                                        selectedCells.Add(cell);
                                        Repaint();
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (selectedCells.Contains(targetCellA))
                            {
                                selectedCells.Remove(targetCellA);
                                Repaint();
                            }
                            else if (mapData.Keys.Contains(targetCellA))
                            {
                                selectedCells.Add(targetCellA);
                                Repaint();
                            }
                        }
                    }
                    
                    break;
            }
        }

        EditorGUILayout.BeginHorizontal();

        #region SIDEBAR

        EditorGUILayout.BeginVertical(GUI.skin.box, new GUILayoutOption[] { GUILayout.Width(200) , GUILayout.ExpandHeight(true)});

        GUILayout.Label("<b>Map Editor</b>", headerStyle, new GUILayoutOption[] { GUILayout.ExpandWidth(true),  });
        
        Color bgColour = GUI.backgroundColor;
        if (isDrawing)
        {
            GUI.backgroundColor = new Color(.9f, .9f, .9f, 1f);
            if (GUILayout.Button("Drawing Mode", GUILayout.ExpandWidth(true)))
            {
                isDrawing = false;
            }
        }
        else
        {
            GUI.backgroundColor = new Color(.4f, .4f, .4f, 1f);
            if (GUILayout.Button("Viewing Mode", GUILayout.ExpandWidth(true)))
            {
                isDrawing = true;
                CalculateFeasableCells();
            }
        }
        GUI.backgroundColor = bgColour;

        if (!isDrawing)
        {

            EditorGUILayout.BeginHorizontal();

            if (GUILayout.Button("Load Cells", GUILayout.ExpandWidth(true)))
            {
                LoadCells();
            }
            if (GUILayout.Button("Save Cells", GUILayout.ExpandWidth(true)))
            {
                SaveCells(false);
            }

            EditorGUILayout.EndHorizontal();
        }
        else
        {
            if (GUILayout.Button("Validate MapData", GUILayout.ExpandWidth(true)))
            {
                mapData.EnsureNoNull();
                CalculateFeasableCells();
                EditorUtility.SetDirty(mapData);
                AssetDatabase.SaveAssets();
            }
        }

            // Load in all cells as button list

            scrollPositionData = GUILayout.BeginScrollView(scrollPositionData, new GUILayoutOption[] { GUILayout.ExpandWidth(true) });

        foreach (Vector2Int key in mapData.Keys)
        {
            GUILayout.Label(loadedCells.Contains(key) ? ("<b>Chunk " + key.ToString() + "</b>") : ("Chunk " + key.ToString()), labelStyle);
        }

        GUILayout.EndScrollView();

        if (!isDrawing)
        {
            //GUI.backgroundColor = Color.cyan;
            //GUILayout.Button("Reload Textures", GUILayout.ExpandWidth(true));
            //GUILayout.Space(25);
            GUI.backgroundColor = Color.red;
            if (GUILayout.Button("Mark Loaded Terrain Data"))
            {
                List<TerrainData> toSelect = new List<TerrainData>();
                for (int i = 0; i < SceneManager.sceneCount; i++)
                {
                    Scene scene = SceneManager.GetSceneAt(i);

                    foreach (GameObject root in scene.GetRootGameObjects())
                    {
                        if (root.TryGetComponent<Terrain>(out Terrain terrain))
                        {
                            toSelect.Add(terrain.terrainData);
                            break;
                        }
                    }
                }

                Selection.objects = toSelect.ToArray();
            }

            EditorGUILayout.Space();

            if (GUILayout.Button("Disconnect Entities", GUILayout.ExpandWidth(true)))
            {
                // Go through each selected chunk and locate its entities
                // Write all the entnties into a default save file
                // Involves making actual map saving persistence

                // Make a default save file, which is then used anytime a new world is created
                // The default save file is copied for each new world and THEN stored in LocalAppData
                
                // Involves making SaveableBehaviours persitance feature
                // Need to give a saveableBehaviour a catagory
                // Create another attribute and give it to saveableBehaviours, so you can mark them as

                // [SaveLocation(Global || World || Chunk)]
                // Global: Game Settings
                // World: Player
                // Chunk: Entities

                // then use reflection to have it decide on where to create each file

                // In the map editor i then want a button that brings all the entities back into the scene and removes them from savedata
            }

            EditorGUILayout.Space();

            if (GUILayout.Button("Deload Cells", GUILayout.ExpandWidth(true)))
            {
                DeloadCells();
            }
            GUI.backgroundColor = bgColour;
        }


        EditorGUILayout.EndVertical();

        #endregion SIDEBAR

        #region VIEW_PANNEL

        EditorGUILayout.BeginVertical(GUI.skin.box, new GUILayoutOption[] { GUILayout.ExpandWidth(true), GUILayout.ExpandHeight(true) });

        GUILayout.Label(isDrawing ? "<b>Draw</b>" : "<b>View</b>", headerStyle, new GUILayoutOption[] { GUILayout.ExpandWidth(true), });

        //EditorGUILayout.BeginHorizontal(GUILayout.ExpandWidth(true));
        //GUILayout.FlexibleSpace();

        //GUILayout.Button("Simple View");
        //GUILayout.Button("Texture View");
        //GUILayout.Button("Biome View");

        //GUILayout.FlexibleSpace();
        //EditorGUILayout.EndHorizontal();

        if (!isDrawing)
        {
            EditorGUILayout.BeginHorizontal(GUILayout.ExpandWidth(true));
            GUILayout.FlexibleSpace();

            if (GUILayout.Button("Select All"))
            {
                foreach (Vector2Int cell in mapData.Keys)
                {
                    if (!selectedCells.Contains(cell)) selectedCells.Add(cell);
                }
                Repaint();
            }
            if (GUILayout.Button("Deselect All"))
            {
                selectedCells.Clear();
                Repaint();
            }

            GUILayout.Space(20);
            Color bgC = GUI.backgroundColor;
            GUI.backgroundColor = MapDesignManager.ShowChunkOutlines ? Color.white : Color.black;
            if (GUILayout.Button("Toggle Chunk Markers"))
            {
                MapDesignManager.ShowChunkOutlines = !MapDesignManager.ShowChunkOutlines;
            }

            GUI.backgroundColor = bgC;

            GUILayout.FlexibleSpace();
            EditorGUILayout.EndHorizontal();
        }

        GUILayout.BeginHorizontal(GUI.skin.box, new GUILayoutOption[] { GUILayout.ExpandWidth(true), GUILayout.ExpandHeight(true) });

        #region CELLS

        if (mapImage == null)
        {
            mapImage = new Texture2D(1, 1);
            mapImage.LoadImage(System.IO.File.ReadAllBytes(Application.dataPath + "/Sprites/Concepts/map.png"));
            mapImage.Apply();
        }
        if (mapImage != null) GUI.DrawTexture(TranslateMap(), mapImage,ScaleMode.StretchToFill,true,0,new Color(1,1,1,.5f),0,0);

        GUIStyle simpleBox = new GUIStyle(GUI.skin.box);
        simpleBox.richText = true;
        foreach (Vector2Int cell in mapData.Keys)
        {
            if (!isDrawing)
            {
                bool selected = selectedCells.Contains(cell);
                
                GUI.backgroundColor = selected ? Color.green : Color.black;
                if (selected)
                {
                    GUI.Box(Translate(cell), GUIContent.none, simpleBox);
                    GUI.Box(Translate(cell), GUIContent.none, simpleBox);
                    GUI.Box(Translate(cell), GUIContent.none, simpleBox);
                }
                GUI.Box(Translate(cell), GUIContent.none, simpleBox);
                GUI.Box(Translate(cell), "<color=white>" + cell.ToString() + "</color>", simpleBox);

                continue;
            }

            GUI.backgroundColor = bgColour;
            GUI.Box(Translate(cell), GUIContent.none, simpleBox);
            GUI.Box(Translate(cell), "<color=#ffffff7f>" + cell.ToString() + "</color>", simpleBox);
        }

        if (isDrawing)
        {
            foreach (Vector2Int cell in feasableCells)
            {
                GUI.backgroundColor = Color.white;
                GUI.Box(Translate(cell),cell.ToString(), simpleBox);
            }
        }


        GUI.backgroundColor = bgColour;

        #endregion CELLS

        GUILayout.EndHorizontal();

        GUILayout.Label("Map Editor tool created by George Pennock (the shameless self promo in a project that im working on ikr who do i think i am)", labelStyle, GUILayout.ExpandWidth(true));

        EditorGUILayout.EndVertical();

        #endregion VIEW_PANNEL

        EditorGUILayout.EndHorizontal();
        
        if (isDragging)
        {
            Color original = GUI.backgroundColor;
            GUI.backgroundColor = Color.white;
            GUI.Box(new Rect(dragStart.x, dragStart.y, dragEnd.x - dragStart.x, dragEnd.y - dragStart.y), "");
            GUI.backgroundColor = original;
        }
    }

    private void CreateCell(Vector2Int position)
    {
        if (mapData.Get(position, out _)) return;

        Scene originalActiveScene = EditorSceneManager.GetActiveScene();

        Scene newScene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene,NewSceneMode.Additive);
        newScene.name = "Chunk (" + position.x + "," + position.y + ")";
        if (newScene == null) return;

        EditorSceneManager.SetActiveScene(originalActiveScene);

        if (!EditorSceneManager.SaveScene(newScene, "Assets/Scenes/Chunks/" + newScene.name + ".unity", true))
        {
            Debug.LogError("Couldn't save scene :(");
            return;
        }

        EditorSceneManager.CloseScene(newScene, true);

        AssetDatabase.TryGetGUIDAndLocalFileIdentifier<SceneAsset>(AssetDatabase.LoadAssetAtPath<SceneAsset>("Assets/Scenes/Chunks/" + "Chunk (" + position.x + "," + position.y + ")" + ".unity"), out string guid, out _);

        mapData.Apply(position, guid);

        Debug.Log("New scene created " + position);


        CalculateFeasableCells();
    }
    
    private Terrain GetTerrainFromScene(string sceneGuid)
    {
        string path = AssetDatabase.GUIDToAssetPath(sceneGuid);
        Scene opened = EditorSceneManager.OpenScene(path, OpenSceneMode.Additive);
        GameObject[] gameObjects = opened.GetRootGameObjects();
        foreach (GameObject gameObject in gameObjects)
        {
            if (gameObject.TryGetComponent<Terrain>(out Terrain terrain))
            {
                EditorSceneManager.CloseScene(opened, true);
                return terrain;
            }
        }
        EditorSceneManager.CloseScene(opened, true);
        return null;
    }

    private void LoadCells()
    {
        Scene currentScene = SceneManager.GetActiveScene();
        if (currentScene.name != "TerrainScene")
        {
            Debug.LogError("Must load scenes whilst focused on the Terrain Scene");
            return;
        }

        foreach (Vector2Int cell in selectedCells)
        {
            if (mapData.Get(cell, out string chunk))
            {
                string path = AssetDatabase.GUIDToAssetPath(chunk);
                EditorSceneManager.OpenScene(path,OpenSceneMode.Additive);

                loadedCells.Add(cell);
            }
            else
            {
                Debug.LogError("Failed to get mapData");
            }
        }

        selectedCells.Clear();
        Repaint();
    }

    private void SaveCells(bool delete)
    {
        mapData.Validate();

        Scene rootScene = SceneManager.GetActiveScene();

        EditorSceneManager.SaveOpenScenes();

        int count = SceneManager.sceneCount;

        for (int i = 0; i < count; i++)
        {
            Scene currentScene = SceneManager.GetSceneAt(i);
            if (currentScene != rootScene)
            {
                GameObject[] roots = currentScene.GetRootGameObjects();
                foreach (GameObject g in roots)
                {
                    if (g.TryGetComponent<Terrain>(out Terrain terrain))
                    {
                        Vector2Int cell = Main.Map.NearestChunk(terrain.transform.position);

                        if (!AssetDatabase.TryGetGUIDAndLocalFileIdentifier(AssetDatabase.LoadAssetAtPath<SceneAsset>(currentScene.path), out string guid, out long _))
                        {
                            return;
                        }
                        mapData.Apply(cell, guid);
                        break;
                    }
                }
            }
        }

        EditorUtility.SetDirty(mapData);
        AssetDatabase.SaveAssets();
    }

    private void DeloadCells()
    {
        SaveCells(true);

        EditorSceneManager.SaveOpenScenes();

        Scene rootScene = SceneManager.GetActiveScene();

        int count = SceneManager.sceneCount;

        List<Scene> cache = new List<Scene>();
        for (int i = 0; i < count; i++)
        {
            Scene currentScene = SceneManager.GetSceneAt(i);
            if (currentScene != rootScene)
            {
                cache.Add(currentScene);
            }
        }

        for (int i = 0; i < cache.Count; i++)
        {
            EditorSceneManager.CloseScene(cache[i], true);
        }

        loadedCells.Clear();

    }

    private void ScaleUp()
    {
        CELL_SIZE += 5;
        if (CELL_SIZE > 100) CELL_SIZE = 100;
        Repaint();
    }

    private void ScaleDown()
    {
        CELL_SIZE -= 5;
        if (CELL_SIZE < 5) CELL_SIZE = 5;
        Repaint();
    }

    private void ResetScale()
    {
        CELL_SIZE = 50;
        Repaint();
    }

    private int CELL_SIZE = 50;
    private const int CELL_PADDING = 5;

    private Rect Translate(Vector2Int cell)
    {
        return new Rect(scrollPositionCells.x + ((CELL_SIZE + CELL_PADDING) * cell.x), scrollPositionCells.y + ((CELL_SIZE + CELL_PADDING) * cell.y), CELL_SIZE, CELL_SIZE);
    }

    private const int MAP_REF = 42;
    private const int MAP_REF_OFFSET = MAP_REF / 2;
    private Rect TranslateMap()
    {
        return new Rect(scrollPositionCells.x - ((CELL_SIZE + CELL_PADDING) * (MAP_REF - MAP_REF_OFFSET)), scrollPositionCells.y + ((CELL_SIZE + CELL_PADDING) * (MAP_REF - MAP_REF_OFFSET)), (CELL_SIZE + CELL_PADDING) * (MAP_REF), -(CELL_SIZE + CELL_PADDING) * (MAP_REF));
    }

    private Vector2Int TranslateInverse(Vector2 position)
    {
        return new Vector2Int(
            Mathf.FloorToInt((position.x - scrollPositionCells.x) / (CELL_SIZE + CELL_PADDING)),
            Mathf.FloorToInt((position.y - scrollPositionCells.y) / (CELL_SIZE + CELL_PADDING)));
    }
}
