#pragma once
#include "AIController.h"
#include "UUtilityAIAction.generated.h"

UCLASS(Abstract)
class UUtilityAIAction : public UObject
{
	
	GENERATED_BODY()
	
public:
	TArray<float> Utilities;
	TArray<float> Probabilities;
	float MaxExpectedUtility;
	
	float CalculateMaxExpectedUtility();
	// Calculate the utility of this action 
	virtual void CalculateUtility(){};

	// Calculate the probability of this action
	virtual void CalculateProbability(){};
};
