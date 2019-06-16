// Fill out your copyright notice in the Description page of Project Settings.

#include "CProjectileKingTao.h"

#include "Engine/StaticMeshActor.h"
#include "GameFramework/ProjectileMovementComponent.h"

void ACProjectileKingTao::HomingMissile(AActor* TargetToHit)
{
	if (TargetToHit)
	{
		auto HittingTarget = TargetToHit;
		if (HittingTarget == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Null Hitting target"));
			Destroy();
			return;
		}
		
		GetProjectileMovementComp()->HomingTargetComponent = Cast<USceneComponent>(HittingTarget);
		GetProjectileMovementComp()->bIsHomingProjectile = true;
		GetProjectileMovementComp()->HomingAccelerationMagnitude = FMath::RandRange(15000.0f, 25000.0f);

		UE_LOG(LogTemp, Warning, TEXT("TargetToHit is: %s"), *TargetToHit->GetName());


		// To allow for more random moving up and down when we shoot the projectile.
		FVector NewVelocity = FVector(GetProjectileMovementComp()->Velocity.X,
			FMath::RandRange(-300.0f, 300.0f),
			FMath::RandRange(-300.0f, 300.0f));

		GetProjectileMovementComp()->SetVelocityInLocalSpace(NewVelocity);

	}

}
