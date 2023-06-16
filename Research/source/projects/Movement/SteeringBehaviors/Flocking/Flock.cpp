#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include <framework/EliteInterfaces/EIApp.h>

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 10.f }
	, m_NrOfNeighbors{ 0 }
{
	m_IsPartitioning = true;
	m_pCellSpace = new CellSpace{ worldSize, worldSize, m_NrOfCellRows, m_NrOfCellCols, m_FlockSize };

	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);

	m_pEvadeBehavior = new Evade();

	m_pEvadeBehavior->SetFleeRadius(20);

	m_pBlendedSteering = new BlendedSteering({
		{ m_pSeekBehavior, 0.2f },
		{ m_pWanderBehavior, 0.4f },
		{ m_pCohesionBehavior, 0.2f },
		{ m_pSeparationBehavior, 0.4f },
		{ m_pVelMatchBehavior, 0.4f }
		});

	//m_pEvadeBehavior,
	m_pPrioritySteering = new PrioritySteering({
		m_pEvadeBehavior,
		m_pBlendedSteering
		});


	m_Agents.resize(m_FlockSize);
	m_AgentOldPositions.resize(m_FlockSize);

	// TODO: initialize the flock and the memory pool


	for (int i = 0; i < m_FlockSize; i++)
	{
		m_Agents[i] = new SteeringAgent();
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMaxLinearSpeed(50);

		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		float x{ Elite::randomFloat(m_WorldSize) },
			y{ Elite::randomFloat(m_WorldSize) };
		m_Agents[i]->SetPosition({ x, y });
		m_AgentOldPositions[i] = { x, y };
		m_pCellSpace->AddAgent(m_Agents[i]);
	}
	m_Neighbors.resize(m_FlockSize - 1);

	SetupEnemy();

}

Flock::~Flock()
{
	// TODO: clean up any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);
	SAFE_DELETE(m_pEnemyWanderBehavior);

	SAFE_DELETE(m_pEnemy);

	SAFE_DELETE(m_pCellSpace);



	for (auto pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
}

void Flock::Update(float deltaT)
{
	UpdateEvadeTarget();
	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world

#pragma region Enemy
	if (m_TrimWorld)
	{
		m_pEnemy->TrimToWorld(m_WorldSize);
	}
	m_pEnemy->Update(deltaT);

#pragma endregion


	for (size_t i = 0; i < m_Agents.size(); i++)
	{

		if (m_TrimWorld)
		{
			m_Agents[i]->TrimToWorld(m_WorldSize);
		}


		RegisterNeighbors(m_Agents[i]);

		m_Agents[i]->Update(deltaT);

		m_pCellSpace->UpdateAgentCell(m_Agents[i], m_AgentOldPositions[i]);

		m_AgentOldPositions[i] = m_Agents[i]->GetPosition();

	}

}

void Flock::Render(float deltaT)
{

	m_pEnemy->Render(deltaT);
	DEBUGRENDERER2D->DrawSolidCircle(
		m_pEnemy->GetPosition(),
		m_pEnemy->GetRadius(),
		{ 0,0 }, Elite::Color{ 1,0,0 });
	
	
	if (m_CanDebugRender)
	{

		for (auto& agent : m_Agents)
		{
			agent->Render(deltaT);

		}

		int debugIndex{ m_FlockSize - 1 };
	
	
		if (m_IsPartitioning)
		{
			m_pCellSpace->RenderCells(m_NeighborhoodRadius, m_Agents[debugIndex]);
	
		}
	
		for (int i{ 0 }; i < m_NrOfNeighbors; i++)
		{
			SteeringAgent* neigbor{ m_Neighbors[i] };
			DEBUGRENDERER2D->DrawSolidCircle(
				neigbor->GetPosition(),
				neigbor->GetRadius(),
				{ 0,0 }, Elite::Color{ 0,1,1 });
		}
	
	
		DEBUGRENDERER2D->DrawCircle(m_Agents[debugIndex]->GetPosition(), m_NeighborhoodRadius,
			Elite::Color{ 1,1,1 }, 0.9f);
	}

	
}



void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here

	ImGui::Checkbox("Turn on partitioniom", &m_IsPartitioning);


	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);


	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelocityMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");


	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	if (m_IsPartitioning)
	{
		m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);
		m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
		auto neigbors{ m_pCellSpace->GetNeighbors() };

		for (int i = 0; i < m_NrOfNeighbors; ++i)
		{
			m_Neighbors[i] = neigbors[i];
		}
	}
	else
	{
		m_NrOfNeighbors = 0;
		for (auto& agent : m_Agents)
		{
			if (agent != pAgent)
			{
				Vector2 distance{ pAgent->GetPosition() - agent->GetPosition() };
				if (distance.MagnitudeSquared() <= m_NeighborhoodRadius * m_NeighborhoodRadius)
				{

					m_Neighbors[m_NrOfNeighbors] = agent;
					++m_NrOfNeighbors;
				}
			}
		}
	}


}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	//get average of vectors:
	//https://math.stackexchange.com/questions/80923/average-of-multiple-vectors


	if (m_NrOfNeighbors == 0)
		return Elite::ZeroVector2;

	Vector2 averages;

	for (int i{ 0 }; i < m_NrOfNeighbors; i++)
	{
		averages += m_Neighbors[i]->GetPosition();
	}

	averages *= (1.0f / m_NrOfNeighbors);

	return averages;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	//get average of vectors:
	//https://math.stackexchange.com/questions/80923/average-of-multiple-vectors

	if (m_NrOfNeighbors == 0)
		return Elite::ZeroVector2;

	Vector2 averages;
	for (int i{ 0 }; i < m_NrOfNeighbors; i++)
	{
		averages += m_Neighbors[i]->GetLinearVelocity();
	}

	averages *= (1.0f / m_NrOfNeighbors);

	return averages;
}

void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::SetupEnemy()
{
	m_pEnemyWanderBehavior = new Wander{};
	m_pEnemyWanderBehavior->SetWanderRadius(50);

	m_pEnemy = new SteeringAgent();
	m_pEnemy->SetBodyColor({ 1,0,0 });
	m_pEnemy->SetPosition({ 0,m_WorldSize / 2.0f });
	m_pEnemy->SetAutoOrient(true);
	m_pEnemy->SetMaxLinearSpeed(40);

	m_pEnemy->SetSteeringBehavior(m_pEnemyWanderBehavior);

	UpdateEvadeTarget();
}

void Flock::UpdateEvadeTarget()
{
	auto target = TargetData{};
	target.Position = m_pEnemy->GetPosition();
	target.Orientation = m_pEnemy->GetRotation();
	target.LinearVelocity = m_pEnemy->GetLinearVelocity();
	target.AngularVelocity = m_pEnemy->GetAngularVelocity();
	m_pEvadeBehavior->SetTarget(target);
}

Flock& Flock::operator=(const Flock& other)
{

	m_WorldSize = other.m_WorldSize;
	m_FlockSize = other.m_FlockSize;
	m_TrimWorld = other.m_TrimWorld;
	m_pAgentToEvade = other.m_pAgentToEvade;
	m_NeighborhoodRadius = other.m_NeighborhoodRadius;
	m_NrOfNeighbors = other.m_NrOfNeighbors;

	return *this;
}
