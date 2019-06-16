// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CyberProjectile.h"
#include "CProjectileKingTao.generated.h"

/**
 * 
 */
UCLASS()
class CYBER_API ACProjectileKingTao : public ACyberProjectile
{
	GENERATED_BODY()
	
	
public:
	void HomingMissile(AActor* TargetToHit);

};
