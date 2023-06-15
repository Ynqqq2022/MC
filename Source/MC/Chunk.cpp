// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "ProceduralMeshComponent.h"
#include "SimplexNoiseLibrary.h"

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
	FString string ="chunk";
	FName name = FName(*string);
	proceduralMeshComponent = NewObject<UProceduralMeshComponent>(this,name);
	
	RootComponent = proceduralMeshComponent;
	
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
	// FString string ="chunk";
	// FName name = FName(*string);
	// proceduralMeshComponent = NewObject<UProceduralMeshComponent>(this,name);
	// proceduralMeshComponent->RegisterComponent();
	USimplexNoiseLibrary::setNoiseSeed(666);
	//UE_LOG(LogTemp,Warning,TEXT("=====================================================================%s"),*Transform.GetLocation().ToString());
}

TArray<int32> AChunk::calculateNoise()
{
	TArray<int32> noises;
	noises.Reserve(chunkZBlocks * chunkYBlocks);
	int32 chunkIndexInWorldX = chunkIndexInWorld.X;
	int32 chunkIndexInWorldY = chunkIndexInWorld.Y;
	
	for (int32 y = 0; y < chunkYBlocks; y++)
	{
		for (int32 x = 0; x < chunkXBlocks; x++)
		{
			float noiseValue = 
			USimplexNoiseLibrary::SimplexNoise2D((chunkIndexInWorldX*chunkXBlocks + x) * 0.01f, (chunkIndexInWorldY*chunkYBlocks + y) * 0.01f) * 4 +
			USimplexNoiseLibrary::SimplexNoise2D((chunkIndexInWorldX*chunkXBlocks + x) * 0.01f, (chunkIndexInWorldY*chunkYBlocks + y) * 0.01f) * 8 +
			USimplexNoiseLibrary::SimplexNoise2D((chunkIndexInWorldX*chunkXBlocks + x) * 0.004f, (chunkIndexInWorldY*chunkYBlocks + y) * 0.004f) * 16 +
			FMath::Clamp(USimplexNoiseLibrary::SimplexNoise2D((chunkIndexInWorldX*chunkXBlocks + x) * 0.05f, (chunkIndexInWorldY*chunkYBlocks + y) * 0.05f), 0.0f, 5.0f) * 4; // clamp 0-5
			noises.Add(FMath::FloorToInt(noiseValue));
		}
	}
	return noises;
}

void AChunk::GenerateChunk()
{
	blocks.SetNum(chunkZBlocks * chunkYBlocks * chunkXBlocks);
	TArray <int32> noise = calculateNoise();
	for(int32 x = 0;x < chunkXBlocks; x++)
	{
		for(int32 y = 0; y < chunkYBlocks; y++)
		{
			for(int32 z = 0;z < chunkZBlocks; z++)
			{
				int32 index = getIndexInBlocksArray(x, y, z);
				//UE_LOG(LogTemp,Warning,TEXT("%d"),noise[x + y * chunkYBlocks ]);	
				if (z == 30 + noise[x + y * chunkYBlocks ]) blocks[index] = EBlockType::Grass;
				else if (z == 29 + noise[x + y * chunkYBlocks]) blocks[index] = EBlockType::Air;
				else if (z < 29 + noise[x + y * chunkYBlocks]) blocks[index] = EBlockType::Air;
				else blocks[index] = EBlockType::Air;
			}
		}
	}
}

void AChunk::UpdateMesh()
{
	TArray<FMeshData> meshData;
	meshData.SetNum(4);
	
	for(int32 x = 0;x < chunkXBlocks; x++)
	{
		for(int32 y = 0; y < chunkYBlocks; y++)
		{
			for(int32 z = 0;z < chunkZBlocks; z++)
			{
				int32 index = getIndexInBlocksArray(x, y, z);
				EBlockType curBlockType = blocks[index];
				int32 curBlockTypeInt = static_cast<int32>(curBlockType);
				if(curBlockType != EBlockType::Grass)
					continue;
				
				TArray<FVector> &Vertices = meshData[curBlockTypeInt].Vertices;
				TArray<int32> &Triangles = meshData[curBlockTypeInt].Triangles;
				TArray<FVector> &Normals = meshData[curBlockTypeInt].Normals;
				TArray<FVector2D> &UV0 = meshData[curBlockTypeInt].UV0;
				TArray<FColor> &VertexColors = meshData[curBlockTypeInt].VertexColors;
				TArray<FProcMeshTangent> &Tangents = meshData[curBlockTypeInt].Tangents;
				
				int numOfVertices = Vertices.Num();
				//x,y,z所在block的左下角顶点坐标(相对程序化网格体组件的坐标）
				FVector baseLocation =  FVector(x * blockSize, y * blockSize, z * blockSize);

				//添加六个面的顶点数据
				for(int32 i=0;i<6;i++)
				{
					//每个面有4个顶点
					for(int32 j=0;j<4;j++)
					{
						Vertices.Add(baseLocation + EightVerticesInABlock[triangleIndex[i][j]] * blockSize);
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
	proceduralMeshComponent->ClearAllMeshSections();

	for(int i=0;i<meshData.Num();i++)
	{
		if(meshData[i].Vertices.Num()>0)
			proceduralMeshComponent->CreateMeshSection(i, meshData[i].Vertices, meshData[i].Triangles, meshData[i].Normals, meshData[i].UV0, meshData[i].VertexColors,meshData[i].Tangents,false);
	}
}

