#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PotatoAuraDamageComponent.generated.h"

class USphereComponent;
class UDamageType;

UCLASS(ClassGroup=(Potato), meta=(BlueprintSpawnableComponent))
class POTATOPROJECT_API UPotatoAuraDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPotatoAuraDamageComponent();

	// 런타임 설정 (스폰 시/프리셋 적용 후 호출 추천)
	UFUNCTION(BlueprintCallable, Category="Potato|AuraDamage")
	void Configure(float InRadius, float InDps, float InTickInterval);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TickAura();
	bool IsValidTarget(AActor* A) const;

public:
	// ===== Defaults (에디터에서 선인장만 튜닝 가능하게) =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Potato|AuraDamage", meta=(ClampMin="0.0"))
	float Radius = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Potato|AuraDamage", meta=(ClampMin="0.0"))
	float Dps = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Potato|AuraDamage", meta=(ClampMin="0.01"))
	float TickInterval = 0.25f;

	// 특정 Actor Tag가 있는 대상만 (비우면 전체 허용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Potato|AuraDamage")
	FName RequiredTargetTag = NAME_None;

	// 적용할 DamageType
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Potato|AuraDamage")
	TSubclassOf<UDamageType> DamageTypeClass;

private:
	UPROPERTY(Transient)
	AActor* CachedOwner = nullptr;

	UPROPERTY(Transient)
	USphereComponent* Sphere = nullptr;

	FTimerHandle AuraTickTH;

	// 오버랩 대상들(약참조)
	TSet<TWeakObjectPtr<AActor>> OverlappingTargets;

	// 이 컴포넌트 전용 Sphere 이름 (기존 Sphere와 충돌 방지)
	static const FName AuraSphereName;
	
	TMap<TWeakObjectPtr<AActor>, FHitResult> LastSweepHitByTarget;
};