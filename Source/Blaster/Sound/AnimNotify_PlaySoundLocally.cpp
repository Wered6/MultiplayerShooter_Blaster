// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_PlaySoundLocally.h"

void UAnimNotify_PlaySoundLocally::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	const UWorld* World = MeshComp->GetWorld();
	if (World && World->WorldType == EWorldType::EditorPreview)
	{
		// for not checking instigator in previews
	}
	else
	{
		SavedSound = Sound;

#pragma region Nullchecks
		if (!MeshComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|MeshComp is nullptr"), *FString(__FUNCTION__))
			return;
		}
		if (!MeshComp->GetOwner())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|MeshComp->GetOwner() is nullptr"), *FString(__FUNCTION__))
			return;
		}
		if (!MeshComp->GetOwner()->GetInstigator())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s|MeshComp->GetOwner()->GetInstigator() is nullptr"), *FString(__FUNCTION__))
			return;
		}
#pragma endregion

		if (!MeshComp->GetOwner()->GetInstigator()->IsLocallyControlled())
		{
			Sound = nullptr;
		}

		Sound = SavedSound;
	}


	Super::Notify(MeshComp, Animation, EventReference);
}
