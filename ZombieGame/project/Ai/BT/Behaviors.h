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
#include "Utils\Utils.h"


//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------


namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		pPlayer->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeExploreWorld(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		const AgentInfo& playerInfo{ pPlayer->GetInfo() };
		WorldInfoExtended& worldInfo{ pPlayer->GetWorldInfo() };


		while (worldInfo.WorldGrid[worldInfo.CurrentCellIndex].IsVisited)
		{
			if (worldInfo.CurrentCellIndex == static_cast<int>(worldInfo.WorldGrid.size() - 1))
				break;
			worldInfo.CurrentCellIndex++;
		}


		if (worldInfo.WorldGrid[worldInfo.CurrentCellIndex].IsInBounds(playerInfo.Position))
		{
			worldInfo.WorldGrid[worldInfo.CurrentCellIndex].IsVisited = true;
			if (worldInfo.CurrentCellIndex >= static_cast<int>(worldInfo.WorldGrid.size()))
				return Elite::BehaviorState::Failure;
		}

		Rect currentCell = worldInfo.WorldGrid[worldInfo.CurrentCellIndex];

		pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint({ currentCell.Left + currentCell.Width * 0.5f, currentCell.Bottom + currentCell.Height * 0.5f }));

		pPlayer->SetToSeek();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekHouse(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		HouseInfoExtended nextTarget;
		if (!pBlackboard->GetData("TargetHouse", nextTarget))
			return Elite::BehaviorState::Failure;


		const AgentInfo& playerInfo{ pPlayer->GetInfo() };
		WorldInfoExtended& worldInfo{ pPlayer->GetWorldInfo() };

		worldInfo.WorldGrid[worldInfo.PositionToIndex(playerInfo.Position)].IsVisited = true;

		pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(nextTarget.Center));

		pPlayer->SetToSeek();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState AddToVisitedHouses(Elite::Blackboard* pBlackboard)
	{
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


		while (house.HouseGrid[house.CurrentCellIndex].IsVisited)
		{
			if (house.CurrentCellIndex == static_cast<int>(house.HouseGrid.size() - 1))
				break;
			house.CurrentCellIndex++;
		}

		if (house.HouseGrid[house.CurrentCellIndex].IsInBounds(playerInfo.Position))
		{
			house.HouseGrid[house.CurrentCellIndex].IsVisited = true;
			++house.CurrentCellIndex;
			if (house.CurrentCellIndex >= static_cast<int>(house.HouseGrid.size()))
				return Elite::BehaviorState::Failure;

		}

		Rect currentCell = house.HouseGrid[house.CurrentCellIndex];

		pBlackboard->ChangeData("TargetHouse", house);

		WorldInfoExtended& worldInfo{ pPlayer->GetWorldInfo() };

		worldInfo.WorldGrid[worldInfo.PositionToIndex(playerInfo.Position)].IsVisited = true;

		pPlayer->SetTarget({ currentCell.Left + currentCell.Width * 0.5f, currentCell.Bottom + currentCell.Height * 0.5f });

		pPlayer->SetToSeek();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState PickUpItem(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		EntityInfoExtended nextTarget;
		if (!pBlackboard->GetData("ItemTarget", nextTarget))
			return Elite::BehaviorState::Failure;

		ItemInfo item{};
		pInterface->Item_GetInfo(nextTarget.AsEntityInfo(), item);

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		HouseInfoExtended house;
		if (!pBlackboard->GetData("TargetHouse", house))
			return Elite::BehaviorState::Failure;

		if (house.IsInit)
		{
			int i = house.PositionToIndex(playerInfo.Position);
			if (i != -1)
				house.HouseGrid[i].IsVisited = true;

			pBlackboard->ChangeData("TargetHouse", house);
		}


		WorldInfoExtended& worldInfo{ pPlayer->GetWorldInfo() };

		worldInfo.WorldGrid[worldInfo.PositionToIndex(playerInfo.Position)].IsVisited = true;



		if (Elite::Square(playerInfo.GrabRange) < nextTarget.Location.DistanceSquared(playerInfo.Position))
		{
			pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(nextTarget.Location));

			pPlayer->SetToSeek();

			return Elite::BehaviorState::Success;
		}

		switch (item.Type)
		{
		case eItemType::SHOTGUN:
		{
			if (!pInterface->Item_Grab(nextTarget.AsEntityInfo(), item)) return Elite::BehaviorState::Failure;
			pInterface->Inventory_RemoveItem(0);
			pInterface->Inventory_AddItem(0, item);
		}
		case eItemType::PISTOL:
		{
			if (!pInterface->Item_Grab(nextTarget.AsEntityInfo(), item)) return Elite::BehaviorState::Failure;
			pInterface->Inventory_RemoveItem(1);
			pInterface->Inventory_AddItem(1, item);
		}
		case eItemType::FOOD:
		{
			if (!pInterface->Item_Grab(nextTarget.AsEntityInfo(), item)) return Elite::BehaviorState::Failure;
			if (!pInterface->Inventory_GetItem(2, ItemInfo{}))
			{
				pInterface->Inventory_RemoveItem(2);
				pInterface->Inventory_AddItem(2, item);
			}
			else
			{
				pInterface->Inventory_RemoveItem(3);
				pInterface->Inventory_AddItem(3, item);
			}
		}
		case eItemType::MEDKIT:
		{
			if (!pInterface->Item_Grab(nextTarget.AsEntityInfo(), item)) return Elite::BehaviorState::Failure;
			pInterface->Inventory_RemoveItem(4);
			pInterface->Inventory_AddItem(4, item);
		}
		case eItemType::GARBAGE:
		{
			pInterface->Item_Destroy(nextTarget.AsEntityInfo());
		}
		default:
			return Elite::BehaviorState::Failure;
			break;
		}

		pBlackboard->ChangeData("ItemTarget", EntityInfoExtended{});
		return  Elite::BehaviorState::Success;


	}

	Elite::BehaviorState EvadeEnemy(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		EntityInfoExtended enemyTarget;
		if (!pBlackboard->GetData("EnemyTarget", enemyTarget) || !enemyTarget.IsInit)
			return Elite::BehaviorState::Failure;

		EnemyInfo enemy{};
		pInterface->Enemy_GetInfo(enemyTarget.AsEntityInfo(), enemy);

		pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(enemy.Location));

		pPlayer->SetToFlee();

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		WorldInfoExtended& worldInfo{ pPlayer->GetWorldInfo() };

		worldInfo.WorldGrid[worldInfo.PositionToIndex(playerInfo.Position)].IsVisited = true;

		return  Elite::BehaviorState::Success;
	}

	Elite::BehaviorState EvadeAndShootEnemy(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		EntityInfoExtended enemyTarget;
		if (!pBlackboard->GetData("EnemyTarget", enemyTarget) || !enemyTarget.IsInit)
			return Elite::BehaviorState::Failure;

		ItemInfo currentInventoryItem{};

		int weaponSlot{ 0 };
		//if 0 is false check 1
		//if 0 is true use weapon 0
		//if 1 is false rerturn fail
		//if 1 is true use weapon 1
		if (!pInterface->Inventory_GetItem(weaponSlot, currentInventoryItem))
		{
			if (pInterface->Weapon_GetAmmo(currentInventoryItem) <= 0)
				if (!pInterface->Inventory_GetItem(++weaponSlot, currentInventoryItem))
					return Elite::BehaviorState::Failure;
		}

		if (pInterface->Weapon_GetAmmo(currentInventoryItem) <= 0)
			return Elite::BehaviorState::Failure;

		EnemyInfo enemy{};
		pInterface->Enemy_GetInfo(enemyTarget.AsEntityInfo(), enemy);

		auto playerLookDir = Elite::OrientationToVector(pPlayer->GetInfo().Orientation);
		auto playerToEnemy = (enemy.Location - pPlayer->GetInfo().Position).GetNormalized();

		float dot = Elite::Dot(playerLookDir, playerToEnemy);



		if (dot >= 1 - 0.00005)
		{
			pInterface->Inventory_UseItem(weaponSlot);
		}



		pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(enemy.Location));

		pPlayer->SetToFlee();

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		WorldInfoExtended& worldInfo{ pPlayer->GetWorldInfo() };

		worldInfo.WorldGrid[worldInfo.PositionToIndex(playerInfo.Position)].IsVisited = true;
		return  Elite::BehaviorState::Success;
	}

	Elite::BehaviorState UseMedicine(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		ItemInfo currentInventoryItem{};
		if (!pInterface->Inventory_GetItem(4, currentInventoryItem))
			return Elite::BehaviorState::Failure;

		if (pInterface->Medkit_GetHealth(currentInventoryItem) <= 0)
			return Elite::BehaviorState::Failure;

		pInterface->Inventory_UseItem(4);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState UseFood(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		ItemInfo currentInventoryItem{};
		int foodSlot{ 2 };
		//if 2 is false check 3
		//if 2 is true use food 2
		//if 3 is false rerturn fail
		//if 3 is true use food 3
		if (!pInterface->Inventory_GetItem(foodSlot, currentInventoryItem))
		{
			if (pInterface->Food_GetEnergy(currentInventoryItem) <= 0)
				if (!pInterface->Inventory_GetItem(++foodSlot, currentInventoryItem))
					return Elite::BehaviorState::Failure;
		}

		if (pInterface->Food_GetEnergy(currentInventoryItem) <= 0)
			return Elite::BehaviorState::Failure;

		pInterface->Inventory_UseItem(foodSlot);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState LookForEnemy(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		pPlayer->SetToTurn();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState AvoidPurgeZone(Elite::Blackboard* pBlackboard)
	{

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		EntityInfoExtended purgeTarget;
		if (!pBlackboard->GetData("PurgeTarget", purgeTarget) || !purgeTarget.IsInit)
			return Elite::BehaviorState::Failure;

		PurgeZoneInfo purge{};
		pInterface->PurgeZone_GetInfo(purgeTarget.AsEntityInfo(), purge);


		pPlayer->SetTarget(pInterface->NavMesh_GetClosestPathPoint(purge.Center));

		pPlayer->SetToFlee();

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

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		std::vector<EntityInfoExtended>* pEnititiesInFov;

		if (!pBlackboard->GetData("EnitiesInFov", pEnititiesInFov) || !pEnititiesInFov)
			return false;

		if (pEnititiesInFov->empty())
			return false;

		EntityInfoExtended closest{};

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		float closestDistSqrt{ FLT_MAX };

		for (auto& enitity : *pEnititiesInFov)
		{
			float distSqrt = enitity.Location.DistanceSquared(playerInfo.Position);
			if (distSqrt < closestDistSqrt && enitity.Type == eEntityType::ITEM)
			{
				closestDistSqrt = distSqrt;
				closest = enitity;
			}
		}

		if (!closest.IsInit)
			return false;

		pBlackboard->ChangeData("ItemTarget", closest);

		return true;
	}

	bool IsItemValuable(Elite::Blackboard* pBlackboard)
	{
		EntityInfoExtended target{};
		if (!pBlackboard->GetData("ItemTarget", target) || !target.IsInit)
			return false;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return false;

		ItemInfo item{};

		if (!pInterface->Item_GetInfo(target.AsEntityInfo(), item))
			return false;


		switch (item.Type)
		{
		case eItemType::SHOTGUN:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(0, currentInventoryItem)) return true;
			return pInterface->Weapon_GetAmmo(currentInventoryItem) <= pInterface->Weapon_GetAmmo(item);
		}
		case eItemType::PISTOL:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(1, currentInventoryItem)) return true;
			return pInterface->Weapon_GetAmmo(currentInventoryItem) <= pInterface->Weapon_GetAmmo(item);
		}
		case eItemType::FOOD:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(2, currentInventoryItem) || !pInterface->Inventory_GetItem(3, currentInventoryItem)) return true;
			return pInterface->Food_GetEnergy(currentInventoryItem) <= pInterface->Food_GetEnergy(item);
		}
		case eItemType::MEDKIT:
		{
			ItemInfo currentInventoryItem{};
			if (!pInterface->Inventory_GetItem(4, currentInventoryItem)) return true;
			return pInterface->Medkit_GetHealth(currentInventoryItem) <= pInterface->Medkit_GetHealth(item);
		}
		case eItemType::GARBAGE:
		{
			return true;
		}
		default:
			return false;
		}
	}

	bool HasSeenEnemy(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		std::vector<EntityInfoExtended>* pEnititiesInFov;

		if (!pBlackboard->GetData("EnitiesInFov", pEnititiesInFov) || !pEnititiesInFov)
			return false;

		if (pEnititiesInFov->empty())
			return false;

		EntityInfoExtended closest{};

		AgentInfo& playerInfo{ pPlayer->GetInfo() };

		float closestDistSqrt{ FLT_MAX };

		for (auto& enitity : *pEnititiesInFov)
		{
			float distSqrt = enitity.Location.DistanceSquared(playerInfo.Position);
			if (distSqrt < closestDistSqrt && enitity.Type == eEntityType::ENEMY)
			{
				closestDistSqrt = distSqrt;
				closest = enitity;
			}
		}

		if (!closest.IsInit)
			return false;

		pBlackboard->ChangeData("EnemyTarget", closest);


		pBlackboard->ChangeData("LookingForEnemy", false);

		return true;
	}

	bool HasViableWeapon(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return false;

		ItemInfo currentInventoryItem{};
		if (pInterface->Inventory_GetItem(0, currentInventoryItem) || pInterface->Inventory_GetItem(1, currentInventoryItem))
		{
			return pInterface->Weapon_GetAmmo(currentInventoryItem) > 0;
		}

		return false;
	}

	bool IsLowHealth(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;


		return pPlayer->GetInfo().Health < 3.f;
	}

	bool IsLowEnergy(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;


		return pPlayer->GetInfo().Energy < 3.f;
	}

	bool IsInPurgeZone(Elite::Blackboard* pBlackboard)
	{
		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		std::vector<EntityInfoExtended>* pEnititiesInFov;

		if (!pBlackboard->GetData("EnitiesInFov", pEnititiesInFov) || !pEnititiesInFov)
			return false;

		if (pEnititiesInFov->empty())
			return false;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return false;


		for (auto& entity : *pEnititiesInFov)
		{
			if (entity.Type != eEntityType::PURGEZONE)
				continue;

			PurgeZoneInfo purge{};

			pInterface->PurgeZone_GetInfo(entity.AsEntityInfo(), purge);

			if (!Utils::IsInCircle(pPlayer->GetInfo().Position, purge.Center, purge.Radius + 5))
				continue;

			pBlackboard->ChangeData("PurgeTarget", entity);

			return true;

		}



		return false;
	}

	bool IsHit(Elite::Blackboard* pBlackboard)
	{
		bool isLookingForEnemy{};
		if (pBlackboard->GetData("LookingForEnemy", isLookingForEnemy) && isLookingForEnemy)
			return true;

		SteeringAgent* pPlayer;
		if (!pBlackboard->GetData("Player", pPlayer) || !pPlayer)
			return false;

		isLookingForEnemy = pPlayer->GetInfo().WasBitten;

		pBlackboard->ChangeData("LookingForEnemy", isLookingForEnemy);

		return  isLookingForEnemy;
	}
}

#endif