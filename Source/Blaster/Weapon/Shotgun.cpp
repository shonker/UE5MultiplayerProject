// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

//cut this into smaller functions
void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		/*
		LEARN WHAT TMAPS ARE -- 
		THIS IS VERY USEFULLY MAPPING EACH
		INDIVIDUAL PLAYER TO A VALUE (# OF TIMES HIT)
		*/
		TMap<ABlasterCharacter*, uint32>HitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			//is the impacted actor a player
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}

			ImpactParticles = BlasterCharacter ? ImpactCharacterParticles : ImpactEnvironmentParticles;
			ImpactSound = BlasterCharacter ? ImpactCharacterSound : ImpactEnvironmentSound;

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					ImpactSound,
					FireHit.ImpactPoint,
					0.25f,
					FMath::FRandRange(-.5f,.5f)
				);
			}

		}// end of for loop
		//THIS IS HOW WE LOOP THROUGH THE TMAP JESUSTHIS IS WHAT IVE WANTED THANK YOU GOD
		for (auto HitPair : HitMap)
		{
			//key returns the individual blastercharacter pointer we have looped to?
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value, //here is the number associated with the key (# times char hit)
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}