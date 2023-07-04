// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailTreeNode.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include"BlockDataStructs.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MC_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InventorySize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemBarSize = 10;
	
	//物品数据
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<EItemType, FItemData> ItemData;

	//背包栏,前ItemBarSize个元素为手持栏中物品，后InventorySize个元素为背包栏中物品。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UItemBase*> Inventory;

	UFUNCTION(BlueprintCallable)
	FItemAssetData GetAssetDataByItemType(EItemType ItemType);
	
	UFUNCTION(BlueprintCallable)
	EItemType ConvertBlockTypeToItemType(EBlockType BlockType);
	
	//为背包栏添加物品，返回未添加的数量。
	UFUNCTION(BlueprintCallable)
	int32 AddItemToInventory(EItemType ItemType , int32 Amount);

	//辅助函数，计算物品叠加后剩余的数量。
	UFUNCTION(BlueprintCallable)
	int32 GetLeftItemAmount(EItemType ItemType, int32 SourceAmount, int32 TargetAmount);
	
private:
	//物品数据表
	UPROPERTY(EditAnywhere,meta=(RequiredAssetDataTags = "RowStructure=ItemData"))
	UDataTable* ItemDataTable;
};