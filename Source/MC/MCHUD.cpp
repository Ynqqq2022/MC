// Fill out your copyright notice in the Description page of Project Settings.


#include "MCHUD.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMCHUD::BeginPlay()
{
	Super::BeginPlay();

	if(CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);
		CrosshairWidget->AddToViewport(-1);
		CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if(InvectoryPanelClass)
	{
		InvectoryPanelWidget = CreateWidget<UUserWidget>(GetWorld(), InvectoryPanelClass);
		InvectoryPanelWidget->AddToViewport(-1);
		InvectoryPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
		bShowInventoryPanel = false;
	}

	if(ItemBarClass)
	{
		ItemBarWidget = CreateWidget<UUserWidget>(GetWorld(), ItemBarClass);
		ItemBarWidget->AddToViewport(-1);
		ItemBarWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if(GameMenuClass)
	{
		GameMenuWidget = CreateWidget<UUserWidget>(GetWorld(), GameMenuClass);
		GameMenuWidget->AddToViewport(-1);
		GameMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AMCHUD::ShowInventory() const
{
	if(InvectoryPanelWidget)
	{
		InvectoryPanelWidget->SetVisibility(ESlateVisibility::Visible);
		const FInputModeUIOnly InputMode;
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->SetShowMouseCursor(true);
		InvectoryPanelWidget->bIsFocusable = true;
		InvectoryPanelWidget->SetFocus();
	}
}

void AMCHUD::HideInventory() const
{
	if(InvectoryPanelWidget)
	{
		InvectoryPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
		const FInputModeGameOnly InputMode;
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->SetShowMouseCursor(false);
	}	
}

void AMCHUD::ShowItemBar() const
{
	if(ItemBarWidget)
	{
		ItemBarWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMCHUD::HideItemBar() const
{
	if(ItemBarWidget)
	{
		ItemBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AMCHUD::ToggleInventoryPanel() 
{
	if(bShowInventoryPanel)
	{
		HideInventory();
		ShowItemBar();
	}
	else
	{
		ShowInventory();
		HideItemBar();
	}
	bShowInventoryPanel = !bShowInventoryPanel;
}

void AMCHUD::ShowGameMenu() const
{
	if(GameMenuWidget)
	{
		GameMenuWidget->SetVisibility(ESlateVisibility::Visible);
		const FInputModeUIOnly InputMode;
		
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->SetShowMouseCursor(true);
		GameMenuWidget->bIsFocusable = true;
		GameMenuWidget->SetFocus();
		UGameplayStatics::SetGamePaused(GetWorld(),true);
	}
}

void AMCHUD::HideGameMenu() const
{
	if(GameMenuWidget)
	{
		GameMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		const FInputModeGameOnly InputMode;
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->SetShowMouseCursor(false);
		UGameplayStatics::SetGamePaused(GetWorld(),false);
	}	
}