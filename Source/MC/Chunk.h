// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.generated.h"

class UProceduralMeshComponent;

/*TODO:使用数据表，将材质与类型关联，且能在蓝图上更改*/
UENUM()
enum class EBlockType:uint8
{
	Air UMETA(DisplayName = "Air"),
	Grass UMETA(DisplayName = "Grass"),
	Soil UMETA(DisplayName = "Soil"),
	Stone UMETA(DisplayName = "Stone"),
};

UCLASS()
class MC_API AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunk();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* proceduralMeshComponent;

	//方块大小
	UPROPERTY(EditAnywhere)
	int32 blockSize = 100;
	//chunk尺寸，表示长宽高有几个block
	UPROPERTY(EditAnywhere)
	int32 chunkXBlocks = 16;
	UPROPERTY(EditAnywhere)
	int32 chunkYBlocks = 16;
	UPROPERTY(EditAnywhere)
	int32 chunkZBlocks = 256;

	//此chunk在world中的XY坐标，以chunk为单位，是为整数索引，且可以为负。
	FIntPoint chunkIndexInWorld;
	
private:
	//辅助函数，通过i,j,k获得blocks数组中的索引。
	int getIndexInBlocksArray(int x, int y, int z){ return z + y * chunkZBlocks + x * chunkZBlocks * chunkYBlocks; }
	
	TArray<int32> calculateNoise();
	
	//生成Chunk，为每一方块确定类型。
	void GenerateChunk();
	//为每一方块生成渲染数据，提供给程序化网格体组件。
	void UpdateMesh();

	//存储此chunk中每个方块的类型
	TArray<EBlockType> blocks;
		
	int32 kindsOfBlocks = 4;
};
