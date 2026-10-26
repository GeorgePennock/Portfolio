using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;
using UnityEngine.AI;
using UnityEditor;

namespace Main
{
    [RequireComponent(typeof(EntityPositionUpdate))]
	public class SmartEntity : LivingEntity
	{
        private IEntityBrain brain;
		private NavMeshAgent agent;

        // .1f added as a floating buffer (this feels silly but ide rather have it just in case yknow)
        public bool AtGoal { get { return agent.remainingDistance < (agent.stoppingDistance + .1f); } }

        public float NavigationProgress { get { return startDistance == 0 ? 1 : (1 - (agent.remainingDistance / startDistance)); } }

        public float Speed 
        { 
            get 
            { 
                return agent.speed;
            } 
            set
            {
                agent.speed = value;
            }
        }

        private float startDistance = 0;

        private void Awake()
        {
            agent = GetComponent<NavMeshAgent>();
            brain = GetComponent<IEntityBrain>();

            startDistance = 0;

            if (agent == null)
            {
                Debug.LogError("No agent on smart entity.");
                return;
            }

            if (brain == null)
            {
                Debug.LogError("No brain on smart entity.");
                return;
            }

            brain.Activate();
        }

        public void Navigate(Vector3 target)
        {
            if (agent.isOnNavMesh == false) return;

            agent.destination = target;
            startDistance = agent.remainingDistance;
        }
    }
}