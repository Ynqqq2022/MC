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
	UPROPERTY()
	int32 CurSelectItemBarIndex;
	UPROPERTY()
	TArray<FItemBase> Inventory;
};
