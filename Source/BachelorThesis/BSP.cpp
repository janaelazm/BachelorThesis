//#include "C:\Users\janae\Documents\Unreal Projects\BachelorThesis\Intermediate\Build\Win64\x64\BachelorThesisEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "BSP.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Sort.h"
#include "Algo/Unique.h"
#include "Engine/StaticMeshActor.h"
#include "Generators/RectangleMeshGenerator.h"

// Sets default values
ABSP::ABSP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABSP::BeginPlay()
{
	Super::BeginPlay();

	//Initialize();
	BinaryTree.Root = new FNode();
	BinaryTree.Root->BottomRightCorner = FVector(GetActorLocation().X, GetActorLocation().Y, 0);
	BinaryTree.Root->PartitionDimensions = FVector(Dimensions.X, Dimensions.Y,0);
	BinaryTree.Partitions.Add(BinaryTree.Root);
	Partition(*BinaryTree.Root);
	//GeneratePartitionWalls();
	GenerateRooms();
	GenerateCorridors();
	PlayerstartLocation = FVector(BinaryTree.Rooms[0]->BottomRightCorner.X + BinaryTree.Rooms[0]->PartitionDimensions.X/2, BinaryTree.Rooms[0]->BottomRightCorner.Y + BinaryTree.Rooms[0]->PartitionDimensions.Y/2,10);
	SpawnRoomsAndCorridors();
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	PlayerPawn->SetActorLocation(PlayerstartLocation);
	SpawnHealthPacks();
	SpawnEnemies();
}

void ABSP::Initialize()
{
	// Room dimensions
	//Dimensions = FVector2D(500, 500);
	
	// Calculate wall positions
	FVector WallOne = GetActorLocation(); // First wall starts at actor location
	FVector WallTwo = FVector(GetActorLocation().X + Dimensions.X, GetActorLocation().Y, GetActorLocation().Z);
	FVector WallThree = FVector(GetActorLocation().X+Dimensions.X, GetActorLocation().Y + Dimensions.Y , GetActorLocation().Z);
	FVector WallFour = FVector(GetActorLocation().X, GetActorLocation().Y + Dimensions.Y, GetActorLocation().Z);

	// Calculate wall rotations
	FRotator WallOneRotation = GetActorRotation(); // No rotation
	FRotator WallTwoRotation = FRotator(0.0f, 90.0f, 0.0f); // 90 degrees
	FRotator WallThreeRotation = FRotator(0.0f, 180.0f, 0.0f); // 180 degrees
	FRotator WallFourRotation = FRotator(0.0f, -90.0f, 0.0f); // -90 degrees

	// Generate walls
	GenerateWalls(WallOne, WallOneRotation, FVector(Dimensions.X / WallMeshSize, 1, 1));
	GenerateWalls(WallTwo, WallTwoRotation, FVector(Dimensions.X / WallMeshSize, 1, 1));
	GenerateWalls(WallThree, WallThreeRotation, FVector(Dimensions.X / WallMeshSize, 1, 1));
	GenerateWalls(WallFour, WallFourRotation, FVector(Dimensions.X / WallMeshSize, 1, 1));
}


void ABSP::GenerateWalls(FVector Position, FRotator WallRotation, FVector Scale)
{
	if (WallActorBlueprint) // Ensure WallActorClass is valid
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;       // Set the BSP actor as the owner
		SpawnParams.Instigator = GetInstigator();

		// Spawn a new wall actor
		AActor* NewWall = GetWorld()->SpawnActor<AActor>(WallActorBlueprint, Position, WallRotation, SpawnParams);

		if (NewWall)
		{
			TArray<AActor*> AllActors;
			// Apply the scale to the spawned actor
			NewWall->SetActorScale3D(Scale);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WallActorClass is not set! Cannot spawn walls."));
	}
}

void ABSP::Partition(FNode &CurrentNode)
{
	if (CurrentNode.Depth >= MaxDepth)
	{
		return;
	}
	
	FNode* NewNodeA = new FNode();
	FNode* NewNodeB = new FNode();
	
	NewNodeA->Parent = &CurrentNode;
	NewNodeB->Parent = &CurrentNode;
	
	bool YAxisSplit;
	if (CurrentNode.PartitionDimensions.X <= CurrentNode.PartitionDimensions.Y)
	{
		YAxisSplit = true;
	}
	else
	{
		YAxisSplit = false;
	}
	
    if (YAxisSplit)
    {
        float MinSplitY = CurrentNode.BottomRightCorner.Y + MinRoomSize;
        float MaxSplitY = CurrentNode.BottomRightCorner.Y + CurrentNode.PartitionDimensions.Y - MinRoomSize;
        float SplitPointY = FMath::FRandRange(FMath::Max(MinSplitY, CurrentNode.BottomRightCorner.Y ), 
                                              FMath::Min(MaxSplitY, CurrentNode.BottomRightCorner.Y + CurrentNode.PartitionDimensions.Y));

        // Split vertically
        NewNodeB->BottomRightCorner = FVector(CurrentNode.BottomRightCorner.X, SplitPointY,0);
        NewNodeB->PartitionDimensions = FVector(CurrentNode.PartitionDimensions.X, (CurrentNode.BottomRightCorner.Y + CurrentNode.PartitionDimensions.Y) - SplitPointY, 0);
        NewNodeB->WallRotation = FRotator(0, 0, 0);

        NewNodeA->BottomRightCorner = CurrentNode.BottomRightCorner;
        NewNodeA->PartitionDimensions = FVector(CurrentNode.PartitionDimensions.X, SplitPointY - CurrentNode.BottomRightCorner.Y,0);
        NewNodeA->WallRotation = FRotator(0, 0, 0);
    }
    else
    {
        float MinSplitX = CurrentNode.BottomRightCorner.X + MinRoomSize;
        float MaxSplitX = CurrentNode.BottomRightCorner.X + CurrentNode.PartitionDimensions.X - MinRoomSize;
        float SplitPointX = FMath::FRandRange(FMath::Max(MinSplitX, CurrentNode.BottomRightCorner.X),
                                              FMath::Min(MaxSplitX, CurrentNode.BottomRightCorner.X + CurrentNode.PartitionDimensions.X ));

        // Split horizontally
        NewNodeB->BottomRightCorner = FVector(SplitPointX, CurrentNode.BottomRightCorner.Y,0);
        NewNodeB->PartitionDimensions = FVector((CurrentNode.BottomRightCorner.X + CurrentNode.PartitionDimensions.X) - SplitPointX, CurrentNode.PartitionDimensions.Y,0);
        NewNodeB->WallRotation = FRotator(0, -90, 0);

        NewNodeA->BottomRightCorner = CurrentNode.BottomRightCorner;
        NewNodeA->PartitionDimensions = FVector(SplitPointX - CurrentNode.BottomRightCorner.X, CurrentNode.PartitionDimensions.Y,0);
        NewNodeA->WallRotation = FRotator(0, -90, 0);
    }

	BinaryTree.Partitions.Add(NewNodeA);
	BinaryTree.Partitions.Add(NewNodeB);
	
	CurrentNode.Children[0] = NewNodeA;
	CurrentNode.Children[1] = NewNodeB;

	NewNodeB->Depth = CurrentNode.Depth+1;
	NewNodeA->Depth = CurrentNode.Depth+1;

	NewNodeB->Sister = NewNodeA;
	NewNodeA->Sister = NewNodeB;

	Partition(*NewNodeA);
	Partition(*NewNodeB);
	
}

