// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Casing.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

#pragma region Nullchecks
	if (!AreaSphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|AreaSphere is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!PickupWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|PickupWidget is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	// only server
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	PickupWidget->SetVisibility(false);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::SetWeaponState(const EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_Max:
		break;
	}
}

void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
#pragma region Nullchecks
	if (!PickupWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|PickupWidget is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	PickupWidget->SetVisibility(bShowWidget);
}

void AWeapon::Fire(const FVector& HitTarget)
{
#pragma region Nullchecks
	if (!FireAnimation)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|FireAnimation is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!WeaponMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|WeaponMesh is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!CasingClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|CasingClass is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	WeaponMesh->PlayAnimation(FireAnimation, false);

	const USkeletalMeshSocket* AmmoEjectSocket{WeaponMesh->GetSocketByName(FName("AmmoEject"))};
	UWorld* World{GetWorld()};

#pragma region Nullchecks
	if (!AmmoEjectSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|MuzzleFlashSocket is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|World is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	const FTransform SocketTransform{AmmoEjectSocket->GetSocketTransform(WeaponMesh)};

	World->SpawnActor<ACasing>(
		CasingClass,
		SocketTransform.GetLocation(),
		SocketTransform.GetRotation().Rotator()
	);
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
}

// Gets called only on server
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                              AActor* OtherActor,
                              UPrimitiveComponent* OtherComp,
                              int32 OtherBodyIndex,
                              bool bFromSweep,
                              const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter{Cast<ABlasterCharacter>(OtherActor)};
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                 AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp,
                                 int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter{Cast<ABlasterCharacter>(OtherActor)};
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_WeaponState() const
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
	// AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_Max:
		break;
	}
}
