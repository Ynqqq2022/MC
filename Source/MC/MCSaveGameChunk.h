// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BlockDataStructs.h"
#include "MCSaveGameChunk.generated.h"

/**
 * 
 */
UCLASS()
class MC_API UMCSaveGameChunk : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TMap<int32, EBlockType> ChangedBlocks;
};
