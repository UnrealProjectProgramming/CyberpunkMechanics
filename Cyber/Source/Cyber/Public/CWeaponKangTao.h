// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CWeaponBase.h"
#include "CWeaponKangTao.generated.h"

/**
 * 
 */
UCLASS()
class CYBER_API ACWeaponKangTao : public ACWeaponBase
{
	GENERATED_BODY()
	
public:
	virtual void Fire() override;
	
	virtual void Tick(float DeltaTime) override;

	// Implemented In Blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character")
	class AActor* HomingTarget;

	
};