void ABSP::GenerateRooms()
{
    for (int i = 0; i < BinaryTree.Partitions.Num(); i++)
    {
    	if (BinaryTree.Partitions[i]->Children[0] == nullptr && BinaryTree.Partitions[i]->Children[1] == nullptr)
    	{
    		CalculateRoom(BinaryTree.Partitions[i]);
    	}
    }
}

void ABSP:: CalculateRoom(FNode* Room)
{
	float Width, Height, RandomX, RandomY;
	
	// Ensure width and height stay within partition bounds and aspect ratio constraints
	if (Room->PartitionDimensions.X < MinRoomSize/2)
	{
		Width = Room->PartitionDimensions.X;
	}
	else
	{
		Width = FMath::FRandRange(MinRoomSize/2, Room->PartitionDimensions.X);
	}
	if (Room->PartitionDimensions.Y < MinRoomSize/2)
	{
		Height = Room->PartitionDimensions.Y;
	}
	else
	{
		Height = FMath::FRandRange(MinRoomSize/2,  Room->PartitionDimensions.Y);
	}

	//Spawnpoint within partition
	RandomX = FMath::FRandRange(
	Room->BottomRightCorner.X,
	Room->BottomRightCorner.X + Room->PartitionDimensions.X - Width
	);
	RandomY = FMath::FRandRange(
	Room->BottomRightCorner.Y,
	Room->BottomRightCorner.Y + Room->PartitionDimensions.Y - Height
	);
	    	
	//Update the partition with room data
	Room->BottomRightCorner.X = RandomX;
	Room->BottomRightCorner.Y = RandomY;
	Room->PartitionDimensions = FVector(Width, Height,0);
	
	BinaryTree.Rooms.Add(Room);	
}

void ABSP::GenerateCorridors()
{

	for (int i = 0; i < BinaryTree.Rooms.Num()-1; i+=2)
	{
		FindTheBestConnection(BinaryTree.Rooms[i], BinaryTree.Rooms[i]->Sister);
	}

	TArray<FNode*> Rest;

	//Start at 1 to ignore root
	for (int i = 1; i < BinaryTree.Partitions.Num(); i++)
	{
		if (Rest.Contains(BinaryTree.Partitions[i]) || Rest.Contains(BinaryTree.Partitions[i]->Sister))
		{
			continue;
		}
		if (BinaryTree.Partitions[i]->Children[0] != nullptr && BinaryTree.Partitions[i]->Children[1] != nullptr)
		{
			Rest.Add(BinaryTree.Partitions[i]);
		}
	}
	
	for (int i = 0; i < Rest.Num(); i++)
	{
		FindTheBestConnection(Rest[i], Rest[i]->Sister);
	}
	
}

