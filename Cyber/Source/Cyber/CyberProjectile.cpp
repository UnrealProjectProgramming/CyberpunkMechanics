// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CyberProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

ACyberProjectile::ACyberProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ACyberProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 0.0f;
}


void ACyberProjectile::HomingMissile(AStaticMeshActor* TargetToHit)
{
	if (TargetToHit && ProjectileMovement)
	{
		auto HittingTarget = TargetToHit->GetStaticMeshComponent();
		if (HittingTarget == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Null Hitting target"));
			Destroy();
			return;
		}

		ProjectileMovement->HomingTargetComponent = HittingTarget;
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingAccelerationMagnitude = FMath::RandRange(15000.0f, 25000.0f);
	
		UE_LOG(LogTemp, Warning, TEXT("HomingTargetComp or ProjectileMovment are Nullptr"));
		

		// To allow for more random moving up and down when we shoot the projectile.
		FVector NewVelocity = FVector(ProjectileMovement->Velocity.X,
									  FMath::RandRange(-300.0f, 300.0f),
									  FMath::RandRange(-300.0f, 300.0f));

		ProjectileMovement->SetVelocityInLocalSpace(NewVelocity);

	}
	PlayParticleEffects();
}


void ACyberProjectile::PlayParticleEffects()
{
	/* Spawn Particle if avialbe*/
	if (ProjectileTrail && RootComponent)
	{
		UGameplayStatics::SpawnEmitterAttached(ProjectileTrail, RootComponent);
	}
}


void ACyberProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 10.0f, GetActorLocation());		
		if (HitEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(HitEffect, RootComponent);
		}
		SetLifeSpan(2.1f);
	}
}

