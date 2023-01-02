#include "stdafx.h"
#include "SteeringAgent.h"




SteeringAgent::SteeringAgent()
{
	m_pSeek = std::make_shared<Seek>();
}

void SteeringAgent::UpdateAgentInfo(const AgentInfo& newInfo)
{
	m_AgentInfo = newInfo;
}

void SteeringAgent::SetToSeek()
{
	m_pCurrentSteeringBehavior = m_pSeek;
}
