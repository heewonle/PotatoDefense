#include "PotatoWeapon.h"
#include "PotatoProjectile.h"
#include "PotatoWeaponSystem.h"

APotatoWeapon::APotatoWeapon()
{
 	PrimaryActorTick.bCanEverTick = true;
}

void APotatoWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GI = GetGameInstance())
	{

		WeaponSystem = GI->GetSubsystem<UPotatoWeaponSystem>();
	}

}

void APotatoWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APotatoWeapon::Fire()
{
	
	
		if (WeaponSystem)
		{
			APotatoProjectile* newProjectile = DuplicateObject<APotatoProjectile>(Projectile, this);
			WeaponSystem->Projectiles.Add(newProjectile);
		}
	
	
	
}

bool APotatoWeapon::Reload()
{
	if (MagazineSize == CurrentAmmo)
	{
		return false;
	}
	else {
		CurrentAmmo = MagazineSize;
		return true;
	}
}

bool APotatoWeapon::CanFire()
{
	return false;
}