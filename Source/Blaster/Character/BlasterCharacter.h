// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#pragma region Camera

private:
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

#pragma endregion

#pragma region HUD

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category=HUD)
	TObjectPtr<UWidgetComponent> OverheadWidget;

#pragma endregion

#pragma region Replication

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetOverlappingWeapon(AWeapon* Weapon);

private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;

#pragma endregion

#pragma region Components

public:
	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCombatComponent> Combat;

#pragma endregion

private:
	UFUNCTION(Server, Reliable)
	void ServerEquip();

public:
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
};
