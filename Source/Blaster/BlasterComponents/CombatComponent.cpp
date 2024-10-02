// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
#pragma region Nullchecks
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Character is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!WeaponToEquip)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|WeaponToEquip is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

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
}
