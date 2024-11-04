// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter{Cast<ACharacter>(GetOwner())};

#pragma region Nullchecks
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|OwnerCharacter is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	AController* OwnerController{OwnerCharacter->Controller};

#pragma region Nullchecks
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|OwnerController is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
