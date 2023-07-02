// Fill out your copyright notice in the Description page of Project Settings.


#include "MCHUD.h"

#include "Blueprint/UserWidget.h"

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
	}

	if(ItemBarClass)
	{
		ItemBarWidget = CreateWidget<UUserWidget>(GetWorld(), ItemBarClass);
		ItemBarWidget->AddToViewport(-1);
		ItemBarWidget->SetVisibility(ESlateVisibility::Visible);
	}
}


void AMCHUD::ShowInventory() const
{
	if(InvectoryPanelWidget)
	{
		InvectoryPanelWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMCHUD::HideInventory() const
{
	if(InvectoryPanelWidget)
	{
		InvectoryPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	}	
}