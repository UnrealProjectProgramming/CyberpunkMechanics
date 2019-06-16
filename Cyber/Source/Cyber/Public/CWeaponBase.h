// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CWeaponBase.generated.h"

class AStaticMeshActor;
class UParticleSystem;


UCLASS()
class CYBER_API ACWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACWeaponBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<class ACyberProjectile> ProjectileClass;


	// Getters
	float GetZoomedFOV() { return ZoomedFOV; }
	float GetDefaultFOV() { return DefaultFOV; }
	float GetZoomInterpSpeed() { return ZoomInterpSpeed; }


	UFUNCTION()
	virtual void Fire();

	UFUNCTION()
	void StartFire();

	UFUNCTION()
	void StopFire();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomInterpSpeed;

	/* Number of bullets that will be shot in a minute  */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* MuzzleFlashEffect;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class UCameraShake> CameraShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	ACWeaponBase* CurrentWeapon;

	FTimerHandle TimerHandle_TimeBetweenShots;

	UPROPERTY()
	float TimeBetweenShots;

	UPROPERTY()
	float LastTimeFire;

	float DefaultFOV;

	class ACyberCharacter* MyPawn;

	class ACyberProjectile* SpawnedProjectile;

protected:
	void PlayCameraEffects();

private:
	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	
};
