#include "PotatoWeapon.h"

APotatoWeapon::APotatoWeapon()
{
 	PrimaryActorTick.bCanEverTick = true;
}

void APotatoWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void APotatoWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APotatoWeapon::Fire()
{

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