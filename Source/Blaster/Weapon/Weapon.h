// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ACasing;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_Max UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	FORCEINLINE USphereComponent* GetAreaSphere() const
	{
		return AreaSphere;
	}

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const
	{
		return WeaponMesh;
	}

	void SetWeaponState(const EWeaponState State);

	void ShowPickupWidget(const bool bShowWidget) const;

	virtual void Fire(const FVector& HitTarget);

	FORCEINLINE float GetZoomedFOV() const
	{
		return ZoomedFOV;
	}

	FORCEINLINE float GetZoomInterpSpeed() const
	{
		return ZoomInterpSpeed;
	}

	/*
	* Textures fot the weapon crosshairs
	*/
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsRight;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsTop;
	UPROPERTY(EditAnywhere, Category=Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsBottom;

protected:
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	                             AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp,
	                             int32 OtherBodyIndex,
	                             bool bFromSweep,
	                             const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                        AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex);

private:
	UFUNCTION()
	void OnRep_WeaponState() const;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<USphereComponent> AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category="Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	/*
	 * Zoomed FOV while aiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV{30.f};
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed{20.f};
};
