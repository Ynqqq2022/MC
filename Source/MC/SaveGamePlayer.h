// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "GameFramework/SaveGame.h"
#include "SaveGamePlayer.generated.h"

/**
 * 
 */
UCLASS()
class MC_API USaveGamePlayer : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FIntPoint PlayerChunkIndex;
	UPROPERTY()
	FTransform PlayerTransform;
	//角色的朝向受controller影响，也要保存。
	UPROPERTY()
	FRotator PlayerControllerRotator;
	UPROPERTY()
	int32 CurSelectItemBarIndex;
	UPROPERTY()
	TArray<FItemBase> Inventory;
};