TArray<FNode*> ABSP::CalculateCorridors(FNode* FirstChild, FNode* SecondChild)
{
	FVector2D CorridorStart, CorridorEnd;
	FNode RightChild, LeftChild, TopChild, BottomChild;
	FNode* NewCorridor = new FNode();
	FNode* NewCorridorTwo = new FNode();
	TArray<FNode*> ResultCorridors;
	
	if (FirstChild->BottomRightCorner.X > SecondChild->BottomRightCorner.X)
	{
		RightChild  =  *SecondChild;
		LeftChild =  *FirstChild;
	}
	else
	{
		RightChild =  *FirstChild;
		LeftChild =  *SecondChild;
	}

	if (FirstChild->BottomRightCorner.Y + FirstChild->PartitionDimensions.Y > SecondChild->BottomRightCorner.Y + SecondChild->PartitionDimensions.Y)
	{
		TopChild = *FirstChild;
		BottomChild =  *SecondChild;
	}
	else
	{
		TopChild =  *SecondChild;
		BottomChild =  *FirstChild;
	}
	
	if (AreRoomsFacingVertically(
		FVector(RightChild.BottomRightCorner.X, RightChild.BottomRightCorner.Y,0),
		FVector(RightChild.BottomRightCorner.X + RightChild.PartitionDimensions.X, RightChild.BottomRightCorner.Y + RightChild.PartitionDimensions.Y,0),
		FVector(LeftChild.BottomRightCorner.X, LeftChild.BottomRightCorner.Y, 0),
		FVector(LeftChild.BottomRightCorner.X + LeftChild.PartitionDimensions.X, LeftChild.BottomRightCorner.Y + LeftChild.PartitionDimensions.Y, 0)))
	{
		CorridorStart = FVector2D(
			FMath::Max(RightChild.BottomRightCorner.X, LeftChild.BottomRightCorner.X),
			FMath::Min(FirstChild->BottomRightCorner.Y + FirstChild->PartitionDimensions.Y, SecondChild->BottomRightCorner.Y + SecondChild->PartitionDimensions.Y));
		CorridorEnd = FVector2D(
			FMath::Max(RightChild.BottomRightCorner.X, LeftChild.BottomRightCorner.X),
			FMath::Max(FirstChild->BottomRightCorner.Y, SecondChild->BottomRightCorner.Y));
		
		NewCorridor->PartitionDimensions =  FVector(WallMeshSize, FVector2D::Distance(CorridorStart, CorridorEnd), 0);
		NewCorridor->BottomRightCorner = FVector(CorridorStart.X, CorridorStart.Y,0);

		ResultCorridors.Add(NewCorridor);
	
		delete NewCorridorTwo;
	}
	
	else if (AreRoomsFacingHorizontally(
	FVector(TopChild.BottomRightCorner.X, TopChild.BottomRightCorner.Y,0),
		FVector(TopChild.BottomRightCorner.X + TopChild.PartitionDimensions.X, TopChild.BottomRightCorner.Y + TopChild.PartitionDimensions.Y,0),
		FVector(BottomChild.BottomRightCorner.X, BottomChild.BottomRightCorner.Y, 0),
		FVector(BottomChild.BottomRightCorner.X + BottomChild.PartitionDimensions.X, BottomChild.BottomRightCorner.Y + BottomChild.PartitionDimensions.Y, 0)))
	{
		CorridorStart =  FVector2D(
			FMath::Min(FirstChild->BottomRightCorner.X + FirstChild->PartitionDimensions.X, SecondChild->BottomRightCorner.X + SecondChild->PartitionDimensions.X),
			FMath::Max(TopChild.BottomRightCorner.Y , BottomChild.BottomRightCorner.Y));
		CorridorEnd = FVector2D(
			FMath::Max(FirstChild->BottomRightCorner.X, SecondChild->BottomRightCorner.X),
			FMath::Max(TopChild.BottomRightCorner.Y, BottomChild.BottomRightCorner.Y));
		
		NewCorridor->PartitionDimensions =  FVector(FVector2D::Distance(CorridorStart, CorridorEnd), WallMeshSize, 0);
		NewCorridor->BottomRightCorner = FVector(CorridorStart.X, CorridorStart.Y,0);

		ResultCorridors.Add(NewCorridor);
		
		delete NewCorridorTwo;
	}
	
	else
	{
		if ( LeftChild.BottomRightCorner != TopChild.BottomRightCorner) //First child is to the right and is lower than second
		{
			// Calculate the midpoint where the corridor will change direction
			FVector2D MidPoint = FVector2D(LeftChild.BottomRightCorner.X + LeftChild.PartitionDimensions.X - WallMeshSize, TopChild.BottomRightCorner.Y + TopChild.PartitionDimensions.Y -WallMeshSize);

			// Generate the horizontal corridor segment
			FVector2D HorizontalCorridorStart = FVector2D(TopChild.BottomRightCorner.X + TopChild.PartitionDimensions.X, MidPoint.Y);
			FVector2D HorizontalCorridorEnd = MidPoint;

			// Generate the vertical corridor segment
			FVector2D VerticalCorridorStart =  FVector2D(MidPoint.X , LeftChild.BottomRightCorner.Y + LeftChild.PartitionDimensions.Y);
			FVector2D VerticalCorridorEnd = MidPoint;
			
			NewCorridor->PartitionDimensions = FVector(FVector2D::Distance(HorizontalCorridorStart, HorizontalCorridorEnd)+WallMeshSize, WallMeshSize,0);
			NewCorridor->BottomRightCorner = FVector(HorizontalCorridorStart.X, HorizontalCorridorStart.Y,0);
			
			NewCorridorTwo->PartitionDimensions = FVector( WallMeshSize, FVector2D::Distance(VerticalCorridorStart, VerticalCorridorEnd),0);
			NewCorridorTwo->BottomRightCorner = FVector(VerticalCorridorStart.X, VerticalCorridorStart.Y,0);
			
			if (CheckForIntersection(*NewCorridor) || CheckForIntersection(*NewCorridorTwo))
			{
				/// Calculate the midpoint where the corridor will change direction
				MidPoint = FVector2D(TopChild.BottomRightCorner.X, LeftChild.BottomRightCorner.Y);

				// Generate the horizontal corridor segment
				HorizontalCorridorStart = MidPoint;
				HorizontalCorridorEnd = FVector2D(LeftChild.BottomRightCorner.X, MidPoint.Y);

				// Generate the vertical corridor segment
				VerticalCorridorStart =  MidPoint;
				VerticalCorridorEnd = FVector2D(MidPoint.X , TopChild.BottomRightCorner.Y);
				
				NewCorridor->PartitionDimensions = FVector(FVector2D::Distance(HorizontalCorridorStart, HorizontalCorridorEnd), WallMeshSize,0);
				NewCorridor->BottomRightCorner = FVector(HorizontalCorridorStart.X + WallMeshSize, HorizontalCorridorStart.Y,0);
				
				NewCorridorTwo->PartitionDimensions = FVector( WallMeshSize, FVector2D::Distance(VerticalCorridorStart, VerticalCorridorEnd),0);
				NewCorridorTwo->BottomRightCorner = FVector(VerticalCorridorStart.X, VerticalCorridorStart.Y,0);

				//UE_LOG(LogTemp, Error, TEXT("Corridors Intersecting, new one generated at: %f, %f"), NewCorridor->TopLeftCorner.X , NewCorridor->TopLeftCorner.Y);
			}
			NewCorridor->CorridorConnections.Add(TArray<FNode*>{NewCorridorTwo});
			NewCorridorTwo->CorridorConnections.Add(TArray<FNode*>{NewCorridor});
			ResultCorridors.Add(NewCorridor);
			ResultCorridors.Add(NewCorridorTwo);
			
		}
		if (LeftChild.BottomRightCorner == TopChild.BottomRightCorner)
		{
			//Calculate the midpoint where the corridor will change direction
			FVector2D MidPoint = FVector2D(BottomChild.BottomRightCorner.X , (LeftChild.BottomRightCorner.Y + LeftChild.PartitionDimensions.Y) -WallMeshSize);

			// Generate the horizontal corridor segment
			FVector2D HorizontalCorridorStart = MidPoint;
			FVector2D HorizontalCorridorEnd = FVector2D(LeftChild.BottomRightCorner.X, MidPoint.Y);

			// Generate the vertical corridor segment
			FVector2D VerticalCorridorStart = FVector2D(MidPoint.X, BottomChild.BottomRightCorner.Y + BottomChild.PartitionDimensions.Y);
			FVector2D VerticalCorridorEnd = MidPoint;
			
			NewCorridor->PartitionDimensions = FVector(FVector2D::Distance(HorizontalCorridorStart, HorizontalCorridorEnd), WallMeshSize,0);
			NewCorridor->BottomRightCorner = FVector(HorizontalCorridorStart.X, HorizontalCorridorStart.Y,0);
			
			NewCorridorTwo->PartitionDimensions = FVector( WallMeshSize, FVector2D::Distance(VerticalCorridorStart, VerticalCorridorEnd),0);
			NewCorridorTwo->BottomRightCorner = FVector(VerticalCorridorStart.X, VerticalCorridorStart.Y ,0);

			if (CheckForIntersection(*NewCorridor) || CheckForIntersection(*NewCorridorTwo))
			{
				//Calculate the midpoint where the corridor will change direction
				MidPoint = FVector2D(LeftChild.BottomRightCorner.X + LeftChild.PartitionDimensions.X - WallMeshSize , BottomChild.BottomRightCorner.Y);

				// Generate the horizontal corridor segment
				HorizontalCorridorStart = FVector2D(BottomChild.BottomRightCorner.X + BottomChild.PartitionDimensions.X, MidPoint.Y);
				HorizontalCorridorEnd = MidPoint;

				// Generate the vertical corridor segment
				VerticalCorridorStart = MidPoint;
				VerticalCorridorEnd = FVector2D(MidPoint.X, LeftChild.BottomRightCorner.Y );
				
				NewCorridor->PartitionDimensions = FVector(FVector2D::Distance(HorizontalCorridorStart, HorizontalCorridorEnd), WallMeshSize,0);
				NewCorridor->BottomRightCorner = FVector(HorizontalCorridorStart.X, HorizontalCorridorStart.Y,0);
			
				NewCorridorTwo->WallRotation = FRotator(0, 0, 90);
				NewCorridorTwo->PartitionDimensions = FVector( WallMeshSize, FVector2D::Distance(VerticalCorridorStart, VerticalCorridorEnd),0);
				NewCorridorTwo->BottomRightCorner = FVector(VerticalCorridorStart.X, VerticalCorridorStart.Y,0);
			}
			NewCorridor->CorridorConnections.Add(TArray<FNode*>{NewCorridorTwo});
			NewCorridorTwo->CorridorConnections.Add(TArray<FNode*>{NewCorridor});
			ResultCorridors.Add(NewCorridor);
			ResultCorridors.Add(NewCorridorTwo);
		}
	}

	return ResultCorridors;
	
}

