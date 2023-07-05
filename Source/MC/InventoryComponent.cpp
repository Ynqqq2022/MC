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
	for(int32 i = 0; i < ItemBarSize + InventorySize; i++)
	{
		Inventory.Add(NewObject<UItemBase>());
	}

	if (ItemDataTable)
	{
		TArray<FName> ItemDataRowNames = ItemDataTable->GetRowNames();
		for (FName RowName : ItemDataRowNames)
		{
			FItemData* ItemDataPtr = ItemDataTable->FindRow<FItemData>(RowName, FString());
			if (ItemDataPtr)
			{
				ItemData.Add(ItemDataPtr->ItemType, *ItemDataPtr);
				//InventoryItemIndexMap.Add(ItemDataPtr->ItemType, TSet<int32>());
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

FItemAssetData UInventoryComponent::GetAssetDataByItemType(EItemType ItemType) const
{
	const FItemData* CurItemData = ItemData.Find(ItemType);
	return CurItemData ? CurItemData->AssetData: FItemAssetData();	
}

EItemType UInventoryComponent::ConvertBlockTypeToItemType(EBlockType BlockType)
{
	switch (BlockType) {
		case EBlockType::Air: return EItemType::Nothing;;
		case EBlockType::Grass: return EItemType::Grass;
		case EBlockType::Stone: return EItemType::Stone;;
		case EBlockType::Soil:return EItemType::Soil;;
		default: return EItemType::Nothing;
	}
}

int32 UInventoryComponent::AddItemToInventory(EItemType ItemType, int32 Amount)
{
	FItemData* CurItemData = ItemData.Find(ItemType);
	int MaxStackSize = CurItemData ? CurItemData->NumericData.MaxStackSize : 0;
	int32 FirstEmptySlotIndex = -1;

	for (int i = 0; i < ItemBarSize + InventorySize; i++)
	{
		if (Amount <= 0) break;
		UItemBase* CurSlotItem = Inventory[i];
		//空的Slot
		if (CurSlotItem->ItemType == EItemType::Nothing)
		{
			if (FirstEmptySlotIndex == -1)FirstEmptySlotIndex = i;
		}
		else if (Inventory[i]->ItemType == ItemType) //同类物品
		{
			if (Amount + Inventory[i]->Amount < MaxStackSize)
			{
				Inventory[i]->Amount += Amount;
				Amount = 0;
			}
			else
			{
				Amount = Amount - (MaxStackSize - Inventory[i]->Amount);
				Inventory[i]->Amount = MaxStackSize;
			}
		}
	}
	//找完了，除了前面的空位，后面没有已经存在的同类格子。
	if (Amount > 0 && FirstEmptySlotIndex != -1)
	{
		for (int i = FirstEmptySlotIndex; i < ItemBarSize + InventorySize; i++)
		{
			if (Amount <= 0) break;
			UItemBase* CurSlotItem = Inventory[i];
			//空的Slot
			if (CurSlotItem->ItemType == EItemType::Nothing)
			{
				if (Amount < MaxStackSize)
				{
					Inventory[i]->ItemType = ItemType;
					Inventory[i]->Amount = Amount;
					Amount = 0;
				}
				else
				{
					Inventory[i]->ItemType = ItemType;
					Inventory[i]->Amount = MaxStackSize;
					Amount -= MaxStackSize;
				}
			}
		}
	}
	InventoryChanged.Broadcast();
	return Amount;
}

int32 UInventoryComponent::AddItemToInventoryByIndex(int32 Index, EItemType ItemType, int32 Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	if(TempItemBase->ItemType == ItemType)
	{
		int32 LeftAmount = GetLeftItemAmount(ItemType, Amount, TempItemBase->Amount);	
		TempItemBase->Amount += Amount - LeftAmount;
		return LeftAmount;
	}
	return Amount;
}

void UInventoryComponent::SwapItemByIndex(int32 Index, UPARAM(ref)EItemType &ItemType, UPARAM(ref)int32 &Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	Swap(ItemType, TempItemBase->ItemType);
	Swap(Amount, TempItemBase->Amount);
	InventoryChanged.Broadcast();
}

int32 UInventoryComponent::GetLeftItemAmount(EItemType ItemType, int32 SourceAmount, int32 TargetAmount)
{
	FItemData* CurItemData = ItemData.Find(ItemType);
	int32 MaxStackSize = CurItemData ? CurItemData->NumericData.MaxStackSize : 0;
	return MaxStackSize - TargetAmount >= SourceAmount ? 0 : SourceAmount - (MaxStackSize - TargetAmount);
}

int32 UInventoryComponent::RemoveItemByIndex(int32 Index)
{
	int32 AmountToRemove = Inventory[Index]->Amount;
	Inventory[Index] = NewObject<UItemBase>();
	InventoryChanged.Broadcast();
	return AmountToRemove;
}

int32 UInventoryComponent::RemoveHalfItemByIndex(int32 Index)
{
	int32 AmountToRemove = Inventory[Index]->Amount / 2;
	Inventory[Index]->Amount -= AmountToRemove;
	InventoryChanged.Broadcast();
	return AmountToRemove;
}
