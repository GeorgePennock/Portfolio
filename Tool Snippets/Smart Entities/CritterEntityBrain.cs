using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;

namespace Main
{
    public class CritterEntityBrain : EntityBrain<CritterEntityStateMachine, CritterEntityState, CritterEntitySettings>
    {
        public override CritterEntityStateMachine CreateStateMachine()
        {
            return new CritterEntityStateMachine(GetComponent<SmartEntity>(), settings);
        }
    }
}