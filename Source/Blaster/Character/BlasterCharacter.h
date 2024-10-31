// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class ABlasterPlayerController;
enum class ETurningInPlace : uint8;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#pragma region Camera

public:
	FORCEINLINE UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}

private:
	void HideCameraIfCharacterClose() const;

	UPROPERTY(EditAnywhere)
	float CameraThreshold{200.f};

	UPROPERTY(VisibleAnywhere, Category=Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category=Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

#pragma endregion

#pragma region Input

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for equip input */
	void EquipButtonPressed();

	/** Called for crouch input */
	void CrouchButtonPressed();

	/** Called for aim input */
	void AimButtonPressed();
	void AimButtonReleased();

	/** Called for jump input */
	virtual void Jump() override;

	/** Called for fire input */
	void FireButtonPressed();
	void FireButtonReleased();

private:
	/** MappingContext */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> LookAction;

	/** Equip Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> EquipAction;

	/** Crouch Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> CrouchAction;

	/** Aim Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> AimAction;

	/** Fire Input Action */
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> FireAction;

#pragma endregion

#pragma region HUD

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowPlayerName();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category=HUD)
	TObjectPtr<UWidgetComponent> OverheadWidget;

#pragma endregion

#pragma region Components

public:
	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCombatComponent> Combat;

#pragma endregion

#pragma region Weapon

public:
	FVector GetHitTarget() const;

	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	AWeapon* GetEquippedWeapon() const;

	FORCEINLINE float GetAO_Yaw() const
	{
		return AO_Yaw;
	}

	FORCEINLINE float GetAO_Pitch() const
	{
		return AO_Pitch;
	}

	FORCEINLINE ETurningInPlace GetTurningInPlace() const
	{
		return TurningInPlace;
	}

	void PlayFireMontage(const bool bAiming) const;
	void CalculateAO_Pitch();

protected:
	void AimOffset(float DeltaTime);

private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const;

	UFUNCTION(Server, Reliable)
	void ServerEquip();
	void TurnInPlace(float DeltaTime);

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> FireWeaponMontage;

#pragma endregion

public:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	FORCEINLINE bool ShouldRotateRootBone() const
	{
		return bRotateRootBone;
	}

	virtual void OnRep_ReplicatedMovement() override;

protected:
	void PlayHitReactMontage() const;

	void SimProxiesTurn();

private:
	float CalculateSpeed() const;

	UPROPERTY(EditAnywhere, Category=Combat)
	TObjectPtr<UAnimMontage> HitReactMontage;

	bool bRotateRootBone{};
	float TurnThreshold{0.5f};
	FRotator ProxyRotationLastFrame{};
	FRotator ProxyRotation{};
	float ProxyYaw{};
	float TimeSinceLastMovementReplication{};

#pragma region PlayerStats

private:
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxHealth{100.f};
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Player Stats")
	float Health{100.f};

	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> BlasterPlayerController;

#pragma endregion
};
