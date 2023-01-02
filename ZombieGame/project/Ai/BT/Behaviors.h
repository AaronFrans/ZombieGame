/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

#include "Ai/BT/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"


//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------


namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{


		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{

		return Elite::BehaviorState::Success;
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------


namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		return false;
	}
}









#endif