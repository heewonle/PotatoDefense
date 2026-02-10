#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "PotatoWeaponSystem.generated.h"

class APotatoWeapon;

UCLASS()
class POTATOPROJECT_API UPotatoWeaponSystem : public USubsystem
{
	GENERATED_BODY()
public:
	TArray<APotatoWeapon*> WeaponSlots;
	int CurrentSlotIndex;
	APotatoWeapon* CurrentWeapon;

	void EquipWeapon(int SlotIndex);
	void Fire();
	void Reload();
	void SwitchWeapon(int SlotIndex);
};
