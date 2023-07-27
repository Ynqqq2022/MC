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

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameMenuClass;

	UFUNCTION(BlueprintCallable)
	void ShowInventory() const;
	UFUNCTION(BlueprintCallable)
	void HideInventory() const;
	UFUNCTION(BlueprintCallable)
	void ShowItemBar() const;
	UFUNCTION(BlueprintCallable)
	void HideItemBar() const;
	UFUNCTION(BlueprintCallable)
	void ToggleInventoryPanel() ;
	UFUNCTION(BlueprintCallable)
	void ShowGameMenu() const;
	UFUNCTION(BlueprintCallable)
	void HideGameMenu()const;
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* CrosshairWidget;

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* InvectoryPanelWidget;

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* ItemBarWidget;

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* GameMenuWidget;

private:
	bool bShowInventoryPanel;
};