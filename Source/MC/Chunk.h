// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "BlockDataStructs.h"
#include "Chunk.generated.h"

class UTerrainGenerationComponent;
class UProceduralMeshComponent;

//Mesh数据准备完毕时调用
DECLARE_DELEGATE( FMeshDataReadyDelegate );

/*TODO:使用数据表，将材质与类型关联，且能在蓝图上更改*/

struct FMeshData
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
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
	
	void UpdateCollision(bool Collision);

	//传入世界坐标，求该坐标在此chunk的索引，包含外围一圈。
	FIntVector GetBlockIndexInChunk(const FVector WorldPosition);

	//传入世界坐标，求该坐标所在Block的中心坐标。
	UFUNCTION(BlueprintCallable)
	FVector GetBlockCenterPosition(const FVector WorldPosition);
	
	/*传入世界坐标，更改该坐标处方块的类型。应保证坐标在block内,而不是在面上。
	  block可以是外围一圈的block。返回是否更改成功，
	*/
	bool SetBlock(FVector Position, EBlockType Type);

	//获取该世界坐标下方块的类型。
	UFUNCTION(BlueprintCallable)
	EBlockType GetBlockType(FVector Position);

	UFUNCTION()
	void AsyncSaveChunk();
	UFUNCTION()
	void SaveChunk();
	//TODO: 实例化网格体快还是程序化网格体快？
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProceduralMeshComponent;

	//方块大小
	UPROPERTY(VisibleAnywhere)
	int32 BlockSize = 100;

	//chunk尺寸，表示长宽高有几个block
	UPROPERTY(VisibleAnywhere)
	int32 ChunkXBlocks = 16;
	UPROPERTY(VisibleAnywhere)
	int32 ChunkYBlocks = 16;
	UPROPERTY(VisibleAnywhere)
	int32 ChunkZBlocks = 256;

	//此chunk在world中的XY坐标，以chunk为单位，是为整数索引，且可以为负。
	UPROPERTY(VisibleAnywhere)
	FIntPoint ChunkIndexInWorld;

	//此chunk在world中的世界坐标
	UPROPERTY(VisibleAnywhere)
	FVector ChunkLocationInWorld;

	//当前chunk中的blocks是否有碰撞，只在player所在的chunk和邻近的chunk生成碰撞，可以优化性能。
	UPROPERTY(VisibleAnywhere)
	bool bHasCollision = false;
	
private:
	//辅助函数，通过x,y,z获得blocks数组中的索引。遍历循环x、y、z，z为最内层。
	inline int GetIndexInBlocksArray(int x, int y, int z){ return z + y * (ChunkZBlocks) + x * (ChunkYBlocks + 2) * ChunkZBlocks; }
	
	TArray<int32> CalculateNoise();

	TMap<int32, EBlockType>  GetChangedBlocksBySaveFiles();	

	//生成Chunk，为每一方块确定类型。
	void GenerateChunk();

	//计算Mesh数据
	void PrepareMeshData();
	void AsyncPrepareMeshData();

	//将Mesh数据应用到程序化网格体上
	void ApplyMeshData();

	//更新Mesh数据，提交给程序化网格体组件
	void UpdateMesh();
	//使用多线程异步更新
	void AsyncUpdateMesh();

public:
	UPROPERTY()
	//存储此chunk中每个方块的类型
	TArray<EBlockType> Blocks;
	//存储此chunk中被更改过的方块。
	TMap<int32, EBlockType> ChangedBlocks;

	//Mesh数据准备好了
	FMeshDataReadyDelegate MeshDataReady;

private:
	TWeakObjectPtr<UTerrainGenerationComponent> TerrainGenerationComponent;

	//Mesh数据
	TMap<EBlockType, FMeshData> MeshDataMap;
};