void ABSP::FindTheBestConnection(FNode* FirstChild, FNode* SecondChild)
{
	TArray<FNode*> FirstChildLeafs;
	TArray<FNode*> FirstChildCorridors;
	TArray<FNode*> SecondChildLeafs;
	TArray<FNode*> SecondChildCorridors;

	TraverseTree(FirstChild, FirstChildLeafs, FirstChildCorridors);
	TraverseTree(SecondChild, SecondChildLeafs, SecondChildCorridors);

	TArray<FNode*> NewCorridor;

	//check for connection from room to room
	for (auto FirstChildLeaf : FirstChildLeafs)
	{
		for (auto SecondChildLeaf : SecondChildLeafs)
		{
			NewCorridor = CalculateCorridors(FirstChildLeaf, SecondChildLeaf);
			if (CheckForIntersection(*NewCorridor[0]) || (NewCorridor.Num() == 2 && CheckForIntersection(*NewCorridor[1])))
			{
				continue;
			}
			FirstChildLeaf->CorridorConnections.Add(NewCorridor);
			SecondChildLeaf->CorridorConnections.Add(NewCorridor);
			for (auto New : NewCorridor)
			{
				New->CorridorConnections.Add({FirstChildLeaf});
				New->CorridorConnections.Add({SecondChildLeaf});
			}
			BinaryTree.Corridors.Add(NewCorridor);
			return;
		}
	}
	
	//if first level corridors have been generated check if connections from corridor to corridor
	//or from corridor to room are possible
	if (!FirstChildCorridors.IsEmpty() && !SecondChildCorridors.IsEmpty() && BinaryTree.Corridors.Num() >= 8)
	{
		for (auto CorridorA : FirstChildCorridors)
		{
			for (auto CorridorB : SecondChildCorridors)
			{
				NewCorridor = CalculateCorridors(CorridorA, CorridorB);
				if (CheckForIntersection(*NewCorridor[0]) || (NewCorridor.Num() == 2 && CheckForIntersection(*NewCorridor[1])) )
				{
					continue;
				}
				//if corridors aren't intersecting add them to corridor list
				FirstChild->CorridorConnections.Add(NewCorridor);
				SecondChild->CorridorConnections.Add(NewCorridor);
				CorridorB->CorridorConnections.Add(NewCorridor);
				CorridorA->CorridorConnections.Add(NewCorridor);
				for (auto New : NewCorridor)
				{
					New->CorridorConnections.Add({CorridorA});
					New->CorridorConnections.Add({CorridorB});
				}
				BinaryTree.Corridors.Add(NewCorridor);
				return;
			}
		}
		
		for (auto FirstChildLeaf : FirstChildLeafs)
		{
			for (auto SecondChildLeaf : SecondChildLeafs)
			{
				for (auto CorridorA : FirstChildCorridors)
				{
					for (auto CorridorB : SecondChildCorridors)
					{
						NewCorridor = CalculateCorridors(CorridorA, SecondChildLeaf);
						if (CheckForIntersection(*NewCorridor[0]) || (NewCorridor.Num() == 2 && CheckForIntersection(*NewCorridor[1])))
						{
							NewCorridor = CalculateCorridors(CorridorB, FirstChildLeaf);
							if (CheckForIntersection(*NewCorridor[0]) || (NewCorridor.Num() == 2 && CheckForIntersection(*NewCorridor[1])))
							{
								continue;
							}
							FirstChildLeaf->CorridorConnections.Add(NewCorridor);
							CorridorB->CorridorConnections.Add(NewCorridor);
							for (auto New : NewCorridor)
							{
								New->CorridorConnections.Add({FirstChildLeaf});
								New->CorridorConnections.Add({CorridorB});
							}
							BinaryTree.Corridors.Add(NewCorridor);
							return;
						}
							SecondChildLeaf->CorridorConnections.Add(NewCorridor);
							CorridorA->CorridorConnections.Add(NewCorridor);
							for (auto New : NewCorridor)
							{
								New->CorridorConnections.Add({SecondChildLeaf});
								New->CorridorConnections.Add({CorridorA});
							}
							BinaryTree.Corridors.Add(NewCorridor);
						return;
					}
				}
			}
		}
	}
}

