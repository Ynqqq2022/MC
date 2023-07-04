// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Inventory.SetNum(InventorySize);
	ItemBar.SetNum(ItemBarSize);

	if (ItemDataTable)
	{
		TArray<FName> ItemDataRowNames = ItemDataTable->GetRowNames();
		for (FName RowName : ItemDataRowNames)
		{
			FItemData* ItemDataPtr = ItemDataTable->FindRow<FItemData>(RowName, FString());
			if (ItemDataPtr)
			{
				ItemData.Add(ItemDataPtr->ItemType, *ItemDataPtr);
			}
		}
	}

	 //AddItemToItemBar(EItemType::Grass, 20);
	// AddItemToItemBar(EItemType::Soil, 3);
	// ...
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

FItemAssetData UInventoryComponent::GetAssetDataByItemType(EItemType ItemType)
{
	FItemData* CurItemData = ItemData.Find(ItemType);
	return CurItemData ? CurItemData->AssetData: FItemAssetData();	
}

EItemType UInventoryComponent::ConvertBlockTypeToItemType(EBlockType BlockType)
{
	switch (BlockType) {
		case EBlockType::Air: return EItemType::Grass;;
		case EBlockType::Grass: return EItemType::Grass;
		case EBlockType::Stone: return EItemType::Stone;;
		case EBlockType::Soil:return EItemType::Soil;;
		default: return EItemType::Grass;
	}
}

int32 UInventoryComponent::AddItemToContainer(TArray<UItemBase*>& Container, int32 ContainerSize, EItemType ItemType,
                                              int32 Amount)
{
	FItemData* CurItemData = ItemData.Find(ItemType);
	int MaxStackSize = CurItemData ? CurItemData->NumericData.MaxStackSize : 0;
	int32 FirstEmptySlotIndex = -1;
	for (int i = 0; i < ContainerSize; i++)
	{
		if (Amount <= 0) return 0;
		UItemBase* CurSlotItem = Container[i];
		//空的Slot
		if (!CurSlotItem)
		{
			if (FirstEmptySlotIndex == -1)FirstEmptySlotIndex = i;
		}
		else if (Container[i]->ItemType == ItemType) //同类物品
		{
			if (Amount + Container[i]->Amount < MaxStackSize)
			{
				Container[i]->Amount += Amount;
				Amount = 0;
			}
			else
			{
				Amount = Amount - (MaxStackSize - Container[i]->Amount);
				Container[i]->Amount = MaxStackSize;
			}
		}
	}
	//找完了，除了前面的空位，后面没有已经存在的同类格子。
	if (Amount > 0 && FirstEmptySlotIndex != -1)
	{
		for (int i = FirstEmptySlotIndex; i < ContainerSize; i++)
		{
			if (Amount <= 0) return 0;
			UItemBase* CurSlotItem = Container[i];
			//空的Slot
			if (!CurSlotItem)
			{
				if (Amount < MaxStackSize)
				{
					UItemBase* TempItem = NewObject<UItemBase>();
					TempItem->ItemType = ItemType;
					TempItem->Amount = Amount;
					Container[i] = TempItem;
					Amount = 0;
				}
				else
				{
					UItemBase* TempItem = NewObject<UItemBase>();
					TempItem->ItemType = ItemType;
					TempItem->Amount = MaxStackSize;
					Container[i] = TempItem;
					Amount -= MaxStackSize;
				}
			}
		}
	}
	return Amount;
}

int32 UInventoryComponent::AddItemToItemBar(EItemType ItemType, int32 Amount)
{
	return AddItemToContainer(ItemBar, ItemBarSize, ItemType, Amount);
}

int32 UInventoryComponent::AddItemToInventory(EItemType ItemType, int32 Amount)
{
	return AddItemToContainer(Inventory, InventorySize, ItemType, Amount);
}

int32 UInventoryComponent::GetLeftItemAmount(EItemType ItemType, int32 SourceAmount, int32 TargetAmount)
{
	FItemData* CurItemData = ItemData.Find(ItemType);
	int32 MaxStackSize = CurItemData ? CurItemData->NumericData.MaxStackSize : 0;
	return MaxStackSize - TargetAmount >= SourceAmount ? 0 : SourceAmount - (MaxStackSize - TargetAmount);
}
