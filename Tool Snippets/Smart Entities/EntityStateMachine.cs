using Main;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Main
{
	public abstract class EntityStateMachine<T, S> : IEntityStateMachine where T : System.Enum where S : IEntityStateSettings
	{
		protected const float DELTA_TIME = 1 / (float)IEntityBrain.TICKS_PER_SECOND;

		protected delegate void EntityState(bool first);

		private T state;

		private EntityState current;

		private Dictionary<T, EntityState> states;

		public T State { get { return state; } }

		public float Timer { get { return timer; } }

		public string Status { get; protected set; }

		protected SmartEntity entity { get; private set; }

		protected Transform transform { get; private set; }

		protected float distance { get { return Vector3.Distance(Player.Ref.transform.position, transform.position); } }

		protected S settings { get; private set; }

		/// <summary>
		/// A useable timer variable that is reset to 0 when state changed.
		/// </summary>
		protected float timer;

		public EntityStateMachine(SmartEntity entity, S settings)
		{
			if (entity == null)
			{
				Debug.LogError("Null entity passed into state machine.");
				return;
			}

			if (settings == null)
			{
				Debug.LogError("Null settings passed into state machine.");
				return;
			}

			this.entity = entity;
			this.transform = entity.transform;
			this.settings = settings;

			states = PopulateStates();
		}

		protected abstract Dictionary<T, EntityState> PopulateStates();

		protected abstract T GetDefaultState { get; }

		public void Initialize()
		{
			state = GetDefaultState;
			if (states.ContainsKey(state))
			{
				current = states[state];
				timer = 0;
                drawCaches.Clear();
                current?.Invoke(true);
			}
		}

		public void ChangeState(T state)
		{
			if (this.state.Equals(state)) return;

			if (states.ContainsKey(state))
			{
				this.state = state;
				current = states[state];
				timer = 0;
                drawCaches.Clear();
                current?.Invoke(true);
			}
		}

		public void Tick()
		{
			drawCaches.Clear();
			PreTick();
			current?.Invoke(false);
		}

		/// <summary>
		/// Ran before any time the game is ticked
		/// </summary>
		protected virtual void PreTick()
		{

		}

		private List<DrawCache> drawCaches = new List<DrawCache>();

		// I can remove drawCache functionaility in the future as an optimisation

		public void DrawRequestCube(Vector3 size, Color color, Vector3 offset = default)
		{
			drawCaches.Add(new DrawCacheCube()
			{
				Offset = offset,
				Size = size,
				Color = color
			});
		}

		public void DrawRequestSphere(float radius, Color color, Vector3 offset = default)
		{
            drawCaches.Add(new DrawCacheSphere()
            {
                Offset = offset,
                Size = radius,
                Color = color
            });
        }

		public void DrawRequests()
		{
			foreach (DrawCache cache in drawCaches) cache.Draw(entity.transform);
		}

		private interface DrawCache
		{
			public void Draw(Transform myTransform);
		}

        private struct DrawCacheSphere : DrawCache
		{
			public Vector3 Offset;
			public float Size;
			public Color Color;
			public void Draw(Transform myTransform)
			{
				Gizmos.color = Color;
				Gizmos.DrawWireSphere(myTransform.position + Offset, Size);
			}
        }
        private struct DrawCacheCube : DrawCache
        {
			public Vector3 Offset;
            public Vector3 Size;
            public Color Color;
            public void Draw(Transform myTransform)
            {
                Gizmos.color = Color;
                Gizmos.DrawWireCube(myTransform.position + Offset, Size);
            }
        }
    }

	public interface IEntityStateMachine { }
	public interface IEntityStateSettings
	{ 
	
	}

}