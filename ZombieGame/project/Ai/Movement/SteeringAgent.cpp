#include "stdafx.h"
#include "SteeringAgent.h"





SteeringAgent::SteeringAgent(WorldInfoExtended worldInfo)
	:m_WorldInfo{worldInfo}
{
	m_pSeek = std::make_shared<Seek>();
	m_pWander = std::make_shared<ExploreWorld>();
	m_pFlee = std::make_shared<Flee>();
	m_pTurn = std::make_shared<Turn>();
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

void SteeringAgent::SetToFlee()
{
	m_pCurrentSteeringBehavior = m_pFlee;
}

void SteeringAgent::SetToTurn()
{
	m_pCurrentSteeringBehavior = m_pTurn;
}
