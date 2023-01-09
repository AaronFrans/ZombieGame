#include "stdafx.h"
#include "SteeringBehavior.h"


SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};

	Elite::Vector2 toTarget = m_Target - agentInfo.Position;

	steering.LinearVelocity = toTarget;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;



	//const Elite::Vector2 curDirection{ Elite::OrientationToVector(agentInfo.Orientation) };
	//
	//const float dot{ curDirection.Dot(toTarget.GetNormalized()) };
	//
	//if (dot < 1.0f)
	//{
	//	const float crossed{ curDirection.Cross(toTarget) };
	//
	//	steering.AngularVelocity = agentInfo.MaxAngularSpeed;
	//
	//	if (crossed < 0)
	//	{
	//		steering.AngularVelocity *= -1.0f;
	//	}
	//	const float velocityOffset{ 0.05f };
	//	steering.AngularVelocity *= 1.0f - dot / (1.0f) + 0.05f;
	//}
	return steering;
}

SteeringPlugin_Output ExploreWorld::CalculateSteering(float deltaT, AgentInfo agentInfo)
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

SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.AutoOrient = false;

	Elite::Vector2 toTarget = agentInfo.Position -  m_Target;

	steering.LinearVelocity = toTarget;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;



	const Elite::Vector2 curDirection{ Elite::OrientationToVector(agentInfo.Orientation) };

	const float dot{ curDirection.Dot(toTarget.GetNormalized()) };

	if (dot < 1.0f)
	{
		const float crossed{ curDirection.Cross(toTarget) };

		steering.AngularVelocity = -agentInfo.MaxAngularSpeed;

		if (crossed < 0)
		{
			steering.AngularVelocity *= -1.0f;
		}
		const float velocityOffset{ 0.05f };
		steering.AngularVelocity *= 1.0f - dot / + 0.05f;
	}

	steering.RunMode = true;
	return steering;
}

SteeringPlugin_Output Turn::CalculateSteering(float deltaT, AgentInfo agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.AutoOrient = false;

	steering.AngularVelocity = agentInfo.MaxAngularSpeed;

	return steering;
}
