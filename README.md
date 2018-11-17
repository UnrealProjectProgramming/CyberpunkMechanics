## CyberpunkMechanics
### Hello Guys ! I will be making some of cyberpunk 2077 mechanics, feel free to use my code in your game.

![Alt Text](https://i.imgur.com/zG5kMuY.png)


## For now I did the the KANG TAO TYPE 41  [Video](https://youtu.be/-Lkt91LC2NI) ( auto bullet travel to target weapon ) and I am willing to create the bullet deflection in the wall and some other mechanics in my free time ;) 


## I am using C++ and blueprint in these mechanics ceration but mostly C++

## Here is a Snippit from the code, check the Commit if you wanna see the full code ;)

```cpp
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
```
