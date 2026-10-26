using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;
using System.Security.Cryptography;

namespace Main
{
	public abstract class EntityBrain<T, K, S> : MonoBehaviour, IEntityBrain where T : EntityStateMachine<K,S> where K : System.Enum where S : IEntityStateSettings
    {
        [SerializeField] protected S settings;

        public T StateMachine { get; private set; }

        public bool Active => active;

        public string StateDescriptor => (StateMachine == null || StateMachine.State == null) ? "INACTIVE BRAIN" : StateMachine.State.ToString();

		public string StateTimer => (StateMachine == null || StateMachine.State == null) ? "" : StateMachine.Timer.ToString("F1");

        private bool active = false;

        private Coroutine coroutine;

		public abstract T CreateStateMachine();

        public void Activate()
		{
			if (StateMachine == null)
			{
                StateMachine = CreateStateMachine();
				StateMachine.Initialize();
            }

			if (Active) return;
            active = true;

            if (coroutine != null) StopCoroutine(coroutine);

			coroutine = StartCoroutine(BrainCoroutine());
		}

		public void Deactivate()
		{
			if (coroutine != null) StopCoroutine(coroutine);
			active = false;
		}

		private IEnumerator BrainCoroutine()
		{
			while (Active && StateMachine != null)
			{
				yield return new WaitForSeconds(1 / (float)IEntityBrain.TICKS_PER_SECOND);
				StateMachine.Tick();
			}
		}

        private void OnDrawGizmosSelected()
        {
			StateMachine?.DrawRequests();
        }

    }

	public interface IEntityBrain
    {
        public const int TICKS_PER_SECOND = 5;

        public void Activate();

		public void Deactivate();
        public bool Active { get; }
        public string StateDescriptor { get; }
		public string StateTimer { get; }
	}

}