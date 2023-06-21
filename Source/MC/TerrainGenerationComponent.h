// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chunk.h"
#include "TerrainGenerationComponent.generated.h"


class AChunk;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MC_API UTerrainGenerationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTerrainGenerationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	//提供蓝图选择chunk类的机会
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AChunk> ChunkClass;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//更新玩家所在区块的世界索引，返回世界索引的dx,dy。
	FIntPoint UpdatePlayerChunkIndex();

	//根据世界坐标添加Chunk
	void AddChunk(FIntPoint ChunkIndexInWorld, bool bHasCollision);
	
	//根据当前玩家位置和渲染距离生成所有Chunk，会给玩家周围的chunk添加碰撞。
	void GenerateChunks();

	/*通过原先的正方形中心索引、新的正方形中心的偏移量、中心到边的距离得到增加的长方形区域
	（有的可能是无效的，只有能表示矩形范围的才有效），区域使用FBox2D表示，FBox2D的Max用以记录边界值，
	只作为范围判断。如Min:(-1,-1)，Max:(2,0) 表示(-1,-1),(0,-1),(1,-1) 这三个chunk。*/
	TArray<FBox2D> GetExtentBox(FIntPoint PreBoxCenterIndex, FIntPoint DxDy, int32 Range);

	//根据玩家位置偏移和渲染距离生成新的Chunk，并删除距离过远的Chunk，且更新碰撞。
	void UpdateChunks(FIntPoint DxDy);	

	//摧毁命中点所在的方块
	UFUNCTION(BlueprintCallable)
	void DestroyBlock(FVector ImpactPoint, FVector ImpactNormal);

	//放置方块
	UFUNCTION(BlueprintCallable)
	void PlaceBlock(FVector ImpactPoint, FVector ImpactNormal,EBlockType Type);

	int32 GetMaterialCount(){ return Materials.Num();}

	UMaterialInterface* GetMaterialByType(EBlockType Type){ return Materials.Contains(Type) ? Materials[Type] : nullptr;}

private:
	//通过世界坐标获取chunk
	AChunk* GetChunkByLocation(FVector ChunkLocation);

	/*当chunk中边缘的方块被更改时，需要通知邻近chunk更新该方块被更改，使用此函数更新所有会被影响的chunk中的边缘方块信息。
	  参数为被更改的方块的位置、改为了什么类型。*/
	void UpdateEdgeBlocks(FVector changedBlockLocation, EBlockType type);

	UPROPERTY(VisibleAnywhere)
	//存储世界中所有的chunk
	//TArray<AChunk*> Chunks;
	TMap<FIntPoint,AChunk*> Chunks;

	//block数据表
	UPROPERTY(EditAnywhere)
	UDataTable* BlockDataTable;

	//存储从数据表中加载的方块材质
	UPROPERTY(EditAnywhere)
	TMap<EBlockType,UMaterialInterface*> Materials;
	
	//玩家当前所在的区块，以区块为单位
	UPROPERTY(VisibleInstanceOnly)
	FIntPoint PlayerChunkIndex;

	//方块大小
	UPROPERTY(EditAnywhere)
	int32 BlockSize = 100;

	//chunk尺寸，表示长宽高有几个block
	UPROPERTY(EditAnywhere)
	int32 ChunkXBlocks = 16;
	UPROPERTY(EditAnywhere)
	int32 ChunkYBlocks = 16;
	UPROPERTY(EditAnywhere)
	int32 ChunkZBlocks = 256;
	

	
	//渲染范围，以区块为单位
	UPROPERTY(EditAnywhere)
	int32 RenderRange = 20;	

	//开启碰撞的区块范围，以区块为单位
	UPROPERTY(EditAnywhere)
	int32 CollisionRange = 1;
};
