#pragma once
#include "CoreMinimal.h"
#include "FNode.generated.h"

USTRUCT(BlueprintType)
struct FNode
{
	GENERATED_BODY()
	
	FNode* Parent;
	TArray<FNode*> Children;
	FNode* Sister;
	FVector PartitionDimensions;
	FVector BottomRightCorner;
	FRotator WallRotation;
	float Depth;
	TArray<TArray<FNode*>> CorridorConnections;

	// Default constructor
	FNode()
		: Parent(nullptr)
		, PartitionDimensions(FVector::ZeroVector)
		, BottomRightCorner(FVector::ZeroVector)
		, WallRotation(FRotator::ZeroRotator)
		, Depth(0.0f)
	{
		// Initialize the Children array with two nullptrs
		Children.SetNum(2); // Sets the array size to 2 with each element being nullptr
	}
};
