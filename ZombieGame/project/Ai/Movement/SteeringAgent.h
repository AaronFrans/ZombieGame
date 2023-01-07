#pragma once
#include "SteeringBehavior.h"

class SteeringAgent final
{
public:

	SteeringAgent();

	SteeringAgent(const SteeringAgent& rhs) = delete;
	SteeringAgent(SteeringAgent&& rhs) = delete;
	SteeringAgent& operator=(const SteeringAgent& rhs) = delete;
	SteeringAgent& operator=(SteeringAgent&& rhs) = delete;
	~SteeringAgent() = default;


	void UpdateAgentInfo(const AgentInfo& newInfo);
	void SetToSeek();
	void SetToWander();

	void SetTarget(Elite::Vector2 target) { m_pCurrentSteeringBehavior->SetTarget(target); };
	SteeringPlugin_Output DoSteering(float deltaT) { return m_pCurrentSteeringBehavior->CalculateSteering(deltaT, m_AgentInfo); }

	AgentInfo GetInfo() { return m_AgentInfo; };

private:

	std::shared_ptr<ISteeringBehavior> m_pCurrentSteeringBehavior;
	AgentInfo m_AgentInfo;


	//Steering
	std::shared_ptr<ISteeringBehavior> m_pSeek;
	std::shared_ptr<ISteeringBehavior> m_pWander;
};

