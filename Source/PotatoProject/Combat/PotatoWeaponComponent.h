#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PotatoWeaponComponent.generated.h"

class UPotatoWeaponData;
class APotatoWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POTATOPROJECT_API UPotatoWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPotatoWeaponComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TObjectPtr<UPotatoWeaponData>> WeaponSlots;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentWeaponIndex = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UPotatoWeaponData> CurrentWeaponData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<APotatoWeapon> CurrentWeaponActor;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire();
	
	FVector GetMuzzleLocation() const;
	
private:
	FVector GetCrosshairTargetLocation() const;
	
	/** 물리적인 투사체(감자, 옥수수, 호박)를 스폰하는 로직을 처리 */
	void FireProjectile(const FVector& TargetLocation);
	
	/** 히트스캔 로직(당근)을 처리 */
	void FireHitscan(const FVector& TargetLocation);
	
	/** 시각적 당근 Actor를 스폰 */
	void SpawnHitscanVisual(const FHitResult& HitResult, const FVector& ShotDirection);
	
	void SpawnWeapon(TSubclassOf<APotatoWeapon> NewClass);
};
