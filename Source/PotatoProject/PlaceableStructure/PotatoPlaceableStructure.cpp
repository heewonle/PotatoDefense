#include "PotatoPlaceableStructure.h"
#include "PotatoStructureData.h"

APotatoPlaceableStructure::APotatoPlaceableStructure()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void APotatoPlaceableStructure::BeginPlay()
{
	Super::BeginPlay();
	
	if (StructureData)
	{
		if (StructureData->bIsDestructible)
		{
			CurrentHealth = StructureData->MaxHealth;
		}
		else
		{
			CurrentHealth = -1.0f; // 파괴 불가능
		}
		
		UE_LOG(LogTemp, Warning, TEXT("%s Initialized!"), *StructureData->DisplayName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Structure 객체가 DataAsset 없이 스폰됐습니다!"));
	}
}

void APotatoPlaceableStructure::Interact(AActor* Interactor)
{
	if (IsDestructible() && CurrentHealth <= 0.0f)
	{
		return;
	}
	
	if (OnInteractedDelegate.IsBound())
	{
		OnInteractedDelegate.Broadcast(Interactor);
	}
}

bool APotatoPlaceableStructure::IsDestructible() const
{
	if (StructureData)
	{
		return StructureData->bIsDestructible;
	}

	return false;
}

void APotatoPlaceableStructure::TakeDamage(float Amount)
{
	if (!IsDestructible() || CurrentHealth <= 0.0f)
	{
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, StructureData->MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		// 파괴 처리: FX 재생 등
		// OnStructureDestroyed.Broadcast();
		Destroy();
	}
}
