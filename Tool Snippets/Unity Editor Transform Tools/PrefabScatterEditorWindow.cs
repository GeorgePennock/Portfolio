using Main;
using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using Unity.VisualScripting.YamlDotNet.Serialization;
using UnityEditor;
using UnityEngine;
using UnityEngine.AddressableAssets;

namespace Main
{
	public class PrefabScatterEditorWindow : EditorWindow
	{
		[MenuItem("Window/Prefab Scatter")]
		private static void OpenPrefabScatterer()
		{
			PrefabScatterEditorWindow wnd = GetWindow<PrefabScatterEditorWindow>();
			wnd.titleContent = new GUIContent("Prefab Scatter Tool");
            
        }

		private bool guiLayoutOpen = true;

        private List<GameObject> scatterables = new List<GameObject>();

        private bool randomYRotation = false;
        private bool rotateToNormals = false;

        private bool active = false;

		private bool entityMode = false;


        private void OnGUI()
        {
			guiLayoutOpen = EditorGUILayout.BeginFoldoutHeaderGroup(guiLayoutOpen, "Prefabs");

			for (int i = 0; i < scatterables.Count; i++)
			{
                scatterables[i] = (GameObject)EditorGUILayout.ObjectField(scatterables[i], typeof(GameObject), false, GUILayout.ExpandWidth(true));
			}

			EditorGUILayout.EndFoldoutHeaderGroup();

			EditorGUILayout.BeginHorizontal();

			if (GUILayout.Button("Add Scatterable"))
			{
                scatterables.Add(null);
			}

			if (GUILayout.Button("Clear Scatterables"))
			{
                scatterables.Clear();
				if (active)
				{
					active = false;
					SceneView.duringSceneGui -= OnSceneGUI;
				}
			}

			EditorGUILayout.EndHorizontal();

			EditorGUILayout.Space();

			randomYRotation = EditorGUILayout.Toggle("Random Y Rotation", randomYRotation);
            rotateToNormals = EditorGUILayout.Toggle("Hit Surface Normals", rotateToNormals);
			EditorGUILayout.Space();
            entityMode = EditorGUILayout.Toggle("Place Entity Sources", entityMode);

			if (scatterables.Count == 0) return;
			EditorGUILayout.Space();

			if (GUILayout.Button("Scatter Mode " + (active ? "[ON]" : "[OFF]")))
			{
                active = !active;
				if (active)
				{
                    SceneView.duringSceneGui += OnSceneGUI;
                }
				else
				{
                    SceneView.duringSceneGui -= OnSceneGUI;
                }
            }
        }

        private void OnDestroy()
        {
			randomYRotation = false;
			rotateToNormals = false;
			scatterables = new List<GameObject>();
			
			if (active) SceneView.duringSceneGui -= OnSceneGUI;
            
			active = false;
        }

		private void OnSceneGUI(SceneView view)
		{
			Event currentEvent = Event.current;
			if (currentEvent != null && currentEvent.type == EventType.MouseUp && currentEvent.button == 0)
			{
			    Ray direction = HandleUtility.GUIPointToWorldRay(currentEvent.mousePosition);
				
				if (scatterables != null && Physics.Raycast(direction,out RaycastHit hit, 1000))
				{
					int random = Random.Range(0, scatterables.Count);
					GameObject toScatter = scatterables[random];
					if (toScatter == null)
					{
						Debug.LogError("Null in the random scatterables.");
						return;
                    }

					GameObject spawned;

                    if (entityMode)
					{
						if (toScatter.TryGetComponent<Entity>(out Entity entity))
						{
							if (entity.TryGetComponent<SaveableBehaviourPrefab>(out SaveableBehaviourPrefab prefab))
							{
								AssetReferenceGameObject argo = prefab.Prefab;
								if (argo == null)
								{
									Debug.LogError("Null Argo.");
									return;
								}
								GameObject source = new GameObject("Entity Source (" + entity.name + ")");
								EntitySourceGeneric esg = source.AddComponent<EntitySourceGeneric>();
								esg.Editor_Reference = argo;

								source.transform.position = hit.point;

								spawned = source;
							}
							else
							{
								Debug.LogError("Cannot place entity as it doesnt have an assetreference.");
								return;
							}
						}
						else
						{
							Debug.LogError("Tried to scatter non-entity");
							return;
						}
					}
					else
					{
						spawned = Instantiate(toScatter, hit.point, Quaternion.identity, null);
                    }

					if (rotateToNormals)
					{
						Quaternion angle = Quaternion.LookRotation(hit.normal, Vector3.up);
						angle *= Quaternion.Euler(90, 0, 0);
						spawned.transform.rotation = angle;
					}

					if (randomYRotation)
					{
						spawned.transform.Rotate(new Vector3(0, Random.Range(0.0f, 360.0f), 0), Space.Self);
                    }
                    
					Undo.RegisterCreatedObjectUndo(spawned, "Scatter " + spawned.name);

                    currentEvent.Use();
                }
			}

        }
    }
}