// Fill out your copyright notice in the Description page of Project Settings.


#include "MCPlayerController.h"

#include "SaveGamePlayer.h"
#include "Kismet/GameplayStatics.h"

AMCPlayerController::AMCPlayerController()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	TerrainGenerationComponent = CreateDefaultSubobject<UTerrainGenerationComponent>(TEXT("TerrainGenerationComponent"));
}

//保存玩家数据到存档
void AMCPlayerController::SaveGamePlayerInfo()
{
	FString CurSlotName{"PlayInfo"};
	USaveGamePlayer* CurSave =  NewObject<USaveGamePlayer>();
	CurSave->PlayerChunkIndex = TerrainGenerationComponent->PlayerChunkIndex;
	CurSave->PlayerTransform = GetPawn()->GetActorTransform();
	CurSave->CurSelectItemBarIndex = InventoryComponent->CurSelectItemBarIndex;
	CurSave->Inventory = InventoryComponent->Inventory;
	UGameplayStatics::SaveGameToSlot(CurSave, CurSlotName, 0);
}

void AMCPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	//读取存档
	FString CurSlotName{"PlayInfo"};
	if (UGameplayStatics::DoesSaveGameExist(CurSlotName, 0))
	{
		PlayerData = Cast<USaveGamePlayer>(UGameplayStatics::LoadGameFromSlot(CurSlotName, 0));
	}
}

void AMCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if(PlayerData)
	{
		GetPawn()->SetActorTransform(PlayerData->PlayerTransform);
	}
}
