// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

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

	void FireButtonPressed(const bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon() const;

private:
	TObjectPtr<ABlasterCharacter> Character;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;
};
