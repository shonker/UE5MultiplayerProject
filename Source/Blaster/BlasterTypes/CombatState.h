#pragma once


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_ThrowCharging UMETA(DisplayName = "ThrowCharging"),
	ECS_Throwing UMETA(DisplayName = "Throwing"),
	ECS_KissCharging UMETA(DisplayName = "KissCharging"),
	ECS_Kissing UMETA(DisplayName = "Kissing"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};