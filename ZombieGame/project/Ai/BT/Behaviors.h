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

	Elite::BehaviorState ExploreHouse(Elite::Blackboard* pBlackboard)
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
			house.HouseGrid[house.CurrentCellIndex].IsVisited = true;
			++house.CurrentCellIndex;
			if (house.CurrentCellIndex >= static_cast<int>(house.HouseGrid.size()))
				return Elite::BehaviorState::Failure;

		}

		Rect currentCell = house.HouseGrid[house.CurrentCellIndex];

		pBlackboard->ChangeData("TargetHouse", house);


		pPlayer->SetTarget({ currentCell.Left + currentCell.Width * 0.5f, currentCell.Bottom + currentCell.Height * 0.5f });

		pPlayer->SetToSeek();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState PickUpItem(Elite::Blackboard* pBlackboard)
	{
		std::cout << "Change to seek\n";
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		ItemInfoExtended nextTarget;
		if (!pBlackboard->GetData("ItemTarget", nextTarget))
			return Elite::BehaviorState::Failure;

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		if (Elite::Square(playerInfo.GrabRange) > nextTarget.Location.DistanceSquared(playerInfo.Position))
		{
			pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(nextTarget.Location));

			pPlayer->SetToSeek();

			return Elite::BehaviorState::Success;
		}
		else
		{
			switch (nextTarget.Type)
			{
			case eItemType::SHOTGUN:
			{
				pInterface->Inventory_AddItem(0, nextTarget.AsItemInfo());
			}
			case eItemType::PISTOL:
			{
				pInterface->Inventory_AddItem(1, nextTarget.AsItemInfo());
			}
			case eItemType::FOOD:
			{
				if (!pInterface->Inventory_GetItem(2, ItemInfo{}))
					pInterface->Inventory_AddItem(2, nextTarget.AsItemInfo());
				else
					pInterface->Inventory_AddItem(3, nextTarget.AsItemInfo());
			}
			case eItemType::MEDKIT:
			{
				pInterface->Inventory_AddItem(4, nextTarget.AsItemInfo());
			}
			case eItemType::GARBAGE:
			{
				//pInterface->Item_Destroy(nextTarget.AsItemInfo());
			}
			default:
				return Elite::BehaviorState::Failure;
				break;
			}

			pBlackboard->ChangeData("ItemTarget", ItemInfoExtended{});
			return  Elite::BehaviorState::Success;


		}
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------


namespace BT_Conditions
{
	bool HasSeenHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "see house\n";

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

	bool HasSeenItem(Elite::Blackboard* pBlackboard)
	{

		//ItemInfoExtended target{};
		//if (pBlackboard->GetData("Player", target) && target.IsInit)
		//	return true;

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		std::vector<ItemInfoExtended>* pItemsInFov;

		if (!pBlackboard->GetData("ItemsInFov", pItemsInFov) || !pItemsInFov)
			return false;

		if (pItemsInFov->empty())
			return false;

		ItemInfoExtended closest{};

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		float closestDistSqrt{ FLT_MAX };

		for (auto& item : *pItemsInFov)
		{
			float distSqrt = item.Location.DistanceSquared(playerInfo.Position);
			if (distSqrt < closestDistSqrt)
			{
				closestDistSqrt = distSqrt;
				closest = item;
			}
		}

		if (!closest.IsInit)
			return false;

		pBlackboard->ChangeData("ItemTarget", closest);

		return true;
	}

	bool IsItemValuable(Elite::Blackboard* pBlackboard)
	{

		ItemInfoExtended target{};
		if (!pBlackboard->GetData("ItemTarget", target) || !target.IsInit)
			return false;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return false;

		std::vector<eItemType> bestInvent{};
		if (!pBlackboard->GetData("BestInventory", bestInvent) || bestInvent.empty())
			return false;



		switch (target.Type)
		{
		case eItemType::SHOTGUN:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(0, currentInventoryItem)) return true;
			return pInterface->Weapon_GetAmmo(currentInventoryItem) <= 0;
		}
		case eItemType::PISTOL:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(1, currentInventoryItem)) return true;
			return pInterface->Weapon_GetAmmo(currentInventoryItem) <= 0;
		}
		case eItemType::FOOD:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(2, currentInventoryItem)) return true;
			return !pInterface->Inventory_GetItem(3, currentInventoryItem);
		}
		case eItemType::MEDKIT:
		{
			ItemInfo currentInventoryItem{};
			return !pInterface->Inventory_GetItem(4, currentInventoryItem);
		}
		case eItemType::GARBAGE:
		{
			return false;
		}
		default:
			return false;
		}
	}
}

#endif