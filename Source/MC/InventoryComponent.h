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
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
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

	//备份数据，方便用于动态分配物品。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UItemBase*> InventoryBackUp;
	
	UFUNCTION(BlueprintCallable)
	FItemAssetData GetAssetDataByItemType( EItemType ItemType) const;

	UFUNCTION(BlueprintCallable)
	int32 GetMaxStackSizeByItemType( EItemType ItemType) const;
	
	UFUNCTION(BlueprintCallable)
	EItemType ConvertBlockTypeToItemType(EBlockType BlockType);

	
	/*自动找到合适的位置添加添加物品，返回未添加的数量。
	 * 优先先手持栏，优先同类物品叠加，再空余的手持栏。手持栏满了则放入背包。
	 */
	UFUNCTION(BlueprintCallable)
	int32 AutoAddItemToInventory(EItemType ItemType , int32 Amount);

	//在背包栏指定位置添加物品,通过引用传递ItemType和Amount,表示操作结束后源物品的状态。
	UFUNCTION(BlueprintCallable)
	void AddItemToInventoryByIndex(int32 Index, UPARAM(ref)EItemType ItemType, UPARAM(ref)int32 &Amount);

	//在背包栏指定位置添加一个指定类型的物品,通过引用传递ItemType和Amount,表示操作结束后源物品的状态。
	UFUNCTION(BlueprintCallable)
	void SplitSingleItemToInventoryByIndex(int32 Index, UPARAM(ref)EItemType ItemType, UPARAM(ref)int32 &Amount);

	//互换背包指定位置的物品。
	UFUNCTION(BlueprintCallable)
	void SwapItemByIndex(int32 Index, UPARAM(ref)EItemType &ItemType, UPARAM(ref)int32 &Amount);

	//辅助函数，计算同类物品叠加后剩余的数量。
	UFUNCTION(BlueprintCallable)
	int32 GetLeftItemAmount(EItemType ItemType, int32 SourceAmount, int32 TargetAmount);
	
	//删除指定索引处的所有物品，返回被删除物品的个数。
	UFUNCTION(BlueprintCallable)
	int32 RemoveItemByIndex(int32 Index);

	//删除指定索引处一半的物品，返回被删除物品的个数。
	UFUNCTION(BlueprintCallable)
	int32 RemoveHalfItemByIndex(int32 Index);

	//直接设置背包的函数，用于复原操作。。。合理吗？
	UFUNCTION(BlueprintCallable)
	void SetItemByIndex(int32 Index, EItemType ItemType, int32 Amount);

	//返回可以添加的数量
	UFUNCTION(BlueprintCallable)
	int32 CanAddItem(int32 Index, EItemType ItemType, int32 Amount);

	UFUNCTION(BlueprintCallable)
	void BackUp();

	//动态分配拖拽时每个格子。
	UFUNCTION(BlueprintCallable)
	int32 AutoDiv(EItemType ItemType, int32 TotalAmount, TArray<int32> MovedSlotIndices);

	//试图将背包中该类物品整合到一个格子，通过引用返回整合后的数量。
	UFUNCTION(BlueprintCallable)
	void GetItemStack(EItemType ItemType, UPARAM(ref)int32& CurAmount);
private:
	//物品数据表
	UPROPERTY(EditAnywhere,meta=(RequiredAssetDataTags = "RowStructure=ItemData"))
	UDataTable* ItemDataTable;
};