// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/PotatoProductionComponent.h"
#include "Core/PotatoResourceManager.h"

UPotatoProductionComponent::UPotatoProductionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPotatoProductionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		ResourceManager = World->GetSubsystem<UPotatoResourceManager>();
	}

	if (ResourceManager)
	{
		ResourceManager->RegisterProduction(
			ProductionPerMinuteWood,
			ProductionPerMinuteStone,
			ProductionPerMinuteCrop,
			ProductionPerMinuteLivestock
		);
	}
}

void UPotatoProductionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ResourceManager)
	{
		ResourceManager->UnregisterProduction(
			ProductionPerMinuteWood,
			ProductionPerMinuteStone,
			ProductionPerMinuteCrop,
			ProductionPerMinuteLivestock
		);
	}

	Super::EndPlay(EndPlayReason);
}

bool UPotatoProductionComponent::TryPurchase()
{
	if (!ResourceManager) return false;

	if (!ResourceManager->HasEnoughResource(EResourceType::Wood, BuyCostWood)) return false;
	if (!ResourceManager->HasEnoughResource(EResourceType::Stone, BuyCostStone)) return false;
	if (!ResourceManager->HasEnoughResource(EResourceType::Crop, BuyCostCrop)) return false;
	if (!ResourceManager->HasEnoughResource(EResourceType::Livestock, BuyCostLivestock)) return false;

	ResourceManager->RemoveResource(EResourceType::Wood, BuyCostWood);
	ResourceManager->RemoveResource(EResourceType::Stone, BuyCostStone);
	ResourceManager->RemoveResource(EResourceType::Crop, BuyCostCrop);
	ResourceManager->RemoveResource(EResourceType::Livestock, BuyCostLivestock);

    return true;
}

void UPotatoProductionComponent::Refund()
{
	if (!ResourceManager) return;

	if(RefundWood > 0) ResourceManager->AddResource(EResourceType::Wood, RefundWood);
	if(RefundStone > 0) ResourceManager->AddResource(EResourceType::Stone, RefundStone);
	if(RefundCrop > 0) ResourceManager->AddResource(EResourceType::Crop, RefundCrop);
	if(RefundLivestock > 0) ResourceManager->AddResource(EResourceType::Livestock, RefundLivestock);
}
