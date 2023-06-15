// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerationComponent.h"

#include "Chunk.h"
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
	UpdateActorPosition();
	AddChunks();
	// ...
}


// Called every frame
void UTerrainGenerationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (UpdateActorPosition())
	{
		AddChunks();
	}
	// ...
}

bool UTerrainGenerationComponent::UpdateActorPosition()
{
	//通过玩家控制器获取玩家的位置
	actorLocation = UGameplayStatics::GetPlayerPawn(GetOwner(), 0)->GetActorLocation();
	//转化为以chunk为单位的位置
	const FIntPoint newPlayerChunkPosition = FIntPoint(
		actorLocation.X / (blockSize * chunkXBlocks),
		actorLocation.Y / (blockSize * chunkYBlocks)
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
			if(chunks.Contains(chunkIndexInWorld))
			{
				;
			}
			else
			{
				FTransform spawnTransform = FTransform(chunkLocationInWorld);
				AChunk* newChunk = GetWorld()->SpawnActorDeferred<AChunk>(chunkClass,spawnTransform);
				newChunk->blockSize = blockSize;
				newChunk->chunkXBlocks = chunkXBlocks;
				newChunk->chunkYBlocks = chunkYBlocks;
				newChunk->chunkZBlocks = chunkZBlocks;
				newChunk->chunkIndexInWorld = chunkIndexInWorld;
				newChunk->FinishSpawning(spawnTransform);

				chunks.Add(chunkIndexInWorld, newChunk);
			}
		}
	}
}