void ABSP::TraverseTree(FNode* Start, TArray<FNode*>& Nodes, TArray<FNode*>& Corridors)
{
	if (Start->Children[0] != nullptr && Start->Children[1] != nullptr)
	{
		TraverseTree(Start->Children[0], Nodes, Corridors);
		TraverseTree(Start->Children[1], Nodes, Corridors);
	}
	else
	{
		Nodes.Add(Start);
		for (auto CorridorArray : Start->CorridorConnections)
		{
			for (auto Corridor : CorridorArray)
			{
				if (!Corridors.Contains(Corridor))  // Prevent duplicate corridors
				{
					Corridors.Add(Corridor);
				}
			}
		}
	}
	
}


bool ABSP::AreRoomsFacingHorizontally(FVector TopChildBottomLeftCorner, FVector TopChildTopRightCorner, FVector BottomChildBottomLeftCorner, FVector BottomChildTopRightCorner)
{
	
	float Top_MinY = FMath::Min(TopChildBottomLeftCorner.Y,TopChildTopRightCorner.Y) ;
	float Top_MaxY =  FMath::Max(TopChildBottomLeftCorner.Y,TopChildTopRightCorner.Y) ;
	
	float Bottom_MinY = FMath::Min(BottomChildBottomLeftCorner.Y, BottomChildTopRightCorner.Y);
	float Bottom_MaxY = FMath::Max(BottomChildBottomLeftCorner.Y, BottomChildTopRightCorner.Y);

	return Top_MaxY > Bottom_MinY && Top_MinY < Bottom_MaxY  && (FMath::Abs(Top_MinY - Bottom_MaxY) >= WallMeshSize); 
}

bool ABSP::AreRoomsFacingVertically(FVector LeftChildBottomLeftCorner, FVector LeftChildTopRightCorner, FVector RightChildBottomLeftCorner, FVector RightChildTopRightCorner)
{
	float Left_MinX = FMath::Min(LeftChildBottomLeftCorner.X, LeftChildTopRightCorner.X);
	float Left_MaxX =  FMath::Max(LeftChildBottomLeftCorner.X, LeftChildTopRightCorner.X);

	float Right_MinX = FMath::Min(RightChildBottomLeftCorner.X, RightChildTopRightCorner.X);
	float Right_MaxX = FMath::Max(RightChildBottomLeftCorner.X, RightChildTopRightCorner.X);
	
	return Left_MaxX > Right_MinX && Left_MinX < Right_MaxX && FMath::Abs(Left_MaxX - Right_MinX) >= WallMeshSize ; 
}

bool ABSP::CheckForIntersection(FNode& Corridor)
{
	bool OverlapX, OverlapY;

	// Bottom-Right corner (spawn point)
	FVector BottomRight = FVector(
		FMath::Min(Corridor.BottomRightCorner.X, Corridor.BottomRightCorner.X + Corridor.PartitionDimensions.X),
		FMath::Min(Corridor.BottomRightCorner.Y, Corridor.BottomRightCorner.Y + Corridor.PartitionDimensions.Y),
		0);

	// Top-Left: Transform the mesh's max local corner to world space
	FVector TopLeft = FVector(
		FMath::Max(Corridor.BottomRightCorner.X, Corridor.BottomRightCorner.X + Corridor.PartitionDimensions.X),
		FMath::Max(Corridor.BottomRightCorner.Y, Corridor.BottomRightCorner.Y + Corridor.PartitionDimensions.Y),
		0);
	
	if (!BinaryTree.Corridors.IsEmpty())
	{
		for (auto CorridorArray : BinaryTree.Corridors)
		{
			for (auto ExistingCorridor : CorridorArray)
			{
				// Bottom-left corner (spawn point)
				FVector ExistingBottomRight = FVector(
					FMath::Min(ExistingCorridor->BottomRightCorner.X, ExistingCorridor->BottomRightCorner.X + ExistingCorridor->PartitionDimensions.X),
					FMath::Min(ExistingCorridor->BottomRightCorner.Y, ExistingCorridor->BottomRightCorner.Y + ExistingCorridor->PartitionDimensions.Y),
					0);

				// Top-right: Transform the mesh's max local corner to world space
				FVector ExistingTopLeft = FVector(
				FMath::Max(ExistingCorridor->BottomRightCorner.X, ExistingCorridor->BottomRightCorner.X + ExistingCorridor->PartitionDimensions.X),
				FMath::Max(ExistingCorridor->BottomRightCorner.Y, ExistingCorridor->BottomRightCorner.Y + ExistingCorridor->PartitionDimensions.Y),
				0);
				
				// Check for overlap on X and Y axes
				OverlapX = (BottomRight.X < ExistingTopLeft.X) && (TopLeft.X > ExistingBottomRight.X);
				OverlapY = (BottomRight.Y < ExistingTopLeft.Y) && (TopLeft.Y > ExistingBottomRight.Y);
				
				if (OverlapX && OverlapY)
				{
					return true;
				}
			}
		}
	}

	if (!BinaryTree.Rooms.IsEmpty())
	{
		for (int r=0 ; r < BinaryTree.Rooms.Num(); r++)
		{
			FVector MinExisting = BinaryTree.Rooms[r]->BottomRightCorner;
			FVector MaxExisting = BinaryTree.Rooms[r]->BottomRightCorner + BinaryTree.Rooms[r]->PartitionDimensions;

			// Check for overlap on the X and Y axes
			OverlapX = (BottomRight.X < MaxExisting.X) && (TopLeft.X > MinExisting.X);
			OverlapY = (BottomRight.Y < MaxExisting.Y) && (TopLeft.Y > MinExisting.Y);

			if (OverlapX && OverlapY)
			{
				return true;
			}
		}
	}
	return false;
}



