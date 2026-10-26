using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;
using UnityEditor;

namespace Main
{

	public class VineCurveEditorWindow : EditorWindow
	{
		[MenuItem("Tools/Curve Vine")]
		private static void CurveVineOpen()
		{
			if (Selection.activeGameObject == null) return;
			GameObject selected = Selection.activeGameObject;

			if (!selected.TryGetComponent<LineRenderer>(out LineRenderer lr))
            {
                Debug.LogWarning("Must have a line renderer selected");
                return;
			}

			if (lr.positionCount != 2)
			{
				Debug.LogWarning("Line Renderer must have exactly two points");
				return;
			}

            VineCurveEditorWindow wnd = GetWindow<VineCurveEditorWindow>();
            wnd.titleContent = new GUIContent("Vine Curve Editor");

			wnd.Configure(lr);
		}

		private void Configure(LineRenderer lineRenderer)
		{
			this.renderer = lineRenderer;

			pointA = renderer.GetPosition(0);
			pointB = renderer.GetPosition(1);

			if (pointA.y > pointB.y)
			{
				Vector3 cache = pointB;
				pointB = pointA;
				pointA = cache;
			}

			RedoLineRenderer();
		}

		private LineRenderer renderer = null;

		private int points = 25;

		private float heightOffset = -10f;

		private float curvePower = 2f;

		private Vector3 pointA;
		private Vector3 pointB;

        private void OnGUI()
        {
			EditorGUI.BeginChangeCheck();
			points = EditorGUILayout.IntSlider("Points", points, 3, 100);

			heightOffset = EditorGUILayout.FloatField("Curve Height", heightOffset);


			curvePower = EditorGUILayout.Slider("Curve Factor",curvePower, 2, 10);

            if (EditorGUI.EndChangeCheck())
			{
				RedoLineRenderer();
			}
		}

		private void RedoLineRenderer()
		{
			renderer.positionCount = points;

			for (int i = 0; i < points; i++)
			{
				float factor = i / (float)(points - 1);

				float distanceToMiddle = 1 - (Mathf.Abs(factor - .5f) * 2);

				float h = (1 - Mathf.Pow(1 - distanceToMiddle, curvePower)) * heightOffset;
				
				renderer.SetPosition(i, Vector3.Lerp(pointA, pointB, factor) + new Vector3(0,h,0));
			}
		}

    }
}