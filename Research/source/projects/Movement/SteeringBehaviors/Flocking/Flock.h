#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include <projects/Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h>

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock final
{
public:
	Flock(
		int flockSize = 50,
		float worldSize = 100.f,
		SteeringAgent* pAgentToEvade = nullptr,
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return  m_NrOfNeighbors; }
	float  GetNeighborhoodRadius() const { m_NeighborhoodRadius; }
	const std::vector<SteeringAgent*>& GetNeighbors() const { return  m_Neighbors; }

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetTarget_Seek(TargetData target);
	void SetWorldTrimSize(float size) { m_WorldSize = size; }

private:
	//Datamembers
	int m_FlockSize = 0;
	std::vector<SteeringAgent*> m_Agents;
	std::vector<SteeringAgent*> m_Neighbors;
	std::vector<Elite::Vector2> m_AgentOldPositions;


	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	float m_NeighborhoodRadius = 5.f;
	int m_NrOfNeighbors = 0;


	int m_NrOfCellRows = 25;
	int m_NrOfCellCols = 25;

	bool m_CanDebugRender;
	bool m_IsPartitioning;

	CellSpace* m_pCellSpace;

	// To Avoid
	SteeringAgent* m_pEnemy;
	SteeringAgent* m_pAgentToEvade = nullptr;

	//Steering Behaviors
	Seek* m_pSeekBehavior = nullptr;
	Separation* m_pSeparationBehavior = nullptr;
	Cohesion* m_pCohesionBehavior = nullptr;
	VelocityMatch* m_pVelMatchBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Wander* m_pEnemyWanderBehavior = nullptr;
	Evade* m_pEvadeBehavior = nullptr;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	float* GetWeight(ISteeringBehavior* pBehaviour);

	void SetupEnemy();

	void UpdateEvadeTarget();

	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};