// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bAiming = false;
	bFireButtonPressed = false;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!Character->GetFollowCamera())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character->GetFollowCamera() is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	DefaultFOV = Character->GetFollowCamera()->FieldOfView;
	CurrentFOV = DefaultFOV;

	// We do this only in autonomous proxy
	if (Character->IsLocallyControlled())
	{
#pragma region Nullchecks
		if (!Character->Controller)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|Character->Controller is nullptr"), *FString(__FUNCTION__))
			return;
		}
#pragma endregion
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
		HUD = Cast<ABlasterHUD>(Controller->GetHUD());
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	// We do this only in autonomous proxy
	if (Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!WeaponToEquip)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket{Character->GetMesh()->GetSocketByName(FName("RightHandSocket"))};

#pragma region Nullchecks
	if (!HandSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|HandSocket is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetAiming(const bool bIsAiming)
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::ServerSetAiming_Implementation(const bool bIsAiming)
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	bAiming = bIsAiming;

	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::OnRep_EquippedWeapon() const
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket{Character->GetMesh()->GetSocketByName(FName("RightHandSocket"))};

#pragma region Nullchecks
		if (!HandSocket)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|HandSocket is nullptr"), *FString(__FUNCTION__))
			return;
		}
#pragma endregion

		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		bCanFire = false;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireButtonPressed(const bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (EquippedWeapon && bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!EquippedWeapon)
	{
		return;
	}

	Character->PlayFireMontage(bAiming);
	EquippedWeapon->Fire(TraceHitTarget);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;

#pragma region Nullchecks
	if (!GEngine)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|GEngine is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!GEngine->GameViewport)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|GEngine->GameViewport is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	GEngine->GameViewport->GetViewportSize(ViewportSize);

	const FVector2D CrosshairLocation(ViewportSize / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start{CrosshairWorldPosition};

		const float DistanceToCharacter{static_cast<float>((Character->GetActorLocation() - Start).Size())};
		Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);

		const FVector End{Start + CrosshairWorldDirection * TRACE_LENGTH};

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);

		bOnTarget = TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>();
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
#pragma region Nullchecks
	// Added because sometimes HUD didn't set in BeginPlay
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!IsValid(Controller))
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
	}
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Controller is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!IsValid(HUD))
	{
		HUD = Cast<ABlasterHUD>(Controller->GetHUD());
	}
	//
	if (!HUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|HUD is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (EquippedWeapon)
	{
		HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
		HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
		HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
		HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
	}
	else
	{
		HUDPackage.CrosshairsCenter = nullptr;
		HUDPackage.CrosshairsLeft = nullptr;
		HUDPackage.CrosshairsRight = nullptr;
		HUDPackage.CrosshairsTop = nullptr;
		HUDPackage.CrosshairsBottom = nullptr;
	}
	// Calculate crosshair spread
	// if crouched [0, 300] -> [0, 1] else [0, 600] -> [0, 1]
	const FVector2D SpeedRange(0.f, Character->bIsCrouched
		                                ? Character->GetCharacterMovement()->MaxWalkSpeedCrouched
		                                : Character->GetCharacterMovement()->MaxWalkSpeed);
	const FVector2D VelocityMultiplierRange(0.f, 1.f);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(SpeedRange, VelocityMultiplierRange, Velocity.Size());

	if (Character->GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

	if (bOnTarget)
	{
		HUDPackage.CrosshairsColor = FLinearColor::Red;
		CrosshairAimAtEnemyFactor = FMath::FInterpTo(CrosshairAimAtEnemyFactor, 0.15f, DeltaTime, 20.f);
	}
	else
	{
		HUDPackage.CrosshairsColor = FLinearColor::White;
		CrosshairAimAtEnemyFactor = FMath::FInterpTo(CrosshairAimAtEnemyFactor, 0.f, DeltaTime, 20.f);
	}

	HUDPackage.CrosshairSpread =
		0.5f +
		CrosshairVelocityFactor +
		CrosshairInAirFactor -
		CrosshairAimFactor +
		CrosshairShootingFactor -
		CrosshairAimAtEnemyFactor;

	HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!Character->GetFollowCamera())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character->GetFollowCamera() is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!EquippedWeapon)
	{
		return;
	}

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
}

void UCombatComponent::StartFireTimer()
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!EquippedWeapon)
	{
		return;
	}

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (!EquippedWeapon)
	{
		return;
	}

	bCanFire = true;
	if (EquippedWeapon->bAutomatic && bFireButtonPressed)
	{
		Fire();
	}
}
