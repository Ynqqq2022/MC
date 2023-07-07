// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"

#include "Chaos/Pair.h"

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
	for (int32 i = 0; i < ItemBarSize + InventorySize; i++)
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
	AutoAddItemToInventory(EItemType::Grass, 64*3);
	AutoAddItemToInventory(EItemType::Soil, 64*3);
	AutoAddItemToInventory(EItemType::Stone, 64*3);
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
	return CurItemData ? CurItemData->AssetData : FItemAssetData();
}

int32 UInventoryComponent::GetMaxStackSizeByItemType(EItemType ItemType) const
{
	const FItemData* CurItemData = ItemData.Find(ItemType);
	return CurItemData->NumericData.MaxStackSize;
}

EItemType UInventoryComponent::ConvertBlockTypeToItemType(EBlockType BlockType)
{
	switch (BlockType)
	{
	case EBlockType::Air: return EItemType::Nothing;;
	case EBlockType::Grass: return EItemType::Grass;
	case EBlockType::Stone: return EItemType::Stone;;
	case EBlockType::Soil: return EItemType::Soil;;
	default: return EItemType::Nothing;
	}
}

int32 UInventoryComponent::AutoAddItemToInventory(EItemType ItemType, int32 Amount)
{
	/*
	 * 先检测Bar里有没有同类的，有则在上面+，满了则下一个，都满了则第一个空的。
	 *							没有则第一个空的。
	 *	bar都满了，则看inventory里有没有同类的，有则在上面+，满了则下一个，都满了则第一个空的。
	 *								没有则第一个空的。
	 */
	int32 CurItemMaxSize = GetMaxStackSizeByItemType(ItemType);

	auto SameKindPred = [ItemType, CurItemMaxSize](const UItemBase* Ele)
	{
		return Ele->ItemType == ItemType && Ele->Amount < CurItemMaxSize;
	};

	auto EmptySlotPred = [](const UItemBase* Ele)
	{
		return Ele->ItemType == EItemType::Nothing;
	};

	//先在手持栏中找是否存有同类物品，有的话叠加。
	int32 ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	while (ThisTypeIndex != INDEX_NONE && ThisTypeIndex < ItemBarSize)
	{
		if (Amount <= 0) break;
		AddItemToInventoryByIndex(ThisTypeIndex, ItemType, Amount);
		ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	}

	//手持栏无同类物品，或同类物品已叠加满，使用手持栏空余格子。
	int32 EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	while (EmptySlotIndex != INDEX_NONE && EmptySlotIndex < ItemBarSize)
	{
		if (Amount <= 0) break;
		AddItemToInventoryByIndex(EmptySlotIndex, ItemType, Amount);
		EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	}

	//手持栏满了，存入背包栏，先与同类物品叠加。
	ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	while (ThisTypeIndex != INDEX_NONE && ThisTypeIndex >= ItemBarSize)
	{
		if (Amount <= 0) break;
		AddItemToInventoryByIndex(ThisTypeIndex, ItemType, Amount);
		ThisTypeIndex = Inventory.IndexOfByPredicate(SameKindPred);
	}

	//找背包栏空余处存放。
	EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	while (EmptySlotIndex != INDEX_NONE && EmptySlotIndex >= ItemBarSize)
	{
		if (Amount <= 0) break;
		AddItemToInventoryByIndex(EmptySlotIndex, ItemType, Amount);
		EmptySlotIndex = Inventory.IndexOfByPredicate(EmptySlotPred);
	}

	return Amount;
}

void UInventoryComponent::AddItemToInventoryByIndex(int32 Index, UPARAM(ref)EItemType ItemType,
                                                    UPARAM(ref)int32& Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	if (TempItemBase->ItemType == EItemType::Nothing || TempItemBase->ItemType == ItemType)
	{
		int32 LeftAmount = GetLeftItemAmount(ItemType, Amount, TempItemBase->Amount);
		Swap(TempItemBase->ItemType, ItemType);
		TempItemBase->Amount += Amount - LeftAmount;
		Amount = LeftAmount;
		if (Amount == 0)
		{
			ItemType = EItemType::Nothing;
		}
	}
}

void UInventoryComponent::SplitSingleItemToInventoryByIndex(int32 Index, UPARAM(ref)EItemType ItemType,
                                                            UPARAM(ref)int32& Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	if (Amount >= 1 && (TempItemBase->ItemType == EItemType::Nothing || TempItemBase->ItemType == ItemType))
	{
		int32 LeftAmount = GetLeftItemAmount(ItemType, 1, TempItemBase->Amount);
		if (LeftAmount == 0)
		{
			--Amount;
			++TempItemBase->Amount;
			TempItemBase->ItemType = ItemType;
		}
		if (Amount <= 0)
		{
			ItemType = EItemType::Nothing;
		}
	}
}

void UInventoryComponent::SwapItemByIndex(int32 Index, UPARAM(ref)EItemType& ItemType, UPARAM(ref)int32& Amount)
{
	UItemBase* TempItemBase = Inventory[Index];
	Swap(ItemType, TempItemBase->ItemType);
	Swap(Amount, TempItemBase->Amount);
}

int32 UInventoryComponent::GetLeftItemAmount(EItemType ItemType, int32 SourceAmount, int32 TargetAmount)
{
	if (ItemType == EItemType::Nothing)
		return 0;
	int32 MaxStackSize = GetMaxStackSizeByItemType(ItemType);
	return MaxStackSize - TargetAmount >= SourceAmount ? 0 : SourceAmount - (MaxStackSize - TargetAmount);
}

int32 UInventoryComponent::RemoveItemByIndex(int32 Index)
{
	int32 AmountToRemove = Inventory[Index]->Amount;
	Inventory[Index]->ItemType = EItemType::Nothing;
	Inventory[Index]->Amount = 0;
	return AmountToRemove;
}

int32 UInventoryComponent::RemoveHalfItemByIndex(int32 Index)
{
	int32& SlotItemAmount = Inventory[Index]->Amount;

	int32 AmountToRemove = SlotItemAmount == 1 ? 1 : SlotItemAmount / 2;
	SlotItemAmount -= AmountToRemove;
	if (SlotItemAmount == 0)
		Inventory[Index]->ItemType = EItemType::Nothing;
	return AmountToRemove;
}

void UInventoryComponent::SetItemByIndex(int32 Index, EItemType ItemType, int32 Amount)
{
	Inventory[Index]->ItemType = ItemType;
	Inventory[Index]->Amount = Amount;
}

int32 UInventoryComponent::CanAddItem(int32 Index, EItemType ItemType, int32 Amount)
{
	int32 CurMaxSize = GetMaxStackSizeByItemType(ItemType);
	if (Inventory[Index]->ItemType == EItemType::Nothing)
		return Amount;
	if (Inventory[Index]->ItemType != ItemType)
		return 0;
	return Amount <= CurMaxSize - Inventory[Index]->Amount ? Amount : CurMaxSize - Inventory[Index]->Amount;
}

void UInventoryComponent::BackUp()
{
	InventoryBackUp.SetNum(Inventory.Num());
	for (int i = 0; i < Inventory.Num(); i++)
	{
		InventoryBackUp[i] = NewObject<UItemBase>();
		InventoryBackUp[i]->Amount = Inventory[i]->Amount;
		InventoryBackUp[i]->ItemType = Inventory[i]->ItemType;
	}
}

int32 UInventoryComponent::AutoDiv(EItemType ItemType, int32 TotalAmount, TArray<int32> MovedSlotIndices)
{
	TArray<int32> CanPlaceSlotsIndices;
	for (auto i : MovedSlotIndices)
	{
		if (CanAddItem(i, ItemType, 1))
		{
			CanPlaceSlotsIndices.Add(i);
		}
	}

	if (CanPlaceSlotsIndices.Num() == 0)
		return TotalAmount;
	if (TotalAmount == 1)
	{
		Inventory[CanPlaceSlotsIndices[0]]->Amount = InventoryBackUp[CanPlaceSlotsIndices[0]]->Amount + 1;
		Inventory[CanPlaceSlotsIndices[0]]->ItemType = ItemType;
		return 0;
	}
	int32 TotalActualPlacedItems = 0;
	int32 AmountPerSlot = TotalAmount / CanPlaceSlotsIndices.Num();

	if (AmountPerSlot >= 1)
	{
		for (auto i : CanPlaceSlotsIndices)
		{
			int32 Left = GetLeftItemAmount(ItemType, AmountPerSlot, InventoryBackUp[i]->Amount);
			Inventory[i]->Amount = InventoryBackUp[i]->Amount + AmountPerSlot - Left;
			Inventory[i]->ItemType = ItemType;
			TotalActualPlacedItems += AmountPerSlot - Left;
		}
		return TotalAmount - TotalActualPlacedItems;
	}
	return 0;
}

void UInventoryComponent::GetItemStack(EItemType ItemType,  UPARAM(ref)int32& CurAmount)
{
	TArray<TPair<int32, int32>> ThisKindSlotIndices;
	for (int i = 0; i < InventorySize + ItemBarSize; i++)
	{
		if (Inventory[i]->ItemType == ItemType)
		{
			ThisKindSlotIndices.Add(TPair<int32, int32>(i, Inventory[i]->Amount));
		}
	}
	
	ThisKindSlotIndices.Sort(
		[](const TPair<int32, int32> X1, const TPair<int32, int32> X2)
		{
			return X1.Get<1>() <= X2.Get<1>();
		});

	for(auto i: ThisKindSlotIndices)
	{
		int32 LeftAmount = GetLeftItemAmount(ItemType, i.Get<1>(),CurAmount);
		CurAmount += Inventory[i.Get<0>()]->Amount - LeftAmount;
		Inventory[i.Get<0>()]->Amount = LeftAmount;	
		if(LeftAmount == 0)
		{
			Inventory[i.Get<0>()]->ItemType = EItemType::Nothing;
		}
	}
}