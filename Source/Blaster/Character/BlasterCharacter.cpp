// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);

	// Moving
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);

	// Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);

	// Equip
	EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlasterCharacter::Equip);

	// Crouch
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::Crouch);
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
void ABlasterCharacter::Equip()
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

void ABlasterCharacter::Crouch()
{
	bIsCrouched ? ACharacter::UnCrouch() : ACharacter::Crouch();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
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

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return Combat && Combat->EquippedWeapon;
}
