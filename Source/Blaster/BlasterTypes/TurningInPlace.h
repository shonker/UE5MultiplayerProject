#pragma once
//these are replicated in CombatComp.h
//there is a rep notify in there as well for handling state change logic
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    ETIP_Left UMETA(DisplayName = "Turning Left"),
    ETIP_Right UMETA(DisplayName = "Turning Right"),
    ETIP_NotTurning UMETA(DisplayName = "Not Turning"),
    
    ETIP_MAX UMETA(DisplayName = "Default Max")
};