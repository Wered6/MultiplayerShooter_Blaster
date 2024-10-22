// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 10.f;
	LifeSpan = 2.f;
	bDidHit = false;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

#pragma region Nullchecks
	if (!CasingMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|CasingMesh is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);

	// Add randomness in rotation
	FVector CasingMeshImpulseVector{GetActorForwardVector()};
	CasingMeshImpulseVector.Y += FMath::RandRange(-0.2f, 0.2f);
	CasingMeshImpulseVector.Z += FMath::RandRange(-0.2f, 0.2f);
	CasingMesh->AddImpulse(CasingMeshImpulseVector * ShellEjectionImpulse);

	SetLifeSpan(LifeSpan);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
#pragma region Nullchecks
	if (!ShellSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|ShellSound is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	if (!bDidHit)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
		bDidHit = true;
	}
}