void ABSP::GeneratePartitionWalls()
{
	for (int i = 0; i < BinaryTree.Partitions.Num(); i++)
	{
			if (BinaryTree.Partitions[i]->WallRotation.Yaw == -90)
			{
				GenerateWalls(FVector(BinaryTree.Partitions[i]->BottomRightCorner.X, BinaryTree.Partitions[i]->BottomRightCorner.Y + BinaryTree.Partitions[i]->PartitionDimensions.Y, 0),
				BinaryTree.Partitions[i]->WallRotation,
				FVector(BinaryTree.Partitions[i]->PartitionDimensions.Y / WallMeshSize, 1, 1));
			}
			else
			{
				GenerateWalls(FVector(BinaryTree.Partitions[i]->BottomRightCorner.X, BinaryTree.Partitions[i]->BottomRightCorner.Y + BinaryTree.Partitions[i]->PartitionDimensions.Y, 0),
					BinaryTree.Partitions[i]->WallRotation,
					FVector(BinaryTree.Partitions[i]->PartitionDimensions.X / WallMeshSize, 1, 1));
			}
	}
		GenerateWalls(FVector(GetActorLocation().X, GetActorLocation().Y, 0),FRotator(0, 0, 90), FVector(Dimensions.X / WallMeshSize, 1, Dimensions.Y / WallMeshSize));
}

void ABSP::SpawnHealthPacks()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;       // Set the BSP actor as the owner
	SpawnParams.Instigator = GetInstigator();
	
	for (int i = 0 ; i < BinaryTree.Rooms.Num(); i+=2)
	{
		float RandomX = FMath::FRandRange(BinaryTree.Rooms[i]->BottomRightCorner.X, BinaryTree.Rooms[i]->BottomRightCorner.X + BinaryTree.Rooms[i]->PartitionDimensions.X);
		float RandomY = FMath::FRandRange(BinaryTree.Rooms[i]->BottomRightCorner.Y, BinaryTree.Rooms[i]->BottomRightCorner.Y + BinaryTree.Rooms[i]->PartitionDimensions.Y);
		AActor* NewHealthPack = GetWorld()->SpawnActor<AActor>(HealthPackActorBlueprint, FVector(RandomX, RandomY,10), FRotator(0, 0, 0), SpawnParams);
		HealthPacksInLevel.Add(NewHealthPack);
	}
}

void ABSP::SpawnEnemies()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;       // Set the BSP actor as the owner
	SpawnParams.Instigator = GetInstigator();
	
	for (int i = 1 ; i < BinaryTree.Rooms.Num(); i+=2)
	{
		float RandomX = FMath::FRandRange(BinaryTree.Rooms[i]->BottomRightCorner.X, BinaryTree.Rooms[i]->BottomRightCorner.X + BinaryTree.Rooms[i]->PartitionDimensions.X);
		float RandomY = FMath::FRandRange(BinaryTree.Rooms[i]->BottomRightCorner.Y, BinaryTree.Rooms[i]->BottomRightCorner.Y + BinaryTree.Rooms[i]->PartitionDimensions.Y);
		AActor* NewHealthPack = GetWorld()->SpawnActor<AActor>(EnemyActorBlueprint, FVector(RandomX, RandomY,50), FRotator(0, 0, 0), SpawnParams);
	}
}

void ABSP::SpawnRoomsAndCorridors()
{
	for (auto CorridorArray : BinaryTree.Corridors)
	{
		for (auto Corridor : CorridorArray)
		{
			//Floor
			GenerateWalls(FVector(Corridor->BottomRightCorner.X, Corridor->BottomRightCorner.Y, 0), FRotator(0,0,90), FVector(Corridor->PartitionDimensions.X/WallMeshSize, 1, Corridor->PartitionDimensions.Y/WallMeshSize));
			//Walls
			GenerateBottomWalls(Corridor);
			GenerateTopWalls(Corridor);
			GenerateLeftWalls(Corridor);
			GenerateRightWall(Corridor);
		}
	}
	for(auto Room : BinaryTree.Rooms)
	{
		//Floor
		GenerateWalls(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y, 0), FRotator(0,0,90), FVector(Room->PartitionDimensions.X / WallMeshSize, 1, Room->PartitionDimensions.Y / WallMeshSize)); //Floor
		//Walls
		GenerateBottomWalls(Room);
		GenerateTopWalls(Room);
		GenerateLeftWalls(Room);
		GenerateRightWall(Room);
	}
}


