// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();

	// Offset Yaw for Strafing
	const FRotator AimRotation{BlasterCharacter->GetBaseAimRotation()};
	const FRotator MovementRotation{UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity())};
	const FRotator DeltaRot{UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation)};
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta{UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};
	const float Target{static_cast<float>(Delta.Yaw / DeltaSeconds)};
	const float Interp{FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f)};
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped)
	{
#pragma region Nullchecks
		if (!EquippedWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|EquippedWeapon is nullptr"), *FString(__FUNCTION__))
			return;
		}
		if (!EquippedWeapon->GetWeaponMesh())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|EquippedWeapon->GetWeaponMesh() is nullptr"), *FString(__FUNCTION__))
			return;
		}
		if (!BlasterCharacter->GetMesh())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|BlasterCharacter->GetMesh() is nullptr"), *FString(__FUNCTION__))
			return;
		}
#pragma endregion

		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			const FTransform RightHandTransform{EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), RTS_World)};
			const FVector RightHandLocation{RightHandTransform.GetLocation()};
			const FRotator LookAtRotation{UKismetMathLibrary::FindLookAtRotation(RightHandLocation, RightHandLocation + (RightHandLocation - BlasterCharacter->GetHitTarget()))};
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}
}
