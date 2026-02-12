#include "BuildingSystemComponent.h"
#include "PotatoStructureData.h"
#include "PotatoPlaceableStructure.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

UBuildingSystemComponent::UBuildingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBuildingSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupInputBindings();
}

void UBuildingSystemComponent::SetupInputBindings()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	
	if (!PlayerController)
	{
		return;
	}
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent))
	{

		if (PlaceStructureAction)
		{
			EnhancedInput->BindAction(PlaceStructureAction, ETriggerEvent::Started, this, &UBuildingSystemComponent::OnPlaceStructure);
		}

		if (RotateStructureAction)
		{
			EnhancedInput->BindAction(RotateStructureAction, ETriggerEvent::Triggered, this, &UBuildingSystemComponent::OnRotateStructure);
		}
		
		if (CycleStructureAction)
		{
			EnhancedInput->BindAction(CycleStructureAction, ETriggerEvent::Started, this, &UBuildingSystemComponent::OnCycleStructure);
		}
	}
}

void UBuildingSystemComponent::ToggleBuildMode()
{
	bIsBuildMode = !bIsBuildMode;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController)
	{
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}
	
	if (bIsBuildMode)
	{
		if (BuildIMC)
		{
			Subsystem->AddMappingContext(BuildIMC, 1);
			SetComponentTickEnabled(true);
			CurrentRotationIndex = 0;
			RefreshGhostActorModel();
			
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Build Mode: ON"));
		}
	}
	else
	{
		if (BuildIMC)
		{
			Subsystem->RemoveMappingContext(BuildIMC);
			SetComponentTickEnabled(false);
			
			if (GhostActor)
			{
				GhostActor->Destroy();
				GhostActor = nullptr;
			}
		}
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Build Mode: OFF"));
	}
}

void UBuildingSystemComponent::OnPlaceStructure(const FInputActionValue& Value)
{
}

void UBuildingSystemComponent::OnRotateStructure(const FInputActionValue& Value)
{
}

void UBuildingSystemComponent::OnCycleStructure(const FInputActionValue& Value)
{
}

FVector UBuildingSystemComponent::CalculateSnappedLocation(const FVector& HitLocation) const
{
	return FVector::ZeroVector;
}

void UBuildingSystemComponent::UpdateGhostActorTransform()
{
}

void UBuildingSystemComponent::RefreshGhostActorModel()
{
}

void UBuildingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsBuildMode && GhostActor)
	{
		UpdateGhostActorTransform();
	}

}
