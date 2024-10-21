// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority())
	{
		return;
	}

	const USkeletalMeshSocket* MuzzleFlashSocket{GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"))};
	APawn* InstigatorPawn{Cast<APawn>(GetOwner())};
	UWorld* World{GetWorld()};

#pragma region Nullchecks
	if (!MuzzleFlashSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|MuzzleFlashSocket is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|ProjectileClass is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|World is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!InstigatorPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|InstigatorPawn is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	const FTransform SocketTransform{MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh())};

	// From muzzle flash socket to hit location from TraceUnderCrosshairs
	const FVector ToTarget{HitTarget - SocketTransform.GetLocation()};
	const FRotator TargetRotation{ToTarget.Rotation()};

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = InstigatorPawn;

	World->SpawnActor<AProjectile>(
		ProjectileClass,
		SocketTransform.GetLocation(),
		TargetRotation,
		SpawnParams
	);
}
