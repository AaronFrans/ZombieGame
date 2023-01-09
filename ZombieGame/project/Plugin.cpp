#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Ai\BT\Behaviors.h"

using namespace std;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Survivor";
	info.Student_FirstName = "Aaron";
	info.Student_LastName = "Frans";
	info.Student_Class = "2DAE15";


	WorldInfoExtended worldInfo;
	worldInfo.LoadWorldInfo(m_pInterface->World_GetInfo());

	m_pSteeringAgent = std::make_unique<SteeringAgent>(worldInfo);

	m_pVisitedHouses = std::make_unique<std::vector<HouseInfoExtended>>();
	m_pHousesInFov = std::make_unique<std::vector<HouseInfoExtended>>();
	m_pEnitiesInFov = std::make_unique<std::vector<EntityInfoExtended>>();

	m_pBB = new Elite::Blackboard();


	m_pBB->AddData("Player", m_pSteeringAgent.get());
	m_pBB->AddData("Interface", m_pInterface);
	m_pBB->AddData("TargetHouse", HouseInfoExtended{});
	m_pBB->AddData("VisitedHouses", m_pVisitedHouses.get());
	m_pBB->AddData("HousesInFov", m_pHousesInFov.get());
	m_pBB->AddData("EnitiesInFov", m_pEnitiesInFov.get());
	m_pBB->AddData("ItemTarget", EntityInfoExtended{});
	m_pBB->AddData("EnemyTarget", EntityInfoExtended{});
	m_pBB->AddData("PurgeTarget", EntityInfoExtended{});
	m_pBB->AddData("LookingForEnemy", false);


	m_pBT = new Elite::BehaviorTree(m_pBB,
		new Elite::BehaviorSelector({

			//in purge zone run
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::IsInPurgeZone),
					new Elite::BehaviorAction(BT_Actions::AvoidPurgeZone),
				}),
			//when hit look for enemy
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::IsHit),
					new Elite::BehaviorInvertConditional(BT_Conditions::HasSeenEnemy),
					new Elite::BehaviorAction(BT_Actions::LookForEnemy),
				}),
			//Low health -> heal
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::IsLowHealth),
					new Elite::BehaviorAction(BT_Actions::UseMedicine),
				}),
			//Low food -> eat
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::IsLowEnergy),
					new Elite::BehaviorAction(BT_Actions::UseFood),
				}),
			//When seen item Evaluate and pick it up
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::HasSeenItem),
					new Elite::BehaviorConditional(BT_Conditions::IsItemValuable),
					new Elite::BehaviorAction(BT_Actions::PickUpItem)
				}),
			//House Navigation
			new Elite::BehaviorSelector(
				{
					new Elite::BehaviorSelector(
						{
							//See if not been in house else add to visited
							new Elite::BehaviorSequence(
								{
									new Elite::BehaviorConditional(BT_Conditions::HasNotVisitedHouse),
									new Elite::BehaviorConditional(BT_Conditions::HasSeenHouse),
									new Elite::BehaviorConditional(BT_Conditions::IsInHouse),
									new Elite::BehaviorAction(BT_Actions::AddToVisitedHouses)
								}),
						// Check the whole house for items
						new Elite::BehaviorSequence(
							{
								new Elite::BehaviorConditional(BT_Conditions::HasSeenHouse),
								new Elite::BehaviorInvertConditional(BT_Conditions::HasFinishedLootingHouse),
								new Elite::BehaviorConditional(BT_Conditions::IsInHouse),
								new Elite::BehaviorAction(BT_Actions::ExploreHouse)
							}),
					}),
			}),
			// Go to house when seen
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::HasSeenHouse),
					new Elite::BehaviorConditional(BT_Conditions::HasNotVisitedHouse),
					new Elite::BehaviorAction(BT_Actions::ChangeToSeekHouse)
				}),
			// Enemy Fight/ Avoid
			new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::HasSeenEnemy),
					new Elite::BehaviorSelector(
					{
						new Elite::BehaviorSequence(
						{
							new Elite::BehaviorInvertConditional(BT_Conditions::HasViableWeapon),
							new Elite::BehaviorAction(BT_Actions::EvadeEnemy),
						}),
						new Elite::BehaviorAction(BT_Actions::EvadeAndShootEnemy),
					}),
				}),
			//Explore World
			new Elite::BehaviorAction(BT_Actions::ChangeExploreWorld)
			})
	);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = false; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;


	
	//params.Seed = 30;
	params.Seed = -1;
	params.SpawnZombieOnRightClick = true;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	{
		m_pInterface->RequestShutdown();
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	{
		if (m_InventorySlot > 0)
			--m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	{
		if (m_InventorySlot < 4)
			++m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	{
		ItemInfo info = {};
		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
		std::cout << (int)info.Type << std::endl;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{

	auto steering = SteeringPlugin_Output();

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	m_pSteeringAgent->UpdateAgentInfo(m_pInterface->Agent_GetInfo());
	*m_pHousesInFov = GetHousesInFOV();
	*m_pEnitiesInFov = GetEntitiesInFOV();

	m_pBT->Update(dt);

	//Use the navmesh to calculate the next navmesh point
	//auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	//OR, Use the mouse target
	//auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target); //Uncomment this to use mouse position as guidance

	//uses m_pInterface->Fov_GetHouseByIndex(...)



	for (auto& e : *m_pEnitiesInFov)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			//std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y << "---Radius: "<< zoneInfo.Radius << std::endl;
		}
	}

	//INVENTORY USAGE DEMO
	//********************

	//if (m_GrabItem)
	//{
	//	ItemInfo item;
	//	//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
	//	//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
	//	//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
	//	//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
	//	if (m_pInterface->Item_Grab({}, item))
	//	{
	//		//Once grabbed, you can add it to a specific inventory slot
	//		//Slot must be empty
	//		m_pInterface->Inventory_AddItem(m_InventorySlot, item);
	//	}
	//}
	//
	//if (m_UseItem)
	//{
	//	//Use an item (make sure there is an item at the given inventory slot)
	//	m_pInterface->Inventory_UseItem(m_InventorySlot);
	//}
	//
	//if (m_RemoveItem)
	//{
	//	//Remove an item from a inventory slot
	//	m_pInterface->Inventory_RemoveItem(m_InventorySlot);
	//}

	steering = m_pSteeringAgent->DoSteering(dt);


	//Simple Seek Behaviour (towards Target)
	//steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	//steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	//steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	//
	//if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	//{
	//	steering.LinearVelocity = Elite::ZeroVector2;
	//}
	//
	////steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	//steering.AutoOrient = true; //Setting AutoOrient to TRue overrides the AngularVelocity
	//
	//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)
	//SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

//@End (Demo Purposes)
	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{

	if (m_pSteeringAgent)
	{
		std::vector<Rect>& grid = m_pSteeringAgent->GetWorldInfo().WorldGrid;
		float colorInc{ 1.f / grid.size() };
		for (int i = 0; i < static_cast<int>(grid.size()); ++i)
		{
			std::vector<Elite::Vector2> housePoints{
				Elite::Vector2{ grid[i].Left, grid[i].Bottom },
				Elite::Vector2{ grid[i].Left, grid[i].Bottom + grid[i].Height },
				Elite::Vector2{ grid[i].Left + grid[i].Width, grid[i].Bottom + grid[i].Height },
				Elite::Vector2{ grid[i].Left + grid[i].Width, grid[i].Bottom },
			};

			if (grid[i].IsVisited)
			{
				m_pInterface->Draw_Polygon(&housePoints[0], 4, { 0,1,1 });
			}
			else
			{
				m_pInterface->Draw_Polygon(&housePoints[0], 4, { 0,0,1 });
			}
		}
	}


	HouseInfoExtended house;
	if (m_pBB->GetData("TargetHouse", house) || house.IsInit)
	{
		for (auto& house : house.HouseGrid)
		{
			std::vector<Elite::Vector2> housePoints{
				Elite::Vector2{ house.Left, house.Bottom },
				Elite::Vector2{ house.Left, house.Bottom + house.Height },
				Elite::Vector2{ house.Left + house.Width, house.Bottom + house.Height },
				Elite::Vector2{ house.Left + house.Width, house.Bottom },
			};

			if (house.IsVisited)
			{
				m_pInterface->Draw_Polygon(&housePoints[0], 4, { 0,1,1 });
			}
			else
			{
				m_pInterface->Draw_Polygon(&housePoints[0], 4, { 0,0,1 });
			}

		}
	}


	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfoExtended> Plugin::GetHousesInFOV() const
{
	vector<HouseInfoExtended> vHousesInFOV = {};

	HouseInfo hi = {};
	HouseInfoExtended hiEx = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{

			hiEx.LoadHouseInfo(hi, m_pInterface->Agent_GetInfo().AgentSize * 6);
			vHousesInFOV.push_back(hiEx);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfoExtended> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfoExtended> vEntitiesInFOV = {};

	EntityInfo ei = {};
	EntityInfoExtended eiEx = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{

			eiEx.LoadItemInfo(ei);
			vEntitiesInFOV.push_back(eiEx);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

