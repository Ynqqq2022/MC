// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MCHUD.generated.h"

class UInventoryPanel;
/**
 * 
 */
UCLASS()
class MC_API AMCHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InvectoryPanelClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ItemBarClass;
	
	UFUNCTION(BlueprintCallable)
	void ShowInventory() const;
	UFUNCTION(BlueprintCallable)
	void HideInventory() const;
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* CrosshairWidget;

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* InvectoryPanelWidget;

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* ItemBarWidget;
};
