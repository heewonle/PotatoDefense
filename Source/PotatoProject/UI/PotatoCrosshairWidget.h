#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PotatoCrosshairWidget.generated.h"

class UImage;
class APotatoPlayerCharacter;

UCLASS()
class POTATOPROJECT_API UPotatoCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* CrosshairTop;
	
	UPROPERTY(meta = (BindWidget))
	UImage* CrosshairBottom;
	
	UPROPERTY(meta = (BindWidget))
	UImage* CrosshairLeft;
	
	UPROPERTY(meta = (BindWidget))
	UImage* CrosshairRight;
	
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float BaseSpread = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float VelocitySpreadMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float FiringSpread = 50.0f;
	
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float SpreadInterpSpeed = 15.0f;
	
private:
	UPROPERTY();
	APotatoPlayerCharacter* PlayerCharacter;
	
	float CurrentSpread;
};
