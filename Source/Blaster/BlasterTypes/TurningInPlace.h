// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETTP_Left UMETA(DisplayName = "Turning Left"),
	ETTP_Right UMETA(DisplayName = "Turning Right"),
	ETTP_NotTurning UMETA(DisplayName = "Not Turning"),

	ETTP_MAX UMETA(DisplayName = "DefaultMAX")
};
