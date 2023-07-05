// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailTreeNode.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include"BlockDataStructs.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MC_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable)
	FInventoryChanged InventoryChanged;
	
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
	FItemAssetData GetAssetDataByItemType( EItemType ItemType) const;
	
	UFUNCTION(BlueprintCallable)
	EItemType ConvertBlockTypeToItemType(EBlockType BlockType);
	
	//为背包栏添加物品，返回未添加的数量。
	UFUNCTION(BlueprintCallable)
	int32 AddItemToInventory(EItemType ItemType , int32 Amount);

	//在背包栏指定位置添加物品，返回未添加的数量。
	UFUNCTION(BlueprintCallable)
	int32 AddItemToInventoryByIndex(int32 Index, EItemType ItemType , int32 Amount);

	UFUNCTION(BlueprintCallable)
	void SwapItemByIndex(int32 Index, UPARAM(ref)EItemType &ItemType, UPARAM(ref)int32 &Amount);

	//辅助函数，计算物品叠加后剩余的数量。
	UFUNCTION(BlueprintCallable)
	int32 GetLeftItemAmount(EItemType ItemType, int32 SourceAmount, int32 TargetAmount);
	
	//删除指定索引处的所有物品，返回被删除物品的个数。
	UFUNCTION(BlueprintCallable)
	int32 RemoveItemByIndex(int32 Index);

	//删除指定索引处的所有物品，返回被删除物品的个数。
	UFUNCTION(BlueprintCallable)
	int32 RemoveHalfItemByIndex(int32 Index);
	
private:
	//物品数据表
	UPROPERTY(EditAnywhere,meta=(RequiredAssetDataTags = "RowStructure=ItemData"))
	UDataTable* ItemDataTable;

	//记录第i类物品存放在哪些格子中。
	//TMap<EItemType, TSet<int32>> InventoryItemIndexMap;
};