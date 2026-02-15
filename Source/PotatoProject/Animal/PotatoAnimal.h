#pragma once

#include "CoreMinimal.h"
#include "../Core/PotatoEnums.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PotatoAnimal.generated.h"

class UPotatoProductionComponent;
class UBoxComponent;

UCLASS(Blueprintable)
class POTATOPROJECT_API APotatoAnimal : public ACharacter
{
	GENERATED_BODY()
	
public:	
	APotatoAnimal();

protected:
	virtual void BeginPlay() override;

public:	
	// 생산/경제 컴포넌트 (비용, 분당 생산량, 환급 모두 여기서 관리)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animal|Production")
	TObjectPtr<UPotatoProductionComponent> ProductionComp;

	// Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animal")
	EAnimalType AnimalType = EAnimalType::Cow;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal|Runtime")
	TObjectPtr<AActor> AssignedBarn = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal|Runtime")
	TObjectPtr<UBoxComponent> MovingArea = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Animal|Runtime")
	void InitializeWithBarn(AActor* InBarn, UBoxComponent* InBounds);
};
