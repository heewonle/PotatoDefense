// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NA_NPC.generated.h"

/**
 * 
 */
UCLASS()
class POTATOPROJECT_API UNA_NPC : public UNavArea
{
	GENERATED_BODY()
public:
	UNA_NPC() 
	{
		DefaultCost = 1.f;
		FixedAreaEnteringCost = 0.f;
		DrawColor = FColor::Blue; // 디버그용(선택)
	}
	
};
