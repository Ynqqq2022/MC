// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include"BlockDataStructs.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectSlotIndexChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectItemTypeChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MC_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	
	virtual void InitializeComponent() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable)
	FInventoryChanged InventoryChanged;

	UPROPERTY(BlueprintAssignable)
	FSelectSlotIndexChanged SelectSlotIndexChanged;

	UPROPERTY(BlueprintAssignable)
	FSelectItemTypeChanged SelectItemTypeChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InventorySize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemBarSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CurSelectItemBarIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType PreSelectItemType = EItemType::Nothing;

	//物品数据
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<EItemType, FItemData> ItemData;

	//背包栏,前ItemBarSize个元素为手持栏中物品，后InventorySize个元素为背包栏中物品。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FItemBase> Inventory;

	//备份数据，方便用于动态分配物品。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FItemBase> InventoryBackUp;
	
	UFUNCTION(BlueprintCallable)
	FItemAssetData GetAssetDataByItemType( EItemType ItemType) const;

	UFUNCTION(BlueprintCallable)
	FItemAssetData GetSelectItemAssetData() const;
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxStackSizeByItemType( EItemType ItemType) const;
	
	UFUNCTION(BlueprintCallable)
	EItemType ConvertBlockTypeToItemType(EBlockType BlockType) const;

	UFUNCTION(BlueprintCallable)
	EBlockType ConvertItemTypeToBlockType(EItemType ItemType) const;
	
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

	//删除一个选中的物品。
	UFUNCTION(BlueprintCallable)
	void RemoveOneSelectItem();
	
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

	//返回当前选择的物品的种类。
	UFUNCTION(BlueprintCallable)
	EItemType GetCurSelectItemType()const{return Inventory[CurSelectItemBarIndex].ItemType;}

	//返回当前选择的物品的种类对应的BlockType,不是方块则返回EBlockType::Air。
	UFUNCTION(BlueprintCallable)
	EBlockType GetCurSelectBlockType()const{return ConvertItemTypeToBlockType(Inventory[CurSelectItemBarIndex].ItemType);}
	
	//设置当前选中的物品
	UFUNCTION(BlueprintCallable)
	void ChangeCurSelectByIndex(int32 Index);

	//向后选择物品
	UFUNCTION(BlueprintCallable)
	void ChangeCurSelectForward();

	//向前选择物品
	UFUNCTION(BlueprintCallable)
	void ChangeCurSelectBackward();

	//操作背包后，应发送背包变化的消息。
	UFUNCTION(BlueprintCallable)
	void SendInventoryChangedMessage();
private:
	//物品数据表
	UPROPERTY(EditAnywhere,meta=(RequiredAssetDataTags = "RowStructure=ItemData"))
	UDataTable* ItemDataTable;

	//检测选择的物品种类是否变了，变了则发送该消息。
	void SendSelectItemChangeMessage();

};