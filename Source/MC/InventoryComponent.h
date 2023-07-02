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

	//背包栏
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UItemBase*> Inventory;

	//下方物品栏
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UItemBase*> ItemBar;

	UFUNCTION(BlueprintCallable)
	FItemAssetData GetAssetDataByItemType(EItemType ItemType);
	
	UFUNCTION(BlueprintCallable)
	EItemType ConvertBlockTypeToItemType(EBlockType BlockType);
	
	int32 AddItemToContainer(TArray<UItemBase*> & Container,int32 ContainerSize, EItemType ItemType , int32 Amount);
	
	//为下方物品栏添加物品，返回未添加的数量。
	UFUNCTION(BlueprintCallable)
	int32 AddItemToItemBar(EItemType ItemType , int32 Amount);
	//为背包栏添加物品，返回未添加的数量。
	UFUNCTION(BlueprintCallable)
	int32 AddItemToInventory(EItemType ItemType , int32 Amount);

private:
	//物品数据表
	//UPROPERTY(EditAnywhere,meta=(RequiredAssetDataTags = "RowStructure=ItemData"))
	UPROPERTY(EditAnywhere,meta=(RequiredAssetDataTags = "RowStructure=ItemData"))
	UDataTable* ItemDataTable;
};