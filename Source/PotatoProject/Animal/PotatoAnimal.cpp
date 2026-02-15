#include "PotatoAnimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Core/PotatoProductionComponent.h"

APotatoAnimal::APotatoAnimal()
{
	PrimaryActorTick.bCanEverTick = false;

	// AutoPossess 비활성 (InitializeWithBarn 후 수동 Possess)
	AutoPossessAI = EAutoPossessAI::Disabled;

	// 콜리전: 월드는 Block, Pawn(플레이어/동물/몬스터)만 Overlap
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToAllChannels(ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	// 이동 속도
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 100.0f;

        MoveComp->bUseRVOAvoidance = true;
        MoveComp->AvoidanceWeight = 0.5f;
        MoveComp->AvoidanceConsiderationRadius = 100.f;
	}

	// 생산 컴포넌트
	ProductionComp = CreateDefaultSubobject<UPotatoProductionComponent>(TEXT("ProductionComp"));
}

void APotatoAnimal::BeginPlay()
{
	Super::BeginPlay();
}

void APotatoAnimal::InitializeWithBarn(AActor* InBarn, UBoxComponent* InBounds)
{
	AssignedBarn = InBarn;
	MovingArea = InBounds;

	//데이터 주입 완료 후 수동으로 AIController 부착
	SpawnDefaultController();
}
