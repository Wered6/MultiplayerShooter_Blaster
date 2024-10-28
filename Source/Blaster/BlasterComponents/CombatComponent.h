// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

class ABlasterHUD;
class ABlasterPlayerController;
class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	friend class ABlasterCharacter;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	void SetAiming(const bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon() const;

	void FireButtonPressed(const bool bPressed);
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	void InterpFOV(float DeltaTime);

	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;
	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> Controller;
	UPROPERTY()
	TObjectPtr<ABlasterHUD> HUD;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	/*
	 * HUD and crosshairs
	 */
	float CrosshairVelocityFactor{};
	float CrosshairInAirFactor{};
	float CrosshairAimFactor{};
	float CrosshairShootingFactor{};
	float CrosshairAimAtEnemyFactor{};

	FVector HitTarget;

	FHUDPackage HUDPackage{};

	bool bOnTarget{};

	/*
	 * Aiming and FOV
	 */
	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV{};
	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomedFOV{30.f};
	float CurrentFOV{};
	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomInterpSpeed{20.f};
};
