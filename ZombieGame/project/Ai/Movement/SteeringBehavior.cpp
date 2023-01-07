#include "stdafx.h"

#include "SteeringBehavior.h"


SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target - agentInfo.Position;

	steering.AngularVelocity = 10.f;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;


	return steering;
}

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo agentInfo)
{

	SteeringPlugin_Output steering = {};

	Elite::Vector2 wanderPoint = agentInfo.LinearVelocity.GetNormalized();

	wanderPoint.Normalize();
	wanderPoint *= m_OffsetDistance * 0;

	wanderPoint += agentInfo.Position;

	Elite::Vector2 wanderPointOffset = wanderPoint;

	m_MaxAngleChange = Elite::ToRadians(90);

	m_WanderAngle = agentInfo.Orientation + Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);
	m_Radius = agentInfo.AgentSize + 5;
	wanderPointOffset.x += m_Radius * cos(m_WanderAngle);
	wanderPointOffset.y += m_Radius * sin(m_WanderAngle);

	steering.LinearVelocity = wanderPointOffset - agentInfo.Position;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;


	return steering;
}
