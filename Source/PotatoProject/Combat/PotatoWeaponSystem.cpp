#include "PotatoWeaponSystem.h"
#include "PotatoWeapon.h"
#include "PotatoProjectile.h"

void UPotatoWeaponSystem::EquipWeapon(int SlotIndex)
{
	CurrentWeapon = WeaponSlots[SlotIndex];
	Fire();
}
void UPotatoWeaponSystem::Fire()
{
	//10개까지만 생성하고 가장 오래된 객체 삭제
	if (Projectiles.Num() >= 10)
	{
		APotatoProjectile* OldestProjectile = Projectiles[0];

		if (IsValid(OldestProjectile))
		{
			OldestProjectile->Destroy();
		}

		Projectiles.RemoveAt(0);
	}
	CurrentWeapon->Fire();
}

void UPotatoWeaponSystem::Reload()
{
	CurrentWeapon->Reload();
}

void UPotatoWeaponSystem::SwitchWeapon(int SlotIndex)
{
	CurrentWeapon = WeaponSlots[SlotIndex];
}

void UPotatoWeaponSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// BindSomething();
}

void UPotatoWeaponSystem::Deinitialize()
{
	// UnbindSomething();
	Super::Deinitialize();
}