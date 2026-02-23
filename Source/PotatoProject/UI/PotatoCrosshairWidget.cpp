// Fill out your copyright notice in the Description page of Project Settings.


#include "PotatoCrosshairWidget.h"
#include "Player/PotatoPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Combat/PotatoWeaponComponent.h"
#include "Components/Image.h"

void UPotatoCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerCharacter = Cast<APotatoPlayerCharacter>(GetOwningPlayerPawn());
	CurrentSpread = BaseSpread;
}

void UPotatoCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!PlayerCharacter)
	{
		return;
	}

	float TargetSpread = BaseSpread;

	float PlayerSpeed = PlayerCharacter->GetVelocity().Size();

	// 2. 이동 시 Spread
	float VelocityFactor = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 600.0f), FVector2D(0.0f, 50.0f),
	                                                         PlayerSpeed);
	TargetSpread += VelocityFactor * VelocitySpreadMultiplier;
	
	// 3. 점프 시 Spread
	if (PlayerCharacter->GetCharacterMovement()->IsFalling())
	{
		TargetSpread += 10.0f;
	}
	
	// 4. 발사 시 Spread
	UPotatoWeaponComponent* WeaponComponent = PlayerCharacter->FindComponentByClass<UPotatoWeaponComponent>();
	if (WeaponComponent)
	{
		float TimeSinceFire = GetWorld()->GetTimeSeconds() - WeaponComponent->GetLastFireTime();
		
		const float FiringSpreadDuration = 0.15;
		if (TimeSinceFire < FiringSpreadDuration)
		{
			TargetSpread += FiringSpread;
		}
	}
	
	// 5. 보간
	CurrentSpread = FMath::FInterpTo(CurrentSpread, TargetSpread, InDeltaTime, SpreadInterpSpeed);
	
	// 6. 위젯 위치 업데이트
	if (CrosshairTop)
	{
		CrosshairTop->SetRenderTranslation(FVector2D(0.0f, -CurrentSpread));
		CrosshairBottom->SetRenderTranslation(FVector2D(0.0f, CurrentSpread));
		CrosshairLeft->SetRenderTranslation(FVector2D(-CurrentSpread, 0.0f));
		CrosshairRight->SetRenderTranslation(FVector2D(CurrentSpread, 0.0f));
	}
	
	
}
