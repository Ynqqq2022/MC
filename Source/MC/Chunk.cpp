// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"

#include <future>

#include "SimplexNoiseLibrary.h"
#include "TerrainGenerationComponent.h"
#include "MCSaveGameChunk.h"
#include "thread"
#include "mutex"
#include "Kismet/GameplayStatics.h"

static std::mutex MeshDataLock;

//前面的面左下角为0，逆时针分别为0、1、2、3。从前面看后面的面，左上角为4，顺时针分别为4，5，6，7。
const FVector EightVerticesInABlock[8]{
	FVector(0, 0, 0),
	FVector(0, 1, 0),
	FVector(0, 1, 1),
	FVector(0, 0, 1),
	FVector(1, 0, 1),
	FVector(1, 1, 1),
	FVector(1, 1, 0),
	FVector(1, 0, 0)
};
//只用每行前4个值添加顶点。添加三角形的索引只用第一行。因为每个面都添加4个顶点，有重复的顶点，不是简单的0-7这8个顶点了。
const int32 triangleIndex[6][6]{
	{0, 1, 2, 3, 0, 2}, //前
	{1, 6, 5, 2, 1, 5}, //右
	{6, 7, 4, 5, 6, 4}, //后
	{7, 0, 3, 4, 7, 3}, //左
	{3, 2, 5, 4, 3, 5}, //上
	{7, 6, 1, 0, 7, 1}, //下
};

//每个面的法向量
const FVector normals[6]{
	FVector{-1, 0, 0},
	FVector{0, 1, 0},
	FVector{1, 0, 0},
	FVector{0, -1, 0},
	FVector{0, 0, 1},
	FVector{0, 0, -1},
};

//每个面的UV坐标都一样
const FVector2D UV0s[4]
{
	FVector2D{0, 1},
	FVector2D{1, 1},
	FVector2D{1, 0},
	FVector2D{0, 0}
};

// Sets default values
AChunk::AChunk()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FString String = "chunk";
	FName Name = FName(*String);
	ProceduralMeshComponent = NewObject<UProceduralMeshComponent>(this, Name);
	MeshDataReady.BindUObject(this, &AChunk::ApplyMeshData);
	RootComponent = ProceduralMeshComponent;
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	GenerateChunk();
	AsyncUpdateMesh();
	//UpdateMesh();
}

// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChunk::OnConstruction(const FTransform& Transform)
{
	USimplexNoiseLibrary::setNoiseSeed(666);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		TerrainGenerationComponent = PlayerController->FindComponentByClass<UTerrainGenerationComponent>();
	}
	//UE_LOG(LogTemp,Warning,TEXT("=====================================================================%s"),*Transform.GetLocation().ToString());
}

void AChunk::UpdateCollision(bool Collision)
{
	if (bHasCollision != Collision)
	{
		bHasCollision = Collision;
		//AsyncUpdateMesh();
		ApplyMeshData();
		
	}
}

TArray<int32> AChunk::CalculateNoise()
{
	TArray<int32> Noises;
	Noises.Reserve((ChunkXBlocks + 2) * (ChunkYBlocks + 2)); //多生成边缘一圈，得到邻近chunk的信息
	const int32 ChunkIndexInWorldX = ChunkIndexInWorld.X;
	const int32 ChunkIndexInWorldY = ChunkIndexInWorld.Y;

	for (int32 x = -1; x < ChunkXBlocks + 1; x++)
	{
		for (int32 y = -1; y < ChunkYBlocks + 1; y++)
		{
			float NoiseValue =
				USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX * ChunkXBlocks + x) * 0.01f,
				                                     (ChunkIndexInWorldY * ChunkYBlocks + y) * 0.01f) * 4 +
				USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX * ChunkXBlocks + x) * 0.01f,
				                                     (ChunkIndexInWorldY * ChunkYBlocks + y) * 0.01f) * 8 +
				USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX * ChunkXBlocks + x) * 0.004f,
				                                     (ChunkIndexInWorldY * ChunkYBlocks + y) * 0.004f) * 16 +
				FMath::Clamp(USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX * ChunkXBlocks + x) * 0.05f,
				                                                  (ChunkIndexInWorldY * ChunkYBlocks + y) * 0.05f),
				             0.0f, 5.0f) * 4; // clamp 0-5
			Noises.Add(FMath::FloorToInt(NoiseValue));
		}
	}
	return Noises;
}

TMap<int32, EBlockType> AChunk::GetChangedBlocksBySaveFiles()
{
	FString CurSlotName = ChunkIndexInWorld.ToString();
	if (UGameplayStatics::DoesSaveGameExist(CurSlotName, 0))
	{
		UMCSaveGameChunk* LoadedChunk = Cast<UMCSaveGameChunk>(UGameplayStatics::LoadGameFromSlot(CurSlotName, 0));
		return LoadedChunk->ChangedBlocks;
	}
	return TMap<int32, EBlockType>();
}

void AChunk::GenerateChunk()
{
	//此处开个线程是否有点鸡肋？
	auto ChangedBlocksFuture = std::async(std::launch::async, &AChunk::GetChangedBlocksBySaveFiles,this);
	
	Blocks.SetNum(ChunkZBlocks * (ChunkYBlocks + 2) * (ChunkXBlocks + 2));
	//根据当前chunk所在位置计算得到simplex噪声值。noise大小为(chunkYBlocks + 2) * (chunkXBlocks + 2)，(x,y)对应索引为y + x * (chunkYBlocks+2)
	TArray<int32> noise = CalculateNoise();
	//生成方块时，多生成边界一圈，相当于把chunk邻近chunk的边界也存起来了，方便后面剔除面判断。
	//（后面chunk之间可能还要交流信息，不然本chunk的边界方块发生了改变，邻近的chunk却还存着以前的信息。
	for (int32 x = 0; x < ChunkXBlocks + 2; x++)
	{
		for (int32 y = 0; y < ChunkYBlocks + 2; y++)
		{
			for (int32 z = 0; z < ChunkZBlocks; z++)
			{
				int32 Index = GetIndexInBlocksArray(x, y, z);
				int IndexInNoise = y + x * (ChunkYBlocks + 2);
				if (z == 30 + noise[IndexInNoise]) Blocks[Index] = EBlockType::Grass;
				else if (z > 25 + noise[IndexInNoise] && z < 30 + noise[IndexInNoise]) Blocks[Index] = EBlockType::Soil;
				else if (z <= 25 + noise[IndexInNoise]) Blocks[Index] = EBlockType::Stone;
				else Blocks[Index] = EBlockType::Air;
			}
		}
	}

	ChangedBlocks = ChangedBlocksFuture.get();
	//有可能该方块以前更改过，后面又改成原来的类型了，记录下来，从ChangedBlocks中删去。
	TArray<int32> HasNotChangedIndex;
	for (auto i : ChangedBlocks)
	{
		int32 Index = i.Key;
		EBlockType ChangedType = i.Value;
		if (Blocks[Index] != ChangedType)
		{
			Blocks[Index] = ChangedType;
		}
		else
		{
			HasNotChangedIndex.Add(Index);
		}
	}
	for (auto i : HasNotChangedIndex)
	{
		ChangedBlocks.Remove(i);
	}
}

void AChunk::PrepareMeshData()
{
	std::lock_guard<std::mutex> mm(MeshDataLock);
	MeshDataMap.Empty();
	//渲染的方块要去掉外面一圈其他chunk的方块。
	for (int32 x = 1; x < ChunkXBlocks + 1; x++)
	{
		for (int32 y = 1; y < ChunkYBlocks + 1; y++)
		{
			for (int32 z = 0; z < ChunkZBlocks; z++)
			{
				int32 Index = GetIndexInBlocksArray(x, y, z);
				EBlockType CurBlockType = Blocks[Index];
				//int32 CurBlockTypeInt = static_cast<int32>(CurBlockType);
				if (CurBlockType == EBlockType::Air)
					continue;
				FMeshData& CurMeshData = MeshDataMap.FindOrAdd(CurBlockType);

				TArray<FVector>& Vertices = CurMeshData.Vertices;
				TArray<int32>& Triangles = CurMeshData.Triangles;
				TArray<FVector>& Normals = CurMeshData.Normals;
				TArray<FVector2D>& UV0 = CurMeshData.UV0;
				TArray<FColor>& VertexColors = CurMeshData.VertexColors;
				TArray<FProcMeshTangent>& Tangents = CurMeshData.Tangents;

				int numOfVertices = Vertices.Num();
				//x,y,z所在block的左下角顶点坐标(相对程序化网格体组件的坐标）
				FVector baseLocation = FVector((x - 1) * BlockSize, (y - 1) * BlockSize, z * BlockSize);

				//添加六个面的渲染数据，前右后左上下。
				for (int32 i = 0; i < 6; i++)
				{
					//当前面朝向的block的索引
					int32 NearbyBlockIndex;
					switch (i)
					{
					case 0:
						NearbyBlockIndex = GetIndexInBlocksArray(x - 1, y, z);
						break;
					case 1:
						NearbyBlockIndex = GetIndexInBlocksArray(x, y + 1, z);
						break;
					case 2:
						NearbyBlockIndex = GetIndexInBlocksArray(x + 1, y, z);
						break;
					case 3:
						NearbyBlockIndex = GetIndexInBlocksArray(x, y - 1, z);
						break;
					case 4:
						NearbyBlockIndex = GetIndexInBlocksArray(x, y, z + 1);
						break;
					case 5:
						NearbyBlockIndex = GetIndexInBlocksArray(x, y, z - 1);
						break;
					default:
						NearbyBlockIndex = 0;
					}
					if (Blocks[NearbyBlockIndex] != EBlockType::Air)
						continue;

					//每个面有4个顶点
					for (int32 j = 0; j < 4; j++)
					{
						Vertices.Add(baseLocation + EightVerticesInABlock[triangleIndex[i][j]] * BlockSize);
						Normals.Add(normals[i]);
						UV0.Add(UV0s[j]);
					}
					//添加三角形索引
					for (int32 j = 0; j < 6; j++)
						Triangles.Add(numOfVertices + triangleIndex[0][j]); //0 1 2 3 0 2
					numOfVertices += 4;
				}
			}
		}
	}
}

void AChunk::AsyncPrepareMeshData()
{
	//UE_LOG(LogTemp,Warning,TEXT("Thread ID:%d"),FPlatformTLS::GetCurrentThreadId())
	PrepareMeshData();
	//主线程中应用Mesh数据
	AsyncTask(ENamedThreads::GameThread,[this]()
	{
		MeshDataReady.ExecuteIfBound();
	});
}

void AChunk::ApplyMeshData()
{
	std::lock_guard<std::mutex> mm(MeshDataLock);
	//需要吗？
	ProceduralMeshComponent->ClearAllMeshSections();

	for (auto i : MeshDataMap)
	{
		EBlockType CurBlockType = i.Key;
		FMeshData& CurMeshData = i.Value;

		if (CurMeshData.Vertices.Num() > 0)
		{
			int32 CurBlockTypeInt = static_cast<int32>(CurBlockType);
			ProceduralMeshComponent->CreateMeshSection(CurBlockTypeInt, CurMeshData.Vertices, CurMeshData.Triangles,
													   CurMeshData.Normals, CurMeshData.UV0, CurMeshData.VertexColors,
													   CurMeshData.Tangents, bHasCollision);
			if (TerrainGenerationComponent != nullptr)
			{
				UMaterialInterface* CurBlockMaterial = TerrainGenerationComponent->GetMaterialByType(CurBlockType);
				if (CurBlockMaterial)
					ProceduralMeshComponent->SetMaterial(CurBlockTypeInt, CurBlockMaterial);
			}
		}
	}	
}

void AChunk::UpdateMesh()
{
	PrepareMeshData();
	ApplyMeshData();
}

void AChunk::AsyncUpdateMesh()
{
	//使用线程计算Mesh数据
	 AsyncTask(ENamedThreads::AnyThread,[this]()
	 {
	 	AsyncPrepareMeshData();
	 });
	// std::thread PrepareMeshDataThread{&AChunk::AsyncPrepareMeshData, this};
	// PrepareMeshDataThread.detach();
}

FIntVector AChunk::GetBlockIndexInChunk(const FVector WorldPosition)
{
	//以block[0]坐标为原点，即算上周围一圈方块后，worldPosition在当前chunk中的相对坐标。
	const FVector LocalPosition = WorldPosition - ChunkLocationInWorld + FVector(BlockSize, BlockSize, 0);
	int32 x = FMath::FloorToInt(LocalPosition.X / BlockSize);
	int32 y = FMath::FloorToInt(LocalPosition.Y / BlockSize);
	int32 z = FMath::FloorToInt(LocalPosition.Z / BlockSize);
	return {x, y, z};
}

FVector AChunk::GetBlockCenterPosition(const FVector WorldPosition)
{
	FIntVector Index = GetBlockIndexInChunk(WorldPosition);
	FVector CenterPosition;

	CenterPosition[0] = (Index[0] - 0.5f) * BlockSize;
	CenterPosition[1] = (Index[1] - 0.5f) * BlockSize;
	CenterPosition[2] = (Index[2] + 0.5f) * BlockSize;
	CenterPosition += ChunkLocationInWorld;

	return CenterPosition;
}

bool AChunk::SetBlock(FVector Position, EBlockType Type)
{
	FIntVector LocalIndex = GetBlockIndexInChunk(Position);

	int32 x = LocalIndex.X;
	int32 y = LocalIndex.Y;
	int32 z = LocalIndex.Z;

	int32 Index = GetIndexInBlocksArray(x, y, z);
	if (Index < 0 || Index > Blocks.Num()) return false;
	Blocks[Index] = Type;
	ChangedBlocks.Add(Index, Type);
	AsyncUpdateMesh();
	//UpdateMesh();
	return true;
}

EBlockType AChunk::GetBlockType(FVector Position)
{
	FIntVector LocalIndex = GetBlockIndexInChunk(Position);

	int32 x = LocalIndex.X;
	int32 y = LocalIndex.Y;
	int32 z = LocalIndex.Z;

	int32 Index = GetIndexInBlocksArray(x, y, z);
	if (Index < 0 || Index > Blocks.Num()) return EBlockType::Air;
	return Blocks[Index];
}

void AChunk::AsyncSaveChunk()
{
	if (ChangedBlocks.Num() == 0)
		return;
	FString CurSlotName = ChunkIndexInWorld.ToString();
	UMCSaveGameChunk* CurSave = NewObject<UMCSaveGameChunk>();
	CurSave->ChangedBlocks = ChangedBlocks;
	UGameplayStatics::AsyncSaveGameToSlot(CurSave, CurSlotName, 0);
}

void AChunk::SaveChunk()
{
	if (ChangedBlocks.Num() == 0)
		return;
	FString CurSlotName = ChunkIndexInWorld.ToString();
	UMCSaveGameChunk* CurSave = NewObject<UMCSaveGameChunk>();
	CurSave->ChangedBlocks = ChangedBlocks;
	UGameplayStatics::SaveGameToSlot(CurSave, CurSlotName, 0);
}
