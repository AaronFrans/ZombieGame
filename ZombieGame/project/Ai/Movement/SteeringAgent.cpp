#include "stdafx.h"
#include "SteeringAgent.h"




SteeringAgent::SteeringAgent()
{
	m_pSeek = std::make_shared<Seek>();
	m_pWander = std::make_shared<Wander>();
}

void SteeringAgent::UpdateAgentInfo(const AgentInfo& newInfo)
{
	m_AgentInfo = newInfo;
}

void SteeringAgent::SetToSeek()
{
	m_pCurrentSteeringBehavior = m_pSeek;
}

void SteeringAgent::SetToWander()
{

	m_pCurrentSteeringBehavior = m_pWander;
}
