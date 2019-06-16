// Fill out your copyright notice in the Description page of Project Settings.

#include "CWeaponKangTao.h"

#include "CyberProjectile.h"
#include "CProjectileKingTao.h"



void ACWeaponKangTao::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void ACWeaponKangTao::Fire()
{
	Super::Fire();
	ACProjectileKingTao* ProjectileKingTao = Cast<ACProjectileKingTao>(SpawnedProjectile);
	if (ProjectileKingTao)
	{
		ProjectileKingTao->HomingMissile(HomingTarget);
	}
}
