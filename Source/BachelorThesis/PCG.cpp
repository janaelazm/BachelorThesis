//#include "C:\Users\janae\Documents\Unreal Projects\BachelorThesis\Intermediate\Build\Win64\x64\BachelorThesisEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "PCG.h"
#include "Math/UnrealMathUtility.h"
#include "Algo/MinElement.h"
#include "Algo/MaxElement.h"

// Sets default values
APCG::APCG()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the ProceduralMesh component
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));

	// Attach it to the root component
	RootComponent = ProceduralMesh;

	APCG::GridSize = 9;// Must be 2^n + 1 for Diamond-Square
	APCG::Roughness = 1;
	APCG::StepSize = GridSize - 1;
	APCG::Random = 1.0f;
	APCG::GridSpacing = 1;
	APCG::UVGridSpacing = 1;

}

void APCG::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

// Called when the game starts or when spawned
void APCG::BeginPlay()
{
	Super::BeginPlay();
	InitHeightMap();
	GenerateTerrain(1);
	GenerateMesh();

}

// Called every frame
void APCG::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APCG::InitHeightMap()
{

	// Resize the HeightMap array to fit GridSize x GridSize
	APCG::HeightMap.SetNum(GridSize * GridSize);

	// Initialize corner points
	int32 LastIndex = GridSize - 1;

	// Bottom-left
	HeightMap[0] = FMath::FRandRange(-Random, Random);

	// Bottom-right
	HeightMap[LastIndex] = -FMath::FRandRange(-Random, Random);

	// Top-left
	HeightMap[LastIndex * GridSize] = -FMath::FRandRange(-Random, Random);;

	// Top-right
	HeightMap[LastIndex * GridSize + LastIndex] =FMath::FRandRange(-Random, Random);;
}

void APCG::GenerateTerrain(int32 Iteration)
{

	while (APCG::StepSize > 1)
	{
		APCG::DiamondStep();
		APCG::SquareStep();
		APCG::StepSize = APCG::StepSize / 2;
		Random = Random /2;
	}
}

void APCG::DiamondStep()
{
	int32 HalfStepSize = APCG::StepSize / 2;

	for (int x = 0; x < GridSize - 1; x += StepSize)
	{
		for (int y = 0; y < GridSize - 1; y += StepSize)
		{
			// Calculate the average of the four corner points
			float avg = (HeightMap[x + y * GridSize] +
						 HeightMap[(x + StepSize) + y * GridSize] +
						 HeightMap[x + (y + StepSize) * GridSize] +
						 HeightMap[(x + StepSize) + (y + StepSize) * GridSize]) / 4.0f;

			// Add randomness
			HeightMap[(x + HalfStepSize) + (y + HalfStepSize) * GridSize] = avg +
				FMath::FRandRange(-Random, Random);
		}
	}
}

void APCG::SquareStep()
{
	int32 HalfStepSize = APCG::StepSize / 2;
	int32 Count;

	float MidPointLeft;
	float MidPointRight;
	float MidPointTop;
	float MidPointBottom;

	for (int x = 0; x < GridSize; x += HalfStepSize)
	{
		for (int y = (x + HalfStepSize)%StepSize ; y < GridSize; y += StepSize)
		{
			Count = 0;

			//Left
			if (x - HalfStepSize < 0)
			{
				MidPointLeft = 0;
			}
			else
			{
				MidPointLeft = HeightMap[(x-HalfStepSize) + y * GridSize];
				Count++;
			}

			//Top
			if (y + HalfStepSize >= GridSize)
			{
				MidPointTop = 0;
			}
			else
			{
				MidPointTop = HeightMap[x + (y+HalfStepSize) * GridSize];
				Count++;
			}

			//Right
			if (x + HalfStepSize >= GridSize)
			{
				MidPointRight = 0;
			}
			else
			{
				MidPointRight = HeightMap[(x+HalfStepSize) + y * GridSize];
				Count++;
			}
			
			//Bottom
			if (y-HalfStepSize < 0)
			{
				MidPointBottom = 0;
			}
			else
			{
				MidPointBottom = HeightMap[x + (y-HalfStepSize) * GridSize];
				Count++;
			}
			HeightMap[x + y * GridSize] = (MidPointBottom+MidPointLeft+MidPointRight+MidPointTop / Count)+FMath::RandRange(-Random,Random);
		}
	}
	
}

void APCG::GenerateMesh()
{
	for (int x = 0; x < GridSize; x++) {
		for (int y = 0; y < GridSize; y++) {
			FVector Vertex = FVector(x*GridSpacing,y*GridSpacing,HeightMap[x+y*GridSize]);
			MeshVertices.Add(Vertex);
			// Add UV coordinates
			UVs.Add(FVector2D((x*UVGridSpacing)/GridSize, (y*UVGridSpacing)/ (GridSize)));
			//DrawDebugSphere(GetWorld(),Vertex, 20.0f,12, FColor::Green,true,-1.0f,0,2.0f);
		}
	}
	
	for (int x = 0; x < GridSize-1; x++)
	{
		for (int y = 0; y < GridSize-1; y++)
		{
			int BottomLeft = (y * GridSize) + x;        // Bottom-left vertex index
			int BottomRight = (y * GridSize) + (x + 1);  // Bottom-right vertex index
			int TopLeft = ((y + 1) * GridSize) + x;     // Top-left vertex index
			int TopRight = ((y + 1) * GridSize) + (x + 1); // Top-right vertex index

			// First triangle
			MeshTriangles.Add(BottomLeft);
			MeshTriangles.Add(BottomRight);
			MeshTriangles.Add(TopRight);

			// Second triangle
			MeshTriangles.Add(BottomLeft);
			MeshTriangles.Add(TopRight);
			MeshTriangles.Add(TopLeft);
		}
	}

	// Calculate normals (simple for now)
	Normals.SetNum(MeshVertices.Num());
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Normals[i] = FVector(0, 0, 1); // Upward normals
	}

	// Assign colors (optional)
	VertexColors.SetNum(MeshVertices.Num());
	float MinHeight = *Algo::MinElement(HeightMap); // Minimum height
	float MaxHeight = *Algo::MaxElement(HeightMap); // Maximum height
	float HeightRange = MaxHeight - MinHeight;

	TArray<FLinearColor> RainbowColors = {
		FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),  // Red
		FLinearColor(1.0f, 0.5f, 0.0f, 1.0f),  // Orange
		FLinearColor(1.0f, 1.0f, 0.0f, 1.0f),  // Yellow
		FLinearColor(0.0f, 1.0f, 0.0f, 1.0f),  // Green
		FLinearColor(0.0f, 0.0f, 1.0f, 1.0f),  // Blue
		FLinearColor(0.5f, 0.0f, 1.0f, 1.0f),  // Indigo
		FLinearColor(0.5f, 0.0f, 0.5f, 1.0f)   // Violet
	};

	for (int i = 0; i < MeshVertices.Num(); i++)
	{
		float Height = MeshVertices[i].Z;

		// Normalize height between 0 and 1
		float NormalizedHeight = (Height - MinHeight) / HeightRange;

		// Determine the color index to blend between
		int ColorIndex = FMath::Clamp(FMath::FloorToInt(NormalizedHeight * (RainbowColors.Num() - 1)), 0, RainbowColors.Num() - 2);
    
		// Calculate interpolation factor
		float LerpFactor = (NormalizedHeight * (RainbowColors.Num() - 1)) - ColorIndex;

		// Blend between colors
		FLinearColor VertexColor = FMath::Lerp(RainbowColors[ColorIndex], RainbowColors[ColorIndex + 1], LerpFactor);

		// Add to vertex colors
		VertexColors[i] = VertexColor;
	}
	
	// Tangents (optional, not used in this example)
	Tangents.SetNum(MeshVertices.Num());
	for (int32 i = 0; i < Tangents.Num(); i++) 
	{
		Tangents[i] = FProcMeshTangent(0.0f, 1.0f, 0.0f); // Default tangent
	}
	
	// Create the mesh
	ProceduralMesh->CreateMeshSection_LinearColor(0, MeshVertices, MeshTriangles, Normals, UVs, VertexColors, Tangents, true);
	ProceduralMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	ProceduralMesh->SetMaterial(0, TerrainMaterial);

	if (HeightMap.Num() != GridSize * GridSize)
	{
		UE_LOG(LogTemp, Error, TEXT("Heightmap size doesn't match the grid size!"));
		return;
	}

	for (int32 y = 0; y < GridSize; ++y)
	{
		FString RowValues;
		for (int32 x = 0; x < GridSize; ++x)
		{
			int32 Index = x + y * GridSize;
			RowValues += FString::Printf(TEXT("%f "), HeightMap[Index]);
		}
		UE_LOG(LogTemp, Log, TEXT("Row %d: %s"), y, *RowValues);
	}
}
