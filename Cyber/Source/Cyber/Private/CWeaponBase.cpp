// Fill out your copyright notice in the Description page of Project Settings.

#include "CWeaponBase.h"

#include "Camera/CameraComponent.h"
#include "CyberCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "CyberProjectile.h"

// Sets default values
ACWeaponBase::ACWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));


	// Auto Fire Init
	RateOfFire = 550.0f;

	ZoomedFOV = 55.0f;
	ZoomInterpSpeed = 15.0f;

}

// Called when the game starts or when spawned
void ACWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentWeapon = this;
	auto Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	MyPawn = Cast<ACyberCharacter>(Pawn);
	if (MyPawn)
	{
		//FP_Gun->AttachToComponent(MyPawn->GetMesh1P(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
		TimeBetweenShots = 60 / RateOfFire;
		DefaultFOV = MyPawn->GetFirstPersonCameraComponent()->FieldOfView;
	}

}

// Called every frame
void ACWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACWeaponBase::Fire()
{
	bool bFailedAtProjectileSpawning = false;
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{

			const FRotator SpawnRotation = MyPawn->GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			if (!MyPawn) { return; }
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(MyPawn->GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;


			// spawn the projectile at the muzzle
			auto SpawnedProjectile = World->SpawnActor<ACyberProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			LastTimeFire = GetWorld()->TimeSeconds;

			//TODO@: Move this to child class 
			/*if (SpawnedProjectile)
			{
				SpawnedProjectile->HomingMissile(HomingTarget);
				bFailedAtProjectileSpawning = false;
			}
			else
			{
				bFailedAtProjectileSpawning = true;
			}*/
			PlayCameraEffects();
		}
	}

	if (!bFailedAtProjectileSpawning)
	{
		// try and play the sound if specified
		if (FireSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != NULL)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = MyPawn->GetMesh1P()->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}

		if (MuzzleFlashEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, FP_Gun, "Muzzle");
		}

	}
}

void ACWeaponBase::StartFire()
{
	float FirstDelay = FMath::Max(LastTimeFire + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACWeaponBase::Fire, TimeBetweenShots, true, FirstDelay);
}

void ACWeaponBase::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}


void ACWeaponBase::PlayCameraEffects()
{
	APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
	if (PC)
	{
		PC->ClientPlayCameraShake(CameraShakeClass);
	}
}
