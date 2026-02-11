#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "PotatoWeaponSystem.generated.h"

class APotatoWeapon;

class APotatoProjectile;

UCLASS(Blueprintable, BlueprintType)
class POTATOPROJECT_API UPotatoWeaponSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<TSubclassOf<APotatoWeapon>> WeaponSlots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int CurrentSlotIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<APotatoWeapon> CurrentWeapon;
	UPROPERTY()
	APotatoWeapon* CurrentWeaponInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<TSubclassOf<APotatoProjectile>> Projectiles;
	UPROPERTY()
	TArray<APotatoProjectile*> ProjectileLimit;

	void EquipWeapon(int SlotIndex);
	void Fire();
	void Reload();
	void SwitchWeapon(int SlotIndex);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
