#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "PotatoWeaponSystem.generated.h"

class APotatoWeapon;

class APotatoProjectile;

UCLASS()
class POTATOPROJECT_API UPotatoWeaponSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	TArray<APotatoWeapon*> WeaponSlots;
	int CurrentSlotIndex;
	APotatoWeapon* CurrentWeapon;

	TArray<APotatoProjectile*> Projectiles;

	void EquipWeapon(int SlotIndex);
	void Fire();
	void Reload();
	void SwitchWeapon(int SlotIndex);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
