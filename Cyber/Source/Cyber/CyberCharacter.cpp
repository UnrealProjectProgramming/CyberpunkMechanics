// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CyberCharacter.h"
#include "CyberProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Private/KismetTraceUtils.h"
#include "Perception/AIPerceptionComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACyberCharacter

ACyberCharacter::ACyberCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

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

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Auto Fire Init
	RateOfFire = 550.0f;

	ZoomedFOV = 55.0f;
	ZoomInterpSpeed = 15.0f;
}

void ACyberCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh1P->SetHiddenInGame(false, true);

	TimeBetweenShots = 60 / RateOfFire;
	DefaultFOV = FirstPersonCameraComponent->FieldOfView;

}

void ACyberCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(FirstPersonCameraComponent->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	
	FirstPersonCameraComponent->SetFieldOfView(NewFOV);

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACyberCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACyberCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACyberCharacter::StopFire);


	// Bind Targeting Event
	PlayerInputComponent->BindAction("Target", IE_Pressed, this, &ACyberCharacter::AssignTarget);

	// Bind Zooming
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ACyberCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ACyberCharacter::EndZoom);


	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACyberCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACyberCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACyberCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACyberCharacter::LookUpAtRate);
}

void ACyberCharacter::OnFire()
{
	bool bFailedAtProjectileSpawning = false;
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{

			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			

			// spawn the projectile at the muzzle
			auto SpawnedProjectile = World->SpawnActor<ACyberProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			LastTimeFire = GetWorld()->TimeSeconds;

			if (SpawnedProjectile)
			{
				SpawnedProjectile->HomingMissile(HomingTarget);
				bFailedAtProjectileSpawning = false;
			}
			else
			{
				bFailedAtProjectileSpawning = true;
			}
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
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
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


void ACyberCharacter::StartFire()
{
	float FirstDelay = FMath::Max(LastTimeFire + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACyberCharacter::OnFire, TimeBetweenShots, true, FirstDelay);
}


void ACyberCharacter::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}


void ACyberCharacter::BeginZoom()
{
	bWantsToZoom = true;
}


void ACyberCharacter::EndZoom()
{
	bWantsToZoom = false;
}


void ACyberCharacter::AssignTarget()
{
	FHitResult Hit;
	FVector StartDirection = FirstPersonCameraComponent->GetComponentLocation();
	FVector CameraForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector EndDirection = CameraForwardVector * 5000.0f + StartDirection;

	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	bool bSuccessfulHit = GetWorld()->LineTraceSingleByObjectType(Hit, StartDirection, EndDirection, QueryParams);

	if (bSuccessfulHit)
	{
		HomingTarget = Cast<AStaticMeshActor>(Hit.Actor);
		UE_LOG(LogTemp, Warning, TEXT("Successful Hit: %s"), *HomingTarget->GetName());
	}

	DrawDebugLine(GetWorld(), StartDirection, EndDirection, FColor::Red, true, 5.0f);
}


void ACyberCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void ACyberCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}


void ACyberCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void ACyberCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACyberCharacter::PlayCameraEffects()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->ClientPlayCameraShake(CameraShakeClass);
	}
}
