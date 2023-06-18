// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerationComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UTerrainGenerationComponent::UTerrainGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	
	// ...
}


// Called when the game starts
void UTerrainGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	//从数据表加载方块材质
	if (BlockDataTable)
	{
		TArray<FName> BlockDataRowNames = BlockDataTable->GetRowNames();
		for (FName RowName : BlockDataRowNames)
		{
			FBlockData* BlockData = BlockDataTable->FindRow<FBlockData>(RowName, FString());
			if (BlockData)
			{
				Materials.Add(BlockData->Type, BlockData->AssetData.Material);
			}
		}
	}

	UpdatePlayerChunkIndex();
	AddChunks();
	// ...
}


// Called every frame
void UTerrainGenerationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (UpdatePlayerChunkIndex())
	{
		AddChunks();
	}
	// ...
}

bool UTerrainGenerationComponent::UpdatePlayerChunkIndex()
{
	//通过玩家控制器获取玩家的位置
	FVector actorLocation = UGameplayStatics::GetPlayerPawn(GetOwner(), 0)->GetActorLocation();
	//转化为以chunk为单位的位置
	const FIntPoint newPlayerChunkPosition = FIntPoint(
		FMath::FloorToInt(actorLocation.X / (blockSize * chunkXBlocks)),
		FMath::FloorToInt(actorLocation.Y / (blockSize * chunkYBlocks))
	);

	bool changed = (newPlayerChunkPosition.X!= playerChunkPosition.X || newPlayerChunkPosition.Y!= playerChunkPosition.Y );
	playerChunkPosition = newPlayerChunkPosition;
	return changed;
}

void UTerrainGenerationComponent::AddChunks()
{
	if (chunkClass == nullptr)
		return;
	const int32 chunkXSize = blockSize * chunkXBlocks;
	const int32 chunkYSize = blockSize * chunkYBlocks;
	for (int32 x = -renderRange; x <= renderRange; x++)
	{
		for (int32 y = -renderRange; y <= renderRange; y++)
		{
			const FIntPoint chunkIndexInWorld = playerChunkPosition + FIntPoint{x, y};
			const FVector chunkLocationInWorld = FVector(chunkIndexInWorld.X * chunkXSize, chunkIndexInWorld.Y * chunkYSize, 0.0f);
			//player所在chunk周围一圈的chunk创建碰撞。
			bool hasCollision = ((playerChunkPosition - chunkIndexInWorld).Size() <= 1.415);
			if(Chunks.Contains(chunkIndexInWorld))
			{
				//若chunk已存在，则根据hasCollision更新是否创建碰撞。
				Chunks[chunkIndexInWorld]->UpdateCollision(hasCollision);
			}
			else
			{
				FTransform spawnTransform = FTransform(chunkLocationInWorld);
				AChunk* newChunk = GetWorld()->SpawnActorDeferred<AChunk>(chunkClass,spawnTransform);
				newChunk->BlockSize = blockSize;
				newChunk->ChunkXBlocks = chunkXBlocks;
				newChunk->ChunkYBlocks = chunkYBlocks;
				newChunk->ChunkZBlocks = chunkZBlocks;
				newChunk->ChunkIndexInWorld = chunkIndexInWorld;
				newChunk->ChunkLocationInWorld = chunkLocationInWorld;
				newChunk->bHasCollision = hasCollision;
				newChunk->FinishSpawning(spawnTransform);

				Chunks.Add(chunkIndexInWorld, newChunk);
			}
		}
	}
}


AChunk* UTerrainGenerationComponent::GetChunkByLocation(FVector chunkLocation)
{
	FIntPoint index = FIntPoint(FMath::FloorToInt(chunkLocation.X / chunkXBlocks / blockSize), FMath::FloorToInt(chunkLocation.Y / chunkYBlocks / blockSize));
	if (Chunks.Contains(index))
		return Chunks[index];
	else
		return nullptr;
}

void UTerrainGenerationComponent::UpdateEdgeBlocks(FVector changedBlockLocation, EBlockType type)
{
	TArray<FIntPoint> toUpdateChunkIndexOffset;
	AChunk* changedChunk = GetChunkByLocation(changedBlockLocation);
	
	FIntPoint changedChunkIndexInWorld = changedChunk->ChunkIndexInWorld;
	FIntVector changedBlockIndexInChunk = changedChunk->GetBlockIndexInChunk(changedBlockLocation);
	
	//更改的block是在chunk左边缘或右边缘
	if(changedBlockIndexInChunk.X == 1)
	{
		toUpdateChunkIndexOffset.Add(FIntPoint(-1,0));
	}
	else if(changedBlockIndexInChunk.X == chunkXBlocks)
	{
		toUpdateChunkIndexOffset.Add(FIntPoint(1,0));
	}

	//更改的block是在chunk上边缘或下边缘
	if(changedBlockIndexInChunk.Y == 1)
	{
		toUpdateChunkIndexOffset.Add(FIntPoint(0,-1));
	}
	else if(changedBlockIndexInChunk.Y == chunkYBlocks)
	{
		toUpdateChunkIndexOffset.Add(FIntPoint(0,1));
	}

	//顶角的chunk被影响到
	if(toUpdateChunkIndexOffset.Num() == 2)
	{
		toUpdateChunkIndexOffset.Add(toUpdateChunkIndexOffset[0] + toUpdateChunkIndexOffset[1]);
	}

	for(int i=0;i<toUpdateChunkIndexOffset.Num();i++)
	{
		//被影响到的chunk的世界索引 = 当前chunk的世界索引 + 偏移量
		FIntPoint toUpdateEdgeChunkIndex = changedChunkIndexInWorld + toUpdateChunkIndexOffset[i];
		if(Chunks.Contains(toUpdateEdgeChunkIndex))
		{
			Chunks[toUpdateEdgeChunkIndex]->SetBlock(changedBlockLocation, EBlockType::Air);
		}	
	}
}

void UTerrainGenerationComponent::DestroyBlock(FVector impactPoint, FVector impactNormal)
{
	//通过面上的击中点减去击中点的法向量得到block内部的点。
	const FVector pointInBlock = impactPoint - impactNormal * 0.5f * blockSize;
	AChunk* chunkBeHit = GetChunkByLocation(pointInBlock);
	chunkBeHit->SetBlock(pointInBlock, EBlockType::Air);
	
	//更新邻近方块
	UpdateEdgeBlocks(pointInBlock,EBlockType::Air);
}