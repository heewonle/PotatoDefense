#include "PotatoWeaponSystem.h"
#include "PotatoWeapon.h"
#include "PotatoProjectile.h"

void UPotatoWeaponSystem::EquipWeapon(int SlotIndex)
{
	if (CurrentWeapon)
	{
		CurrentWeapon = WeaponSlots[SlotIndex];
		// 1. 기존 무기가 있다면 파괴
		if (CurrentWeaponInstance) CurrentWeaponInstance->Destroy();

		// 2. 설계도를 바탕으로 실제 무기 소환
		FActorSpawnParameters Params;
		CurrentWeaponInstance = GetWorld()->SpawnActor<APotatoWeapon>(CurrentWeapon, Params);
	}
}
void UPotatoWeaponSystem::Fire()
{
	//10개까지만 생성하고 가장 오래된 객체 삭제
	if (ProjectileLimit.Num() >= 10)
	{
		if (ProjectileLimit.IsValidIndex(0)) {
			ProjectileLimit.RemoveAt(0);
		}
		//Projectiles.RemoveAt(0);
	}
	if (IsValid(CurrentWeaponInstance))
	{
		UE_LOG(LogTemp, Log, TEXT("fire2"));
		
		CurrentWeaponInstance->Fire();
	}
}

void UPotatoWeaponSystem::Reload()
{
	CurrentWeaponInstance->Reload();
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