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
	TSubclassOf<AChunk> chunkClass;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//更新玩家所在区块的世界索引，如果更新了，返回true。
	bool UpdatePlayerChunkIndex();

	//根据当前玩家位置和渲染距离添加更新chunk，会给玩家周围的chunk添加碰撞。
	void AddChunks();
	
	
	//摧毁命中点所在的方块
	UFUNCTION(BlueprintCallable)
	void DestroyBlock(FVector impactPoint, FVector impactNormal);

private:
	//通过世界坐标获取chunk
	AChunk* GetChunkByLocation(FVector chunkLocation);

	/*当chunk中边缘的方块被更改时，需要通知邻近chunk更新该方块被更改，使用此函数更新所有会被影响的chunk中的边缘方块信息。
	  参数为被更改的方块的位置、改为了什么类型。*/
	void UpdateEdgeBlocks(FVector changedBlockLocation, EBlockType type);

	//存储世界中所有的chunk
	//TArray<AChunk*> Chunks;
	TMap<FIntPoint,AChunk*> chunks;

	//玩家当前所在的区块，以区块为单位
	UPROPERTY(VisibleInstanceOnly)
	FIntPoint playerChunkPosition;

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
	

	
	//渲染范围，以区块为单位
	UPROPERTY(EditAnywhere)
	int32 renderRange = 20;	
};