void ABSP::GenerateBottomWalls(FNode* Room)
{
	TArray<FNode*> BottomWalls;
	
	for (auto CorridorArray : Room->CorridorConnections)
	{
		for(auto Corridor : CorridorArray){
			//Bottom wall corridors
			if ((Corridor->BottomRightCorner.Y + Corridor->PartitionDimensions.Y) == Room->BottomRightCorner.Y)
			{
				if (Room->PartitionDimensions.X < Corridor->PartitionDimensions.X)
				{
					return;
				}
				if ( (Corridor->BottomRightCorner.X >= Room->BottomRightCorner.X && Corridor->BottomRightCorner.X < Room->BottomRightCorner.X + Room->PartitionDimensions.X)){
					BottomWalls.Add(Corridor);
				}
			}
		}
	}
		
		//BottomWalls
		Algo::Sort(BottomWalls, [](const FNode* A, const FNode* B)
			{
		return A->BottomRightCorner.X < B->BottomRightCorner.X;
			});
	
		if (BottomWalls.Num() > 0)
		{
			float Scale=0;
			//BOTTOM WALLS//
			for (int i = 0 ; i < BottomWalls.Num(); i++)
			{
				if (i == 0)
				{
					Scale = FVector::Distance(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y,0), FVector(BottomWalls[i]->BottomRightCorner.X,BottomWalls[i]->BottomRightCorner.Y + BottomWalls[i]->PartitionDimensions.Y, 0));
					if (Scale > 0)
					{
						GenerateWalls(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y,0), FRotator(0,0,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i == BottomWalls.Num() - 1)
				{
					Scale = FVector::Distance(FVector(BottomWalls[i]->BottomRightCorner.X + BottomWalls[i]->PartitionDimensions.X,BottomWalls[i]->BottomRightCorner.Y + BottomWalls[i]->PartitionDimensions.Y, 0), FVector(Room->BottomRightCorner.X + Room->PartitionDimensions.X, Room->BottomRightCorner.Y,0));
				
					if (Scale > 0)
					{
						GenerateWalls(FVector(BottomWalls[i]->BottomRightCorner.X + BottomWalls[i]->PartitionDimensions.X, BottomWalls[i]->BottomRightCorner.Y + BottomWalls[i]->PartitionDimensions.Y, 0), FRotator(0,0,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i < BottomWalls.Num()-1)
				{
					Scale = FVector::Distance(FVector(BottomWalls[i]->BottomRightCorner.X + BottomWalls[i]->PartitionDimensions.X,BottomWalls[i]->BottomRightCorner.Y + BottomWalls[i]->PartitionDimensions.Y , 0), FVector(BottomWalls[i+1]->BottomRightCorner.X,BottomWalls[i+1]->BottomRightCorner.Y+BottomWalls[i+1]->PartitionDimensions.Y,0));
					GenerateWalls(FVector(BottomWalls[i]->BottomRightCorner.X + BottomWalls[i]->PartitionDimensions.X, BottomWalls[i]->BottomRightCorner.Y + BottomWalls[i]->PartitionDimensions.Y, 0), FRotator(0,0,0), FVector(Scale/WallMeshSize, 1, 1));
				}
				
			}
		}
		else
		{
			GenerateWalls(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y,0), FRotator(0,0,0), FVector(Room->PartitionDimensions.X/WallMeshSize, 1, 1));
		}
}

void ABSP::GenerateTopWalls(FNode* Room)
{
	TArray<FNode*> TopWalls;

	for (auto CorridorArray : Room->CorridorConnections)
	{
		for(auto Corridor : CorridorArray){
			//TopWalls
			if ((Corridor->BottomRightCorner.Y) == Room->BottomRightCorner.Y + Room->PartitionDimensions.Y )
			{
				if (Room->PartitionDimensions.X < Corridor->PartitionDimensions.X)
				{
					return;
				}
				if ((Corridor->BottomRightCorner.X >= Room->BottomRightCorner.X && Corridor->BottomRightCorner.X < Room->BottomRightCorner.X + Room->PartitionDimensions.X))
				{
					TopWalls.Add(Corridor);
				}
			}
		}
	}

		//TopWalls
		Algo::Sort(TopWalls, [](const FNode* A, const FNode* B)
			{
		return A->BottomRightCorner.X < B->BottomRightCorner.X;
			});

		//TOP WALLS//
		if (TopWalls.Num() > 0)
		{
			float Scale=0;
			for (int i = 0 ; i < TopWalls.Num(); i++)
			{
				if (i == 0)
				{
					Scale = FVector::Distance(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y + Room->PartitionDimensions.Y,0), FVector(TopWalls[i]->BottomRightCorner.X, TopWalls[i]->BottomRightCorner.Y, 0));
					
					if (Scale > 0)
					{
						GenerateWalls(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y + Room->PartitionDimensions.Y,0), FRotator(0,0,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i == TopWalls.Num() - 1)
				{
					Scale = FVector::Distance(FVector(TopWalls[i]->BottomRightCorner.X + TopWalls[i]->PartitionDimensions.X,TopWalls[i]->BottomRightCorner.Y, 0), FVector(Room->BottomRightCorner.X + Room->PartitionDimensions.X, Room->BottomRightCorner.Y + Room->PartitionDimensions.Y,0));
					
					if (Scale > 0)
					{
						GenerateWalls(FVector(TopWalls[i]->BottomRightCorner.X + TopWalls[i]->PartitionDimensions.X, TopWalls[i]->BottomRightCorner.Y , 0), FRotator(0,0,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i < TopWalls.Num()-1)
				{
					Scale = FVector::Distance(FVector(TopWalls[i]->BottomRightCorner.X + TopWalls[i]->PartitionDimensions.X, TopWalls[i]->BottomRightCorner.Y,0), FVector(TopWalls[i+1]->BottomRightCorner.X, TopWalls[i+1]->BottomRightCorner.Y,0));
					GenerateWalls(FVector(TopWalls[i]->BottomRightCorner.X + TopWalls[i]->PartitionDimensions.X, TopWalls[i]->BottomRightCorner.Y, 0), FRotator(0,0,0), FVector(Scale/WallMeshSize, 1, 1));
				}
			}
		}
		else
		{
		
			GenerateWalls(FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y + Room->PartitionDimensions.Y,0), FRotator(0,0,0), FVector(Room->PartitionDimensions.X/WallMeshSize, 1, 1));
		}
	
}
void ABSP::GenerateLeftWalls(FNode* Room)
{
	TArray<FNode*> LeftWalls;

	for (auto CorridorArray : Room->CorridorConnections)
	{
		for(auto Corridor : CorridorArray){
			//LeftWalls
			if (Corridor->BottomRightCorner.X == Room->BottomRightCorner.X + Room->PartitionDimensions.X){
				if (Room->PartitionDimensions.Y < Corridor->PartitionDimensions.Y)
				{
					return;
				}
				if ( (Corridor->BottomRightCorner.Y >= Room->BottomRightCorner.Y &&  Corridor->BottomRightCorner.Y < Room->BottomRightCorner.Y + Room->PartitionDimensions.Y))
				{
					LeftWalls.Add(Corridor);
				}
			}
		}
	}

		//LeftWalls
		Algo::Sort(LeftWalls, [](const FNode* A, const FNode* B)
			{
		return A->BottomRightCorner.Y < B->BottomRightCorner.Y;
			});

		//LEFT WALLS//
		if (LeftWalls.Num() > 0)
		{
			float Scale=0;
			for (int i = 0 ; i < LeftWalls.Num(); i++)
			{
				if (i == 0)
				{
					Scale = FVector::Distance(FVector(Room->BottomRightCorner.X + Room->PartitionDimensions.X, Room->BottomRightCorner.Y,0), FVector(LeftWalls[i]->BottomRightCorner.X,LeftWalls[i]->BottomRightCorner.Y, 0));
					if (Scale > 0)
					{
						GenerateWalls(FVector(Room->BottomRightCorner.X + Room->PartitionDimensions.X, Room->BottomRightCorner.Y,0), FRotator(0,90,0), FVector(Scale/WallMeshSize, 1, 1));
					}				}
				
				if (i == LeftWalls.Num() - 1)
				{
					Scale = FVector::Distance(FVector(LeftWalls[i]->BottomRightCorner.X ,LeftWalls[i]->BottomRightCorner.Y + LeftWalls[i]->PartitionDimensions.Y, 0), FVector(Room->BottomRightCorner.X + Room->PartitionDimensions.X, Room->BottomRightCorner.Y + Room->PartitionDimensions.Y,0));
					if (Scale > 0)
					{
						GenerateWalls(FVector(LeftWalls[i]->BottomRightCorner.X , LeftWalls[i]->BottomRightCorner.Y + LeftWalls[i]->PartitionDimensions.Y, 0), FRotator(0,90,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i < LeftWalls.Num()-1)
				{
					Scale = FVector::Distance(FVector(LeftWalls[i]->BottomRightCorner.X ,LeftWalls[i]->BottomRightCorner.Y + LeftWalls[i]->PartitionDimensions.Y, 0 ), FVector(LeftWalls[i+1]->BottomRightCorner.X,LeftWalls[i+1]->BottomRightCorner.Y, 0));
					GenerateWalls(FVector(LeftWalls[i]->BottomRightCorner.X ,LeftWalls[i]->BottomRightCorner.Y + LeftWalls[i]->PartitionDimensions.Y, 0 ), FRotator(0,90,0), FVector(Scale/WallMeshSize, 1, 1));
				}
			}
		}
		else
		{
			GenerateWalls(FVector(Room->BottomRightCorner.X +Room->PartitionDimensions.X, Room->BottomRightCorner.Y,0), FRotator(0,90,0), FVector(Room->PartitionDimensions.Y/WallMeshSize, 1, 1));
		}
}
void ABSP::GenerateRightWall(FNode* Room)
{
	TArray<FNode*> RightWalls;
	for (auto CorridorArray : Room->CorridorConnections)
	{
		for(auto Corridor : CorridorArray){
			//RightWalls
			if ( Corridor->BottomRightCorner.X + Corridor->PartitionDimensions.X == Room->BottomRightCorner.X)
			{
				if (Room->PartitionDimensions.Y < Corridor->PartitionDimensions.Y)
				{
					return;
				}
				if ((Corridor->BottomRightCorner.Y >= Room->BottomRightCorner.Y &&  Corridor->BottomRightCorner.Y < Room->BottomRightCorner.Y + Room->PartitionDimensions.Y))
				{
					RightWalls.Add(Corridor);
				}
			}
	    }
	}

		
		//RightWalls
		Algo::Sort(RightWalls, [](const FNode* A, const FNode* B){
			return A->BottomRightCorner.Y < B->BottomRightCorner.Y;
		});

		//RIGHT WALLS//
		if (RightWalls.Num() > 0)
		{
			float Scale=0;
			for (int i = 0 ; i < RightWalls.Num(); i++)
			{
				if (i == 0)
				{
					Scale = FVector::Distance(FVector(Room->BottomRightCorner.X , Room->BottomRightCorner.Y,0), FVector(RightWalls[i]->BottomRightCorner.X + RightWalls[i]->PartitionDimensions.X ,RightWalls[i]->BottomRightCorner.Y, 0));
					if (Scale > 0)
					{
						GenerateWalls(FVector(Room->BottomRightCorner.X , Room->BottomRightCorner.Y,0), FRotator(0,90,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i == RightWalls.Num() - 1)
				{
					Scale = FVector::Distance(FVector(RightWalls[i]->BottomRightCorner.X + RightWalls[i]->PartitionDimensions.X,RightWalls[i]->BottomRightCorner.Y + RightWalls[i]->PartitionDimensions.Y, 0), FVector(Room->BottomRightCorner.X, Room->BottomRightCorner.Y + Room->PartitionDimensions.Y,0));
				
					if (Scale > 0)
					{
						GenerateWalls(FVector(RightWalls[i]->BottomRightCorner.X + RightWalls[i]->PartitionDimensions.X, RightWalls[i]->BottomRightCorner.Y + RightWalls[i]->PartitionDimensions.Y, 0), FRotator(0,90,0), FVector(Scale/WallMeshSize, 1, 1));
					}
				}
				if (i < RightWalls.Num()-1)
				{
					Scale = FVector::Distance(FVector(RightWalls[i]->BottomRightCorner.X + RightWalls[i]->PartitionDimensions.X,RightWalls[i]->BottomRightCorner.Y + RightWalls[i]->PartitionDimensions.Y,0), FVector(RightWalls[i+1]->BottomRightCorner.X + RightWalls[i+1]->PartitionDimensions.X,RightWalls[i+1]->BottomRightCorner.Y,0));
					GenerateWalls(FVector(RightWalls[i]->BottomRightCorner.X + RightWalls[i]->PartitionDimensions.X,RightWalls[i]->BottomRightCorner.Y + RightWalls[i]->PartitionDimensions.Y,0), FRotator(0,90,0), FVector(Scale/WallMeshSize, 1, 1));
				}
			}
		}
		else
		{
			GenerateWalls(FVector(Room->BottomRightCorner.X , Room->BottomRightCorner.Y,0), FRotator(0,90,0), FVector(Room->PartitionDimensions.Y/WallMeshSize, 1, 1));
		}
}