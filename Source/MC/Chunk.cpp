// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "ProceduralMeshComponent.h"
#include "SimplexNoiseLibrary.h"
#include "TerrainGenerationComponent.h"
#include "Kismet/GameplayStatics.h"

struct FMeshData
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
};

//前面的面左下角为0，逆时针分别为0、1、2、3。从前面看后面的面，左上角为4，顺时针分别为4，5，6，7。
const FVector EightVerticesInABlock[8]{
	FVector(0,0,0),
	FVector(0,1,0),
	FVector(0,1,1),
	FVector(0,0,1),
	FVector(1,0,1),
	FVector(1,1,1),
	FVector(1,1,0),
	FVector(1,0,0)			
};
//只用每行前4个值添加顶点。添加三角形的索引只用第一行。因为每个面都添加4个顶点，有重复的顶点，不是简单的0-7这8个顶点了。
const int32 triangleIndex[6][6]{
	{ 0, 1, 2, 3, 0, 2},//前
	{ 1, 6, 5, 2, 1, 5},//右
	{ 6, 7, 4, 5, 6, 4},//后
	{ 7, 0, 3, 4, 7, 3},//左
	{ 3, 2, 5, 4, 3, 5},//上
	{ 7, 6, 1, 0, 7, 1},//下
};

//每个面的法向量
const FVector normals[6]{
	FVector{-1, 0, 0},
	FVector{ 0, 1, 0},
	FVector{ 1, 0, 0},
	FVector{ 0,-1, 0},
	FVector{ 0, 0, 1},
	FVector{ 0, 0,-1},
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
	FString String ="chunk";
	FName Name = FName(*String);
	ProceduralMeshComponent = NewObject<UProceduralMeshComponent>(this,Name);
	
	RootComponent = ProceduralMeshComponent;
	
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	GenerateChunk();
	UpdateMesh();
}

// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunk::OnConstruction(const FTransform& Transform)
{
	USimplexNoiseLibrary::setNoiseSeed(666);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this,0);
	if(PlayerController)
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
		UpdateMesh();
	}
}

TArray<int32> AChunk::CalculateNoise()
{
	TArray<int32> Noises;
	Noises.Reserve((ChunkXBlocks + 2) * (ChunkYBlocks + 2));	//多生成边缘一圈，得到邻近chunk的信息
	const int32 ChunkIndexInWorldX = ChunkIndexInWorld.X;
	const int32 ChunkIndexInWorldY = ChunkIndexInWorld.Y;
	
	for (int32 x = -1; x < ChunkXBlocks + 1; x++)
	{
		for (int32 y = -1; y < ChunkYBlocks + 1; y++)
		{
			float NoiseValue = 
			USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX*ChunkXBlocks + x) * 0.01f, (ChunkIndexInWorldY*ChunkYBlocks + y) * 0.01f) * 4 +
			USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX*ChunkXBlocks + x) * 0.01f, (ChunkIndexInWorldY*ChunkYBlocks + y) * 0.01f) * 8 +
			USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX*ChunkXBlocks + x) * 0.004f, (ChunkIndexInWorldY*ChunkYBlocks + y) * 0.004f) * 16 +
			FMath::Clamp(USimplexNoiseLibrary::SimplexNoise2D((ChunkIndexInWorldX*ChunkXBlocks + x) * 0.05f, (ChunkIndexInWorldY*ChunkYBlocks + y) * 0.05f), 0.0f, 5.0f) * 4; // clamp 0-5
			Noises.Add(FMath::FloorToInt(NoiseValue));
		}
	}
	return Noises;
}

void AChunk::GenerateChunk()
{
	Blocks.SetNum(ChunkZBlocks * (ChunkYBlocks + 2) * (ChunkXBlocks + 2));
	//根据当前chunk所在位置计算得到simplex噪声值。noise大小为(chunkYBlocks + 2) * (chunkXBlocks + 2)，(x,y)对应索引为y + x * (chunkYBlocks+2)
	TArray <int32> noise = CalculateNoise();
	//生成方块时，多生成边界一圈，相当于把chunk邻近chunk的边界也存起来了，方便后面剔除面判断。
	//（后面chunk之间可能还要交流信息，不然本chunk的边界方块发生了改变，邻近的chunk却还存着以前的信息。
	for(int32 x = 0; x < ChunkXBlocks + 2; x++)
	{
		for(int32 y = 0; y < ChunkYBlocks + 2; y++)
		{
			for(int32 z = 0; z < ChunkZBlocks; z++)
			{
				int32 Index = GetIndexInBlocksArray(x, y, z);
				int IndexInNoise = y + x * (ChunkYBlocks+2);
				if (z == 30 + noise[IndexInNoise]) Blocks[Index] = EBlockType::Grass;
				else if (z == 29 + noise[IndexInNoise]) Blocks[Index] = EBlockType::Grass;
				else if (z < 29 + noise[IndexInNoise]) Blocks[Index] = EBlockType::Grass;
				else Blocks[Index] = EBlockType::Air;
			}
		}
	}
}

void AChunk::UpdateMesh()
{
	TMap<EBlockType,FMeshData> MeshDataMap;
	//MeshData.SetNum(TerrainGenerationComponent->GetMaterialCount());
	
	//渲染的方块要去掉外面一圈其他chunk的方块。
	for(int32 x = 1; x < ChunkXBlocks + 1; x++)
	{
		for(int32 y = 1; y < ChunkYBlocks + 1; y++)
		{
			for(int32 z = 0; z < ChunkZBlocks; z++)
			{
				int32 Index = GetIndexInBlocksArray(x, y, z);
				EBlockType CurBlockType = Blocks[Index];
				//int32 CurBlockTypeInt = static_cast<int32>(CurBlockType);
				if(CurBlockType != EBlockType::Grass)
					continue;
				FMeshData& CurMeshData = MeshDataMap.FindOrAdd(CurBlockType);


				TArray<FVector> &Vertices = CurMeshData.Vertices;
				TArray<int32> &Triangles = CurMeshData.Triangles;
				TArray<FVector> &Normals = CurMeshData.Normals;
				TArray<FVector2D> &UV0 = CurMeshData.UV0;
				TArray<FColor> &VertexColors = CurMeshData.VertexColors;
				TArray<FProcMeshTangent> &Tangents = CurMeshData.Tangents;
				
				int numOfVertices = Vertices.Num();
				//x,y,z所在block的左下角顶点坐标(相对程序化网格体组件的坐标）
				FVector baseLocation =  FVector((x-1) * BlockSize, (y-1) * BlockSize, z * BlockSize);

				//添加六个面的渲染数据，前右后左上下。
				for(int32 i=0;i<6;i++)
				{
					//当前面朝向的block的索引
					int32 NearbyBlockIndex;
					switch (i) {
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
					}
					if (Blocks[NearbyBlockIndex] != EBlockType::Air)
						continue;

					//每个面有4个顶点
					for(int32 j=0;j<4;j++)
					{
						Vertices.Add(baseLocation + EightVerticesInABlock[triangleIndex[i][j]] * BlockSize);
						Normals.Add(normals[i]);
						UV0.Add(UV0s[j]);
					}
					//添加三角形索引
					for(int32 j=0;j<6;j++)
						Triangles.Add(numOfVertices + triangleIndex[0][j]);//0 1 2 3 0 2
					numOfVertices+=4;
				}
			}
		}
	}

	//需要吗？
	ProceduralMeshComponent->ClearAllMeshSections();

	// for(int i=0;i<MeshDataMap.Num();i++)
	// {
	// 	if(MeshDataMap[i].Vertices.Num()>0)
	// 		ProceduralMeshComponent->CreateMeshSection(i, MeshDataMap[i].Vertices, MeshDataMap[i].Triangles, MeshDataMap[i].Normals, MeshDataMap[i].UV0, MeshDataMap[i].VertexColors, MeshDataMap[i].Tangents, bHasCollision);
	// }

	for(auto i  :MeshDataMap)
	{
		EBlockType CurBlockType = i.Key;
		FMeshData& CurMeshData = i.Value;

		if(CurMeshData.Vertices.Num()>0)
		{
			int32 CurBlockTypeInt = static_cast<int32>(CurBlockType);
			ProceduralMeshComponent->CreateMeshSection(CurBlockTypeInt, CurMeshData.Vertices, CurMeshData.Triangles, CurMeshData.Normals, CurMeshData.UV0, CurMeshData.VertexColors, CurMeshData.Tangents, bHasCollision);
			UMaterialInterface* CurBlockMaterial = TerrainGenerationComponent->GetMaterialByType(CurBlockType);
			if(CurBlockMaterial)
				ProceduralMeshComponent->SetMaterial(CurBlockTypeInt, CurBlockMaterial);
		}
	}
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

bool AChunk::SetBlock(FVector Position, EBlockType Type)
{
	FIntVector LocalIndex = GetBlockIndexInChunk(Position);	
	
	int32 x = LocalIndex.X; 
	int32 y = LocalIndex.Y;
	int32 z = LocalIndex.Z;

	int32 Index = GetIndexInBlocksArray(x, y, z);
	if(Index <0 || Index > Blocks.Num()) return false;
	Blocks[Index] = Type;
	UpdateMesh();
	return true;
}