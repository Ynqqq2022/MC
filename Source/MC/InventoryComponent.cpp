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
	/*
	 * 先检测Bar里有没有同类的，有则在上面+，满了则下一个，都满了则第一个空的。
	 *							没有则第一个空的。
	 *	bar都满了，则看inventory里有没有同类的，有则在上面+，满了则下一个，都满了则第一个空的。
	 *								没有则第一个空的。
	 */
	FItemData* CurItemData = ItemData.Find(ItemType);
	int32 CurItemMaxSize = CurItemData->NumericData.MaxStackSize;
	
	auto SameKindPred = [ItemType, CurItemMaxSize](const UItemBase* Ele)
	{
		return Ele->ItemType == ItemType && Ele->Amount < CurItemMaxSize;
	};

	auto EmptySlotPred = [](const UItemBase* Ele)
	{
		return Ele->ItemType == EItemType::Nothing;
	};

	//先在手持栏中找是否存有同类物品，有的话叠加。
	int32  ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	while(ThisTypeIndex != INDEX_NONE && ThisTypeIndex < ItemBarSize)
	{
		if(Amount <= 0) break;
		AddItemToInventoryByIndex(ThisTypeIndex, ItemType, Amount);	
		ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	}

	//手持栏无同类物品，或同类物品已叠加满，使用手持栏空余格子。
	int32  EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	while(EmptySlotIndex != INDEX_NONE && EmptySlotIndex < ItemBarSize)
	{
		if(Amount <= 0) break;
		AddItemToInventoryByIndex(EmptySlotIndex, ItemType, Amount);	
		EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	}

	//手持栏满了，存入背包栏，先与同类物品叠加。
	ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	while(ThisTypeIndex != INDEX_NONE && ThisTypeIndex >= ItemBarSize)
	{
		if(Amount <= 0) break;
		AddItemToInventoryByIndex(ThisTypeIndex, ItemType, Amount);	
		ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	}

	//找背包栏空余处存放。
	EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	while(EmptySlotIndex != INDEX_NONE && EmptySlotIndex >= ItemBarSize)
	{
		if(Amount <= 0) break;
		AddItemToInventoryByIndex(EmptySlotIndex, ItemType, Amount);	
		EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	}
	
	return Amount;
}

void UInventoryComponent::AddItemToInventoryByIndex(int32 Index, EItemType ItemType, UPARAM(ref)int32 &Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	if(TempItemBase->ItemType == EItemType::Nothing || TempItemBase->ItemType == ItemType)
	{
		int32 LeftAmount = GetLeftItemAmount(ItemType, Amount, TempItemBase->Amount);
		Swap(TempItemBase->ItemType ,ItemType);
		TempItemBase->Amount += Amount - LeftAmount;
		Amount = LeftAmount;
	}
}

void UInventoryComponent::SwapItemByIndex(int32 Index, UPARAM(ref)EItemType &ItemType, UPARAM(ref)int32 &Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	Swap(ItemType, TempItemBase->ItemType);
	Swap(Amount, TempItemBase->Amount);
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
	return AmountToRemove;
}

int32 UInventoryComponent::RemoveHalfItemByIndex(int32 Index)
{
	int32 AmountToRemove = Inventory[Index]->Amount / 2;
	Inventory[Index]->Amount -= AmountToRemove;
	return AmountToRemove;
}
