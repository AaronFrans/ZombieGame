#include "stdafx.h"

#include "SteeringBehavior.h"


SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target - pAgent.Position;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.MaxLinearSpeed;


	return steering;
}