//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}


	return steering;
}


//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 fromTarget = pAgent->GetPosition() - m_Target.Position;


	float distance = fromTarget.Magnitude();
	SteeringOutput steering = {};

	if (distance > m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;

	}

	steering.LinearVelocity = fromTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}


	return steering;
}


//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};


	Elite::Vector2 toTarget = m_Target.Position - pAgent->GetPosition();

	const float distance = toTarget.Magnitude();

	if (distance < m_TargetRadius)
	{
		steering.LinearVelocity = { 0.0f, 0.0f };

		return steering;
	}

	steering.LinearVelocity = toTarget;

	steering.LinearVelocity.Normalize();

	if (distance < m_SlowRadius)
		steering.LinearVelocity *= pAgent->GetMaxAngularSpeed() * (distance / m_SlowRadius);
	else
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}


	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	if (pAgent->IsAutoOrienting())
		pAgent->SetAutoOrient(false);

	SteeringOutput steering = {};

	Elite::Vector2 toTarget = m_Target.Position - pAgent->GetPosition();

	Elite::Vector2 agentDir = Elite::OrientationToVector(pAgent->GetRotation());

	float angle = atan2(agentDir.y, agentDir.x) - atan2(toTarget.y, toTarget.x);



	steering.AngularVelocity = -(angle * pAgent->GetMaxAngularSpeed());

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), agentDir, 5, { 0,1,0 });
	}


	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position + (m_Target.GetDirection() * m_Target.LinearVelocity) - pAgent->GetPosition();

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}


	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	Elite::Vector2 fromTarget = pAgent->GetPosition() - (m_Target.Position + (m_Target.GetDirection() * m_Target.LinearVelocity * deltaT));


	float distance = fromTarget.Magnitude();
	SteeringOutput steering = {};

	if (distance > m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;

	}

	steering.LinearVelocity = fromTarget.GetNormalized();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	steering.IsValid = true;

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}


	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	SteeringOutput steering = {};

	Elite::Vector2 wanderPoint = pAgent->GetDirection();

	wanderPoint.Normalize();
	wanderPoint *= m_OffsetDistance * 0;

	wanderPoint += pAgent->GetPosition();

	Elite::Vector2 wanderPointOffset = wanderPoint;

	m_MaxAngleChange = Elite::ToRadians(90);

	m_WanderAngle = pAgent->GetRotation() + Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);
	m_Radius = pAgent->GetRadius() + 5;
	wanderPointOffset.x += m_Radius * cos(m_WanderAngle);
	wanderPointOffset.y += m_Radius * sin(m_WanderAngle);

	steering.LinearVelocity = wanderPointOffset - pAgent->GetPosition();

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();


	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(wanderPoint, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawCircle(wanderPoint, m_Radius, { 1,1,1 }, 0.9f);
		DEBUGRENDERER2D->DrawPoint(wanderPointOffset, 5, { 1,1,0 });
	}

	return steering;

}
