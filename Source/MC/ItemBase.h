// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemBase.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Nothing UMETA(DisplayName = "Nothing"),
	Grass UMETA(DisplayName = "Grass"),
	Stone UMETA(DisplayName = "Stone"),
	Soil UMETA(DisplayName = "Soil"),
};

USTRUCT()
struct FItemNumericData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	int32 MaxStackSize;

	// UPROPERTY()
	// bool bIsStackable;
};

USTRUCT()
struct FItemTextData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	FText Description;
};

USTRUCT(BlueprintType)
struct FItemAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Material;
};

USTRUCT()
struct FItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	float DamageValue;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemTextData TextData;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemNumericData NumericData;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemAssetData AssetData;
};

// UCLASS(BlueprintType)
// class MC_API UItemBase : public UObject
// {
// 	GENERATED_BODY()
// public:
// 	//UItemBase(){ItemType = EItemType::Nothing; Amount = 0;}
// 	UPROPERTY(BlueprintReadOnly)
// 	EItemType ItemType;
// 	UPROPERTY(BlueprintReadOnly)
// 	int32 Amount;
// };

USTRUCT(BlueprintType)
struct FItemBase
{
	GENERATED_BODY()

	FItemBase(){ItemType = EItemType::Nothing; Amount = 0;}
	UPROPERTY(BlueprintReadWrite)
	EItemType ItemType;
	UPROPERTY(BlueprintReadWrite)
	int32 Amount;
};