// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AChunk> chunkClass;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool UpdateActorPosition();
	void AddChunks();

	//点击鼠标左键时，检测是否击中方块，进行相应操作。
	UFUNCTION(BlueprintCallable)
	void DestoryBlock(AChunk* chunkBeHit, FVector ImpactPoint);
private:
	//存储世界中所有的chunk
	//TArray<AChunk*> Chunks;
	TMap<FIntPoint,AChunk*> chunks;

	//玩家当前的世界位置
	FVector actorLocation;

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
	
	//玩家当前所在的区块，以区块为单位
	UPROPERTY(VisibleInstanceOnly)
	FIntPoint playerChunkPosition;
	
	//渲染范围，以区块为单位
	UPROPERTY(EditAnywhere)
	int32 renderRange = 20;	
};
