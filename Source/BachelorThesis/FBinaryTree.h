#pragma once
#include "CoreMinimal.h"
#include "FNode.h"
#include "FBinaryTree.generated.h"

USTRUCT(Blueprintable)
struct FBinaryTree
{
	GENERATED_BODY()
	
	FNode* Root;
	TArray<FNode*> Partitions;
	TArray<FNode*> Rooms; //LeafChildren
	TArray<TArray<FNode*>> Corridors;
};
