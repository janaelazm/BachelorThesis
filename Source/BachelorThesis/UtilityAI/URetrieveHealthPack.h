#pragma once
#include "UUtilityAIAction.h"

class URetrieveHealthPack:UUtilityAIAction
{

	public:
	TArray<float> Utilities;
	TArray<float> Probabilities;
	float MaxExpectedUtility;
	
	// Calculate the utility of this action 
	virtual void CalculateUtility() override;

	// Calculate the probability of this action
	virtual void CalculateProbability() override;
};
