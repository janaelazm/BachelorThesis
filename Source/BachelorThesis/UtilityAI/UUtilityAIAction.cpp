//#include "C:\Users\janae\Documents\Unreal Projects\BachelorThesis\Intermediate\Build\Win64\x64\BachelorThesisEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "UUtilityAIAction.h"

float UUtilityAIAction::CalculateMaxExpectedUtility()
{
	float sum = 0;
	
	for (int i = 0 ; i < Utilities.Num() ; i++ )
	{
		sum += Utilities[i]*Probabilities[i];
	}
	if (Utilities.Num() >= 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Sum is not normalized!"));
	}
	return sum;
}
