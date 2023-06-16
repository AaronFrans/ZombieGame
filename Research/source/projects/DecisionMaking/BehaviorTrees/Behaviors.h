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
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------


namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		pAgent->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;


		Elite::Vector2 food;
		if (!pBlackboard->GetData("Target", food))
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(food);

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
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;


		std::vector<AgarioFood*>* pFoodVec;

		if (!pBlackboard->GetData("FoodVec", pFoodVec) || !pFoodVec)
			return false;

		if(pFoodVec->empty())
			return false;

		const float searchRad{ pAgent->GetRadius() + 20 };

		float closestDistSqrt{ Elite::Square(searchRad) };
		AgarioFood* pClosest{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pFood : *pFoodVec)
		{
			float distSqrt = pFood->GetPosition().DistanceSquared(agentPos);
			if (distSqrt < closestDistSqrt)
			{
				closestDistSqrt = distSqrt;
				pClosest = pFood;
			}
		}

		if (!pClosest)
			return false;

		pBlackboard->ChangeData("Target", pClosest->GetPosition());

		return true;
	}
}









#endif