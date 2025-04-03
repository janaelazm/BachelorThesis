// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PCG.generated.h"

UCLASS()
class BACHELORTHESIS_API APCG : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APCG();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	/** Init the four corners with random values */
	UFUNCTION(BlueprintCallable)
	void InitHeightMap();

	UFUNCTION(BlueprintCallable)
	void DiamondStep();

	UFUNCTION(BlueprintCallable)
	void SquareStep();

	/** preform diamond square */
	UFUNCTION(BlueprintCallable)
	void GenerateTerrain(int32 Iteration);

	UFUNCTION(BlueprintCallable)
	void GenerateMesh();

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UProceduralMeshComponent> ProceduralMesh;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> DefaultSceneRoot;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (ExposeOnSpawn = "true"))
	int32 GridSize;

	/** to determine the roughness of the terrain*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	double Roughness;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	double GridSpacing;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	double UVGridSpacing;

	/** to determine the roughness of the terrain*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	double Random;


	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	int32 StepSize;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default", meta = (ExposeOnSpawn = "true"))
	TArray<float> HeightMap;

	//PCG MESH DATA
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Settings", meta = (ExposeOnSpawn = "true"))
	TArray<FVector> MeshVertices;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Terrain Settings", meta = (ExposeOnSpawn = "true"))
	TArray<int32> MeshTriangles;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Terrain Settings")
	TArray<FVector> Normals;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Terrain Settings")
	TArray<FVector2D> UVs;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Terrain Settings")
	TArray<FProcMeshTangent> Tangents;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Terrain Settings")
	TArray<FLinearColor> VertexColors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Material")
	UMaterialInterface* TerrainMaterial;
};
