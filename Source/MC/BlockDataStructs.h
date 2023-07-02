#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BlockDataStructs.generated.h"

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	Air UMETA(DisplayName = "Air"),
	Grass UMETA(DisplayName = "Grass"),
	Stone UMETA(DisplayName = "Stone"),
	Soil UMETA(DisplayName = "Soil")
};

USTRUCT()
struct FBlockAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
};

//记录方块信息的数据表，RowName要保证和BlockType一致。
USTRUCT()
struct FBlockData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	EBlockType Type;
	
	UPROPERTY(EditAnywhere)
	FBlockAssetData AssetData;
};