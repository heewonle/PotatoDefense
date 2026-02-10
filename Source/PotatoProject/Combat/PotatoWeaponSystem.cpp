#include "PotatoWeaponSystem.h"
#include "PotatoWeapon.h"

void UPotatoWeaponSystem::EquipWeapon(int SlotIndex)
{
	CurrentWeapon = WeaponSlots[SlotIndex];
}
void UPotatoWeaponSystem::Fire()
{
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