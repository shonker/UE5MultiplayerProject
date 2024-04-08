// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (BlasterCharacter == nullptr)
    {
        BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
    }
    if (BlasterCharacter == nullptr) return;

    FVector Velocity = BlasterCharacter->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();

    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
    //this is really checking if there is input, not acceleration
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
    bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
    EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
    bIsCrouched = BlasterCharacter->bIsCrouched;
    bAiming = BlasterCharacter->IsAiming();
    
    TurningInPlace = BlasterCharacter->GetTurningInPlace();
    bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
    bElimmed = BlasterCharacter->IsElimmed();


    FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    //as we r a circle, rotating between -180 and 180 is approx 0 degrees, 
    //thus interpolation between the two should never go all the way -180 -> -100 -> 0 -> 100 -> 180
    //RInterpTo knows we r circle, and knows the shortest path between the two points in the FRotator
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 15.f);
    YawOffset = DeltaRotation.Yaw;

    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = BlasterCharacter->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    const float Target = Delta.Yaw / DeltaSeconds;
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 1.f);
    Lean = FMath::Clamp(Interp, -90.f, 90.f);   

    AO_Yaw = BlasterCharacter->GetAOYaw();
    AO_Pitch = BlasterCharacter->GetAOPitch();
 

    if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
    {
        //make left hand transform to barrel
        LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
        FVector OutPosition;
        FRotator OutRotation;
        BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(FQuat(OutRotation));

        //make right hand point gun at target
        RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_R"), ERelativeTransformSpace::RTS_World);
       
        // RightHandRotation = UKismetMathLibrary::FindLookAtRotation(
        //     RightHandTransform.GetLocation(), 
        //     RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget())
        // );

        FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), 
        RightHandTransform.GetLocation() + RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget());
            LookAtRotation.Roll += BlasterCharacter->RightHandRotationRoll;
            LookAtRotation.Yaw += BlasterCharacter->RightHandRotationYaw;
            LookAtRotation.Pitch += BlasterCharacter->RightHandRotationPitch;

            FRotator TargetRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 100.f);


        if (BlasterCharacter->IsLocallyControlled())
        {
            bLocallyControlled = true;    
            FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
            RightHandRotation = FMath::RInterpTo(RightHandRotation, TargetRotation, DeltaSeconds, 30.f);
            
            //get direction in world space of the X axis of the muzzle
            FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
           // DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation()+ MuzzleX * 1000.f, FColor::Red);
            //DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Red);
        }
        
    }
    bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
    bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
    bTransformRightHand = bAiming;
    //bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}

void UBlasterAnimInstance::AnimNotify_StartKiss()
{
    bKissing = true;
}

void UBlasterAnimInstance::AnimNotify_EndKiss()
{
    bKissing = false;
}