#include "PotatoProjectile.h"

APotatoProjectile::APotatoProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

}

void APotatoProjectile::BeginPlay()
{
	Super::BeginPlay();
	Mesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
}

void APotatoProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APotatoProjectile::Launch(FVector Direction)
{

	if (Mesh)
	{
		//FVector LaunchDirection = GetActorLocation() - GetActorLocation().RightVector;
		Direction.Normalize();
		Mesh->AddImpulse(Direction * Speed , NAME_None, true);
	}
}

void APotatoProjectile::OnHit(AActor* HitActor)
{

}

void APotatoProjectile::Explode()
{

}