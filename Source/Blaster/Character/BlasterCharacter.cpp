// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

// Called on clients for all characters
void ABlasterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ShowPlayerName();
}

// Called on server for clients' characters, but not for server's character
void ABlasterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ShowPlayerName();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);

		UpdateHUDHealth();
		if (HasAuthority())
		{
			ShowPlayerName();
		}
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCameraIfCharacterClose();
}

void ABlasterCharacter::HideCameraIfCharacterClose() const
{
#pragma region Nullchecks
	if (!FollowCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|FollowCamera is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|GetMesh() is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!IsLocallyControlled() || !Combat->EquippedWeapon)
	{
		return;
	}

	USkeletalMeshComponent* WeaponMesh{Combat->EquippedWeapon->GetWeaponMesh()};

#pragma region Nullchecks
	if (!WeaponMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|WeaponMesh is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	const float CameraActorDistance{static_cast<float>((FollowCamera->GetComponentLocation() - GetActorLocation()).Size())};
	if (CameraActorDistance < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		WeaponMesh->bOwnerNoSee = true;
	}
	else
	{
		GetMesh()->SetVisibility(true);
		WeaponMesh->bOwnerNoSee = false;
	}
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
#pragma region Nullchecks
	if (!DefaultMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|DefaultMappingContext is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!JumpAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|JumpAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!MoveAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|MoveAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!LookAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|LookAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!EquipAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|EquipAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!CrouchAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|CrouchAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!AimAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|AimAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!FireAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|FireAction is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add Input Mapping Context
	const APlayerController* PlayerController{Cast<APlayerController>(GetController())};

#pragma region Nullchecks
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|PlayerController is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	UEnhancedInputLocalPlayerSubsystem* LocalPlayerSubsystem{
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())
	};

#pragma region Nullchecks
	if (!LocalPlayerSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|LocalPlayerSubsystem is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	LocalPlayerSubsystem->AddMappingContext(DefaultMappingContext, 0);

	// Set up action bindings
	UEnhancedInputComponent* EnhancedInputComponent{Cast<UEnhancedInputComponent>(PlayerInputComponent)};

#pragma region Nullchecks
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|EnhancedInputComponent is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	// Jumping
	// Add jump here and check, because in github you bind two functions, and here only one
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABlasterCharacter::Jump);

	// Moving
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);

	// Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);

	// Equip
	EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlasterCharacter::EquipButtonPressed);

	// Crouch
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButtonPressed);

	// Aim
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlasterCharacter::AimButtonPressed);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);

	// Fire
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlasterCharacter::FireButtonPressed);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlasterCharacter::FireButtonReleased);
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector{Value.Get<FVector2D>()};

#pragma region Nullchecks
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Controller is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	// Find out which way is forward
	const FRotator Rotation{Controller->GetControlRotation()};
	const FRotator YawRotation{0, Rotation.Yaw, 0};

	// Get forward vector
	const FVector ForwardDirection{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)};

	// Get right vector
	const FVector RightDirection{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)};

	// Add movement
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector{Value.Get<FVector2D>()};

#pragma region Nullchecks
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Controller is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	// Add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::EquipButtonPressed()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	// only server
	if (HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquip();
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	bIsCrouched ? UnCrouch() : Crouch();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::AimButtonPressed()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Combat->SetAiming(true);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::AimButtonReleased()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Combat->SetAiming(false);
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::FireButtonPressed()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Combat->FireButtonPressed(true);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::FireButtonReleased()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Combat->FireButtonPressed(false);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Combat->Character = this;
}

FVector ABlasterCharacter::GetHitTarget() const
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return FVector();
	}
#pragma endregion

	return Combat->HitTarget;
}

// Gets called only on server
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return Combat && Combat->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming() const
{
	return Combat && Combat->bAiming;
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return nullptr;
	}
#pragma endregion

	return Combat->EquippedWeapon;
}

void ABlasterCharacter::PlayFireMontage(const bool bAiming) const
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!Combat->EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat->EquippedWeapon is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!FireWeaponMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|FireWeaponMontage is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	UAnimInstance* AnimInstance{GetMesh()->GetAnimInstance()};

#pragma region Nullchecks
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|AnimInstance is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	AnimInstance->Montage_Play(FireWeaponMontage);
	const FName SectionName{bAiming ? FName("RifleAim") : FName("RifleHip")};
	AnimInstance->Montage_JumpToSection(SectionName);
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && !Combat->EquippedWeapon)
	{
		return;
	}

	const float Speed{CalculateSpeed()};
	const bool bIsInAir{GetCharacterMovement()->IsFalling()};

	if (Speed == 0.f && !bIsInAir) // standing still, and not jumping
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation{FRotator(0.f, GetBaseAimRotation().Yaw, 0.f)};
		const FRotator DeltaAimRotation{UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation)};
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

// Won't be called on server, because replication only works one way (server -> client)
void ABlasterCharacter::OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquip_Implementation()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	Combat->EquipWeapon(OverlappingWeapon);
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
}

void ABlasterCharacter::PlayElimMontage() const
{
	UAnimInstance* AnimInstance{GetMesh()->GetAnimInstance()};

#pragma region Nullchecks
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|AnimInstance is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!ElimMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|ElimMontage is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	AnimInstance->Montage_Play(ElimMontage);
}

void ABlasterCharacter::PlayHitReactMontage() const
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!HitReactMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|FireWeaponMontage is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!Combat->EquippedWeapon)
	{
		return;
	}

	UAnimInstance* AnimInstance{GetMesh()->GetAnimInstance()};

#pragma region Nullchecks
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|AnimInstance is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	AnimInstance->Montage_Play(HitReactMontage);
	const FName SectionName("FromFront");
	AnimInstance->Montage_JumpToSection(SectionName);
}

void ABlasterCharacter::SimProxiesTurn()
{
#pragma region Nullchecks
	if (!Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|Combat is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!Combat->EquippedWeapon)
	{
		return;
	}

	bRotateRootBone = false;

	const float Speed{CalculateSpeed()};
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
	}
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode{GetWorld()->GetAuthGameMode<ABlasterGameMode>()};
		ABlasterPlayerController* AttackerController{Cast<ABlasterPlayerController>(InstigatorController)};

#pragma region Nullchecks
		if (!BlasterGameMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|BlasterGameMode is nullptr"), *FString(__FUNCTION__))
			return;
		}
		if (!AttackerController)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|AttackerController is nullptr"), *FString(__FUNCTION__))
			return;
		}
#pragma endregion

		BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
	}
}

void ABlasterCharacter::UpdateHUDHealth() const
{
#pragma region Nullchecks
	if (!BlasterPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|BlasterPlayerController is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
}

float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health() const
{
	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
	}
	PlayHitReactMontage();
}
