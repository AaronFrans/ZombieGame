#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput output{};

	Elite::Vector2 flockAverage = m_pFlock->GetAverageNeighborPos();
	Elite::Vector2 destination{};
	if (flockAverage == Elite::ZeroVector2)
		return output;

	destination = flockAverage;

	Elite::Vector2 toDestination{ (destination - pAgent->GetPosition()).GetNormalized() };
	output.LinearVelocity = toDestination * pAgent->GetMaxLinearSpeed();
	return output;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput output{};
	Elite::Vector2 awayVector{};

	Elite::Vector2 flockAverage = m_pFlock->GetAverageNeighborPos();
	if (flockAverage == Elite::ZeroVector2)
	{
		return output;
	}

	Elite::Vector2 neighborToAgent{};
	for (int i{ 0 }; i < m_pFlock->GetNrOfNeighbors(); i++)
	{
		neighborToAgent = m_pFlock->GetNeighbors()[i]->GetPosition() - pAgent->GetPosition();
		float temp = neighborToAgent.MagnitudeSquared();
		awayVector += -neighborToAgent / temp;
	}
	output.LinearVelocity = awayVector.GetNormalized() * pAgent->GetMaxLinearSpeed();
	return output;
}

//*************************
//VELOCITY MATCH (FLOCKING)

SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput output{};
	output.LinearVelocity = m_pFlock->GetAverageNeighborVelocity().GetNormalized() * pAgent->GetMaxLinearSpeed();
	return output;
}
