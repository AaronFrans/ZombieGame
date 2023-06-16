#include "stdafx.h"
#include "StatesAndTransitions.h"



void FSMStates::WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	pAgent->SetToWander();
}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{

	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return;

	AgarioFood* nearestFood;
	if (!pBlackboard->GetData("NearestFood", nearestFood) || !nearestFood) return;

	pAgent->SetToSeek(nearestFood->GetPosition());

}

bool FSMConditions::FoodNearbyCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{

	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;
	if (!pBlackboard->GetData("FoodVec", pFoodVec) || !pFoodVec) return false;

	const float radius{ 10.f };

	Elite::Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ 1,1,1 }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser = [agentPos](AgarioFood* pE1, AgarioFood* pE2) {

		float dist1{ pE1->GetPosition().DistanceSquared(agentPos) };
		float dist2{ pE2->GetPosition().DistanceSquared(agentPos) };

		return dist1 < dist2;
	};

	auto closestFoodIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), isCloser);

	if (closestFoodIt == pFoodVec->end()) return false;

	AgarioFood* closestFood = *closestFoodIt;
	if (closestFood->GetPosition().DistanceSquared(agentPos) > Elite::Square(radius)) return false;

	pBlackboard->ChangeData("NearestFood", closestFood);

	return true;
}

bool FSMConditions::AreaClearCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent) return false;
	if (!pBlackboard->GetData("FoodVec", pFoodVec) || !pFoodVec) return false;

	const float radius{ 10.f };

	Elite::Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Elite::Color{ 1,1,1 }, DEBUGRENDERER2D->NextDepthSlice());

	for (auto& foodItem : *pFoodVec)
	{
		if (foodItem->GetPosition().DistanceSquared(agentPos) < Elite::Square(radius))
		{
			return false;
		}
	}


	return true;
}
