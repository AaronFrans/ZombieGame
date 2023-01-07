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
		std::cout << "Change to wander\n";
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		pPlayer->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "Change to seek\n";
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		HouseInfoExtended nextTarget;
		if (!pBlackboard->GetData("TargetHouse", nextTarget))
			return Elite::BehaviorState::Failure;

		pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(nextTarget.Center));

		pPlayer->SetToSeek();

		return Elite::BehaviorState::Success;
	}


	Elite::BehaviorState AddToVisitedHouses(Elite::Blackboard* pBlackboard)
	{
		std::cout << "visited house\n";
		HouseInfoExtended house;
		if (!pBlackboard->GetData("TargetHouse", house) || !house.IsInit)
			return Elite::BehaviorState::Failure;

		std::vector<HouseInfoExtended>* pVisited;
		if (!pBlackboard->GetData("VisitedHouses", pVisited) || !pVisited)
			return Elite::BehaviorState::Failure;

		pVisited->push_back(house);


		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState PatrolHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "Looting\n";
		HouseInfoExtended house;
		if (!pBlackboard->GetData("TargetHouse", house) || !house.IsInit)
			return Elite::BehaviorState::Failure;

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;


		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;


		AgentInfo& playerInfo{ pPlayer->GetInfo() };


		if (house.HouseGrid[house.CurrentCellIndex].IsInBounds(playerInfo.Position))
		{
			++house.CurrentCellIndex;
			if (house.CurrentCellIndex >= house.HouseGrid.size())
				return Elite::BehaviorState::Failure;

			house.HouseGrid[house.CurrentCellIndex].visited = true;
		}

		Rect currentCell = house.HouseGrid[house.CurrentCellIndex];

		pBlackboard->ChangeData("TargetHouse", house);


		pPlayer->SetTarget({ currentCell.Left + currentCell.Width * 0.5f, currentCell.Bottom + currentCell.Height * 0.5f });

		pPlayer->SetToSeek();

		return Elite::BehaviorState::Success;
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------


namespace BT_Conditions
{
	bool HasSeenHouse(Elite::Blackboard* pBlackboard)
	{
		//std::cout << "see house\n";

		HouseInfoExtended target;
		if (pBlackboard->GetData("TargetHouse", target) && target.IsInit)
			return true;

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		std::vector<HouseInfoExtended>* pHousesInFov;

		if (!pBlackboard->GetData("HousesInFov", pHousesInFov) || !pHousesInFov)
			return false;

		if (pHousesInFov->empty())
			return false;

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		float closestDistSqrt{ FLT_MAX };

		HouseInfoExtended closest{};

		for (auto& house : *pHousesInFov)
		{
			float distSqrt = house.Center.DistanceSquared(playerInfo.Position);
			if (distSqrt < closestDistSqrt)
			{
				closestDistSqrt = distSqrt;
				closest = house;
			}
		}

		if (!closest.IsInit)
			return false;

		std::vector<HouseInfoExtended>* pVisited;

		if (!pBlackboard->GetData("VisitedHouses", pVisited) || !pVisited)
			return false;

		for (auto& house : *pVisited)
			if (house.Center == closest.Center && house.Size == closest.Size)
				return false;

		pBlackboard->ChangeData("TargetHouse", closest);

		return true;
	}


	bool HasNotVisitedHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "have i been here before?\n";

		std::vector<HouseInfoExtended>* pVisited;		

		if (!pBlackboard->GetData("VisitedHouses", pVisited) || !pVisited)
			return false;

		if (pVisited->empty())
			return true;

		HouseInfoExtended target{};

		if (!pBlackboard->GetData("TargetHouse", target) || !target.IsInit)
			return false;

		for (auto& house : *pVisited)
			if (house.Center == target.Center && house.Size == target.Size)
				return false;

		return true;

	}

	bool IsInHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "am i Inside?\n";

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		HouseInfoExtended target{};

		if (!pBlackboard->GetData("TargetHouse", target) || !target.IsInit)
			return false;

		AgentInfo& playerInfo{ pPlayer->GetInfo() };


		if (!target.Bounds.IsInBounds(playerInfo.Position))
			return false;

		return true;
	}

	bool HasFinishedLootingHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "No more Loot?\n";

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		HouseInfoExtended target{};

		if (!pBlackboard->GetData("TargetHouse", target) || !target.IsInit)
			return false;

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		if (!target.HouseGrid.back().IsInBounds(playerInfo.Position))
			return false;

		pBlackboard->ChangeData("TargetHouse", HouseInfoExtended{});
		return true;
	}


}









#endif