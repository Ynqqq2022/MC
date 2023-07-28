// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "SaveGamePlayer.h"
#include "TerrainGenerationComponent.h"
#include "GameFramework/PlayerController.h"
#include "MCPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MC_API AMCPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMCPlayerController();
	
	UFUNCTION(BlueprintCallable)
	void SaveGamePlayerInfo();
	virtual void PreInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTerrainGenerationComponent* TerrainGenerationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInventoryComponent* InventoryComponent;

	//从存档中加载的玩家位置、背包等数据。
	UPROPERTY()
	USaveGamePlayer *PlayerData;
};
