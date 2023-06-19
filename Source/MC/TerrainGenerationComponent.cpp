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

	//TODO: 有更好的加载数据时机？
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
	GenerateChunks();
	// ...
}


// Called every frame
void UTerrainGenerationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FIntPoint PlayChunkIndexDxDy = UpdatePlayerChunkIndex();
	if (PlayChunkIndexDxDy != FIntPoint{0,0})
	{
		UpdateChunks(PlayChunkIndexDxDy);
	}
	// ...
}

FIntPoint UTerrainGenerationComponent::UpdatePlayerChunkIndex()
{
	//通过玩家控制器获取玩家的位置
	FVector ActorLocation = UGameplayStatics::GetPlayerPawn(GetOwner(), 0)->GetActorLocation();
	//转化为以chunk为单位的位置
	const FIntPoint NewPlayerChunkPosition = FIntPoint(
		FMath::FloorToInt(ActorLocation.X / (BlockSize * ChunkXBlocks)),
		FMath::FloorToInt(ActorLocation.Y / (BlockSize * ChunkYBlocks))
	);
	FIntPoint DxDy = NewPlayerChunkPosition - PlayerChunkIndex;
	PlayerChunkIndex = NewPlayerChunkPosition;
	return DxDy;
}

void UTerrainGenerationComponent::AddChunk(FIntPoint ChunkIndexInWorld, bool bHasCollision)
{
	const int32 chunkXSize = BlockSize * ChunkXBlocks;
	const int32 chunkYSize = BlockSize * ChunkYBlocks;
	const FVector ChunkLocationInWorld = FVector(ChunkIndexInWorld.X * chunkXSize, ChunkIndexInWorld.Y * chunkYSize, 0.0f);
	FTransform SpawnTransform = FTransform(ChunkLocationInWorld);
	
	AChunk* NewChunk = GetWorld()->SpawnActorDeferred<AChunk>(ChunkClass,SpawnTransform);
	NewChunk->BlockSize = BlockSize;
	NewChunk->ChunkXBlocks = ChunkXBlocks;
	NewChunk->ChunkYBlocks = ChunkYBlocks;
	NewChunk->ChunkZBlocks = ChunkZBlocks;
	NewChunk->ChunkIndexInWorld = ChunkIndexInWorld;
	NewChunk->ChunkLocationInWorld = ChunkLocationInWorld;
	NewChunk->bHasCollision = bHasCollision;
	NewChunk->FinishSpawning(SpawnTransform);

	Chunks.Add(ChunkIndexInWorld, NewChunk);
}

void UTerrainGenerationComponent::GenerateChunks()
{
	if (ChunkClass == nullptr)
		return;
	
	for (int32 x = -RenderRange; x <= RenderRange; x++)
	{
		for (int32 y = -RenderRange; y <= RenderRange; y++)
		{
			const FIntPoint ChunkIndexInWorld = PlayerChunkIndex + FIntPoint{x, y};

			//碰撞范围内的要产生碰撞
			FIntPoint Dxy = PlayerChunkIndex - ChunkIndexInWorld;
			Dxy.X = FMath::Abs(Dxy.X);
			Dxy.Y = FMath::Abs(Dxy.Y);
			bool bHasCollision = Dxy.X <= CollisionRange && Dxy.Y <= CollisionRange;

			if(Chunks.Contains(ChunkIndexInWorld))
			{
				//若chunk已存在，则根据hasCollision更新是否创建碰撞。
				Chunks[ChunkIndexInWorld]->UpdateCollision(bHasCollision);
			}
			else
			{
				AddChunk(ChunkIndexInWorld, bHasCollision);
			}
		}
	}
}

TArray<FBox2D> UTerrainGenerationComponent::GetExtentBox(FIntPoint PreBoxCenterIndex, FIntPoint DxDy, int32 Range)
{
	FIntPoint CurBoxCenterIndex = PreBoxCenterIndex + DxDy;
	//B、D为矩形右上角和左下角的坐标，因为表达的是矩形范围，所以 B的坐标加了1，后面处理时不应取到右边界或者上边界。
	FIntPoint B = CurBoxCenterIndex + FIntPoint(Range + 1, Range + 1);
	FIntPoint D = CurBoxCenterIndex + FIntPoint(-Range, -Range);

	//DxDy过大，移动前后的矩形范围没有了交集，直接返回移动后的矩形范围。
	if (FMath::Abs(DxDy.X) > 2 * Range || FMath::Abs(DxDy.Y) > 2 * Range)
	{
		return {FBox2D(D,B)};
	}

	FBox2D Add1,Add2,Add3;
	if(DxDy.Y < 0)
	{
		Add1[1][1] = Add2[0][1] = Add3[1][1] = D.Y - DxDy.Y;
		Add1[0][1] = Add3[0][1] = D.Y;
		Add2[1][1] = B.Y;
	}
	else
	{
		Add1[0][1] = Add2[1][1] = Add3[0][1] = B.Y - DxDy.Y;
		Add1[1][1] = Add3[1][1] = B.Y;
		Add2[0][1] = D.Y;
	}

	if(DxDy.X < 0)
	{
		Add1[1][0] = Add2[1][0] = Add3[0][0] = D.X -DxDy.X;
		Add1[0][0] = Add2[0][0] = D.X;
		Add3[1][0] = B.X;
	}
	else
	{
		Add1[0][0] = Add2[0][0] = Add3[1][0] = B.X -DxDy.X;
		Add1[1][0] = Add2[1][0] = B.X;
		Add3[0][0] = D.X;	
	}
	return {Add1, Add2, Add3};
}

void UTerrainGenerationComponent::UpdateChunks(FIntPoint DxDy)
{
	TArray<FBox2D> AddAreaIndex = GetExtentBox(PlayerChunkIndex - DxDy, DxDy, RenderRange);
	TArray<FBox2D> RemoveAreaIndex = GetExtentBox(PlayerChunkIndex, FIntPoint(0,0) - DxDy, RenderRange);
	TArray<FBox2D> AddCollisionAreaIndex = GetExtentBox(PlayerChunkIndex - DxDy, DxDy, 1);
	TArray<FBox2D> RemoveCollisionAreaIndex = GetExtentBox(PlayerChunkIndex, FIntPoint(0,0) - DxDy, 1);

	//创建新进入渲染范围的chunk
	for(auto i:AddAreaIndex)
	{
		for(int32 x = i[0][0]; x < i[1][0]; x++)
		{
			for(int32 y = i[0][1]; y < i[1][1]; y++)
			{
				const FIntPoint ChunkIndexInWorld =  FIntPoint{x, y};
				//碰撞范围内的要产生碰撞
				FIntPoint Dxy = PlayerChunkIndex - ChunkIndexInWorld;
				Dxy.X = FMath::Abs(Dxy.X);
				Dxy.Y = FMath::Abs(Dxy.Y);
				bool bHasCollision = Dxy.X <= CollisionRange && Dxy.Y <= CollisionRange;
				if(Chunks.Contains(ChunkIndexInWorld))
				{
					//若chunk已存在，则根据hasCollision更新是否创建碰撞。
					Chunks[ChunkIndexInWorld]->UpdateCollision(bHasCollision);
				}
				else
				{
					AddChunk(ChunkIndexInWorld, bHasCollision);
				}
			}
		}
	}

	//删除渲染范围之外的chunk
	for(auto i:RemoveAreaIndex)
	{
		for(int32 x = i[0][0]; x < i[1][0]; x++)
		{
			for(int32 y = i[0][1]; y < i[1][1]; y++)
			{
				const FIntPoint ChunkIndexInWorld =  FIntPoint{x, y};
				if(Chunks.Contains(ChunkIndexInWorld))
				{
					Chunks[ChunkIndexInWorld]->Destroy();
					Chunks.Remove(ChunkIndexInWorld);
				}
			}
		}
	}

	//为碰撞范围内的chunk添加碰撞
	for(auto i:AddCollisionAreaIndex)
	{
		for(int32 x = i[0][0]; x < i[1][0]; x++)
		{
			for(int32 y = i[0][1]; y < i[1][1]; y++)
			{
				const FIntPoint ChunkIndexInWorld = FIntPoint{x, y};
				if(Chunks.Contains(ChunkIndexInWorld))
				{
					Chunks[ChunkIndexInWorld]->UpdateCollision(true);
				}
			}
		}
	}

	//为碰撞范围外的chunk更新碰撞
	for(auto i:RemoveCollisionAreaIndex)
	{
		for(int32 x = i[0][0]; x < i[1][0]; x++)
		{
			for(int32 y = i[0][1]; y < i[1][1]; y++)
			{
				const FIntPoint ChunkIndexInWorld = FIntPoint{x, y};
				if(Chunks.Contains(ChunkIndexInWorld))
				{
					Chunks[ChunkIndexInWorld]->UpdateCollision(false);
				}
			}
		}
	}	
}

AChunk* UTerrainGenerationComponent::GetChunkByLocation(FVector ChunkLocation)
{
	FIntPoint Index = FIntPoint(FMath::FloorToInt(ChunkLocation.X / ChunkXBlocks / BlockSize), FMath::FloorToInt(ChunkLocation.Y / ChunkYBlocks / BlockSize));
	if (Chunks.Contains(Index))
		return Chunks[Index];
	else
		return nullptr;
}

void UTerrainGenerationComponent::UpdateEdgeBlocks(FVector changedBlockLocation, EBlockType type)
{
	TArray<FIntPoint> ToUpdateChunkIndexOffset;
	AChunk* ChangedChunk = GetChunkByLocation(changedBlockLocation);
	
	FIntPoint ChangedChunkIndexInWorld = ChangedChunk->ChunkIndexInWorld;
	FIntVector ChangedBlockIndexInChunk = ChangedChunk->GetBlockIndexInChunk(changedBlockLocation);
	
	//更改的block是在chunk左边缘或右边缘
	if(ChangedBlockIndexInChunk.X == 1)
	{
		ToUpdateChunkIndexOffset.Add(FIntPoint(-1,0));
	}
	else if(ChangedBlockIndexInChunk.X == ChunkXBlocks)
	{
		ToUpdateChunkIndexOffset.Add(FIntPoint(1,0));
	}

	//更改的block是在chunk上边缘或下边缘
	if(ChangedBlockIndexInChunk.Y == 1)
	{
		ToUpdateChunkIndexOffset.Add(FIntPoint(0,-1));
	}
	else if(ChangedBlockIndexInChunk.Y == ChunkYBlocks)
	{
		ToUpdateChunkIndexOffset.Add(FIntPoint(0,1));
	}

	//顶角的chunk被影响到
	if(ToUpdateChunkIndexOffset.Num() == 2)
	{
		ToUpdateChunkIndexOffset.Add(ToUpdateChunkIndexOffset[0] + ToUpdateChunkIndexOffset[1]);
	}

	for(int i=0;i<ToUpdateChunkIndexOffset.Num();i++)
	{
		//被影响到的chunk的世界索引 = 当前chunk的世界索引 + 偏移量
		FIntPoint ToUpdateEdgeChunkIndex = ChangedChunkIndexInWorld + ToUpdateChunkIndexOffset[i];
		if(Chunks.Contains(ToUpdateEdgeChunkIndex))
		{
			Chunks[ToUpdateEdgeChunkIndex]->SetBlock(changedBlockLocation, EBlockType::Air);
		}	
	}
}

void UTerrainGenerationComponent::DestroyBlock(FVector ImpactPoint, FVector ImpactNormal)
{
	//通过面上的击中点减去击中点的法向量得到block内部的点。
	const FVector PointInBlock = ImpactPoint - ImpactNormal * 0.5f * BlockSize;
	AChunk* ChunkBeHit = GetChunkByLocation(PointInBlock);
	ChunkBeHit->SetBlock(PointInBlock, EBlockType::Air);
	
	//更新邻近方块
	UpdateEdgeBlocks(PointInBlock,EBlockType::Air);
}