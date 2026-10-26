using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;
using Unity.VisualScripting;
using UnityEditor.AddressableAssets;

namespace Main
{
    public class CritterEntityStateMachine : EntityStateMachine<CritterEntityState, CritterEntitySettings>
    {
        private float halfwayPoint = 0;
        private bool hitHalfway = false;

        public CritterEntityStateMachine(SmartEntity entity, CritterEntitySettings settings) : base(entity, settings)
        {
            entity.OnTakeDamage += OnDamage;
        }

        ~CritterEntityStateMachine()
        {
            entity.OnTakeDamage -= OnDamage;
        }

        protected override CritterEntityState GetDefaultState => CritterEntityState.Idle;

        protected override Dictionary<CritterEntityState, EntityState> PopulateStates()
        {
            Dictionary<CritterEntityState, EntityState> states = new Dictionary<CritterEntityState, EntityState>()
            {
                { CritterEntityState.Idle, OnIdle },
                { CritterEntityState.Wander, OnWander },
                { CritterEntityState.Flee, OnFlee }
            };


            return states;
        }

        private void OnDamage(float newHealth, float changeAmount)
        {
            if (State != CritterEntityState.Flee) ChangeState(CritterEntityState.Flee);
        }

        private void OnIdle(bool isFirst)
        {
            // No idle functionality yet
            ChangeState(CritterEntityState.Wander);
        }

        private void OnWander(bool isFirst)
        {
            if (isFirst)
            {
                entity.Speed = settings.WanderSpeed;
                ResetWanderTimer();
                return;
            }

            DrawRequestSphere(settings.PlayerViewTrigger, Color.red);

            if (distance < settings.PlayerViewTrigger)
            {
                // View cone check
                if (true)
                {
                    ChangeState(CritterEntityState.Flee);
                    return;
                }
            }

            if (entity.NavigationProgress <= halfwayPoint && !hitHalfway)
            {
                hitHalfway = true;

                if (Random.value < settings.WanderInteruptChance)
                {
                    Redirect();
                }
                return;
            }

            if (entity.AtGoal)
            {
                timer -= DELTA_TIME;
                if (timer <= 0)
                {
                    Redirect();
                }
            }

        }

        private void Redirect()
        {
            ResetWanderTimer();
            Vector3 goal = new Vector3(Random.Range(-settings.WanderDistance, settings.WanderDistance), 0, Random.Range(-settings.WanderDistance, settings.WanderDistance));
            entity.Navigate(Utility.SurfacePoint(transform.position + goal));
        }

        private void ResetWanderTimer()
        {
            timer = Random.Range(settings.WanderDelay.x, settings.WanderDelay.y);
            halfwayPoint = timer / 2.0f;
            hitHalfway = false;
        }

        private void OnFlee(bool isFirst)
        {
            if (isFirst)
            {
                entity.Speed = settings.FleeSpeed;
                timer = settings.FleeRedirectTime;
            }

            DrawRequestSphere(settings.PlayerSafeDistance, Color.red);

            timer -= DELTA_TIME;

            if (timer <= 0)
            {
                if (distance > settings.PlayerSafeDistance)
                {
                    ChangeState(CritterEntityState.Wander);
                    return;
                }

                PickNewFleeLocation();
                timer = settings.FleeRedirectTime;
            }
        }

        private void PickNewFleeLocation()
        {
            Vector3 direction = (transform.position - Player.Ref.transform.position).normalized * settings.FleeGoalDistance;

            direction += new Vector3(Random.Range(-settings.FleeRedirectOffsets, settings.FleeRedirectOffsets), 0, Random.Range(-settings.FleeRedirectOffsets, settings.FleeRedirectOffsets));

            entity.Navigate(Utility.SurfacePoint(transform.position + direction));
        }
    }

    [System.Serializable]
    public class CritterEntitySettings : IEntityStateSettings
    {
        [Header("Wander Settings")]
        [Tooltip("The maximum distance the critter can wander.")]
        public float WanderDistance = 15.0f;

        [Tooltip("The delay between the critter attempting to wander once it reaches its target.")]
        public Vector2 WanderDelay = new Vector2(2.0f, 5.0f);

        [Tooltip("The chance of the critter wandering again halfway through its wander goals.")]
        [Range(0.0f, 1.0f)]
        public float WanderInteruptChance = .2f;

        [Tooltip("The speed the critter moves at when wandering.")]
        public float WanderSpeed = 3.5f;

        [Header("Detection Settings")]
        [Tooltip("The distance away from the player for it to be viewed.")]
        public float PlayerViewTrigger = 10.0f;

        [Tooltip("The distance away from the player for the critter to consider itself safe.")]
        public float PlayerSafeDistance = 40.0f;

        [Tooltip("The angle the player must be within to be viewed.")]
        [Range(0.0f,360.0f)]
        public float PlayerViewAngle = 120.0f;

        [Header("Flee settings")]
        [Tooltip("The speed the critter moves at when running away.")]
        public float FleeSpeed = 5.5f;

        [Tooltip("The time inbetween recalcuating flee targets.")]
        public float FleeRedirectTime = 1.0f;

        [Tooltip("The random offset whilst fleeing.")]
        public float FleeRedirectOffsets = 5.0f;

        [Tooltip("The total distance the critter wants to move when fleeing.")]
        public float FleeGoalDistance = 20.0f;
    }


    public enum CritterEntityState
    {
        Idle,
        Wander,
        Flee
    }

}