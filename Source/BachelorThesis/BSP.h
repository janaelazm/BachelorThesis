#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FBinaryTree.h"
#include "NavigationSystem.h"
#include "BSP.generated.h"

UCLASS()
class BACHELORTHESIS_API ABSP : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABSP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BSP Settings", meta = (ExposeOnSpawn = "true"))
	FVector2D Dimensions;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BSP Settings", meta = (ExposeOnSpawn = "true"))
	float MinRoomSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BSP Settings", meta = (ExposeOnSpawn = "true"))
	float MaxDepth;

	float WallMeshSize = 400;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BSP Settings", meta = (ExposeOnSpawn = "true"))
	FBinaryTree BinaryTree;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall")
	TSubclassOf<AActor> WallActorBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> HealthPackActorBlueprint;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	ANavMeshBoundsVolume* BoundsVolume;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector PlayerstartLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> EnemyActorBlueprint;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FNode> Rooms;

	TArray<AActor*> HealthPacksInLevel;
	
	void Initialize();
	void GenerateWalls(FVector Position, FRotator WallRotation, FVector Scale);
	void Partition(FNode& CurrentNode);
	void GenerateRooms();
	void CalculateRoom(FNode* Room);
	void GenerateCorridors();
	TArray<FNode*> CalculateCorridors(FNode* FirstChild, FNode* SecondChild);
	void GeneratePartitionWalls();
	void SpawnRoomsAndCorridors();
	void SpawnHealthPacks();
	void SpawnEnemies();
	void GenerateBottomWalls(FNode* Room);
	void GenerateTopWalls(FNode* Room);
	void GenerateLeftWalls(FNode* Room);
	void GenerateRightWall(FNode* Room);
	void FindTheBestConnection(FNode* FirstChild, FNode* SecondChild);
	void TraverseTree(FNode* Start, TArray<FNode*>& Nodes, TArray<FNode*>& Corridors);
	bool AreRoomsFacingHorizontally(FVector TopChildBottomLeftCorner, FVector BottomChildBottomLeftCorner, FVector TopChildTopRightCorner, FVector BottomChildTopRightCorner);
	bool AreRoomsFacingVertically(FVector LeftChildBottomLeftCorner, FVector RightChildBottomLeftCorner, FVector LeftChildTopRightCorner, FVector RightChildTopRightCorner);
	bool CheckForIntersection(FNode& Corridor);
};
