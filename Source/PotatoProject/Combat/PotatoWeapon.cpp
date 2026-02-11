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
		//WeaponSystem->Fire();
	}

}

float FireAccTime = 0.0f; // 누적 시간 계산용
float FireInterval = 2.0f; // 2초마다 발사

void APotatoWeapon::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	//FireAccTime += DeltaTime;
	//
	//if (FireAccTime >= FireInterval)
	//{
	//	if (WeaponSystem) {
	//	
	//		WeaponSystem->EquipWeapon(0);
	//		UE_LOG(LogTemp, Log, TEXT("UPotatoWeaponSystem::Fire() 호출됨!"));
	//		WeaponSystem->Fire();

	//	}
	//	// 시간 초기화 (남은 시간을 유지하려면 FireAccTime -= FireInterval;)
	//	FireAccTime = 0.0f;
	//}
}


void APotatoWeapon::Fire()
{

		if (WeaponSystem)
		{
			FTransform WeaponTransform = GetActorTransform();
			FVector RelativeOffset = GetActorLocation().ForwardVector * -90.0f;
			FVector SpawnLocation = WeaponTransform.TransformPosition(RelativeOffset);
			FRotator SpawnRotation = FRotator::ZeroRotator;
			FActorSpawnParameters SpawnParams;
			APotatoProjectile* NewProjectile = GetWorld()->SpawnActor<APotatoProjectile>(
				ProjectileOrigin,
				SpawnLocation,
				SpawnRotation,
				SpawnParams
			);
			WeaponSystem->ProjectileLimit.Add(NewProjectile);
			FVector Direction = SpawnLocation - GetActorLocation();
			UE_LOG(LogTemp, Log, TEXT("Direction %s"), *Direction.ToString());
			NewProjectile->Launch(Direction);
			WeaponSystem->LimitBullets();
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

void APotatoWeapon::ChangeWeapon(int index)
{
	
}