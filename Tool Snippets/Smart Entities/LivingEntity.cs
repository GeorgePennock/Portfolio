using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;

namespace Main
{
	[RequireComponent(typeof(Health))]
	public class LivingEntity : Entity
	{
		[SaveVar]
		public float Health { get { return health == null ? GetComponent<Health>().Value : health.Value; } 
			set 
			{
				if (health == null) GetComponent<Health>().Value = value;
				else health.Value = value;
			}
		}

		public float MaxHealth { get { return health.Max; } }
		public float HealthPercentage { get { return health.Percentage; } }

		public Health.DeathEvent OnDeath { get { return health.OnDeath; } set { health.OnDeath = value; } }
		public Health.HealthModifiedEvent OnTakeDamage 
		{ 
			get 
			{ 
				if (health == null) health = GetComponent<Health>();
				return health.OnDamage;
			} 
			set
            {
                if (health == null) health = GetComponent<Health>();
                health.OnDamage = value;
			}
		}

		private Health health;

        private void Awake()
        {
            health = GetComponent<Health>();
        }
    }
}