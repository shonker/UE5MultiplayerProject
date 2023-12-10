#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayName = "SniperRifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"),

	EWT_MAX UMETA(DisplayName = "Default Max")
};


/*

processs for adding a new weapon to the game:

1: Update weapon type categories above and the corresponding usage in:
	- blasterchar.cpp's reload montage
	- combatcomponent.cpp's initialize carried ammo & 
	- combatcommonent.h file int32 "StartingSniperAmmo" (eg)

	OPTIONAL STEP: if you are creating a new weapon CLASS, 
		not just modifying parameters of an existing type
1.5: In C++/Weapons/ create new C++ class inheriting from either:
		Weapon (base class of all weapons, assault rifle inherits this)
			---!!!!OR!!!!---
		HitScanWeapon (shotgun, pistol, sniper, submachinegun inherit from this)
		ProjectileWeapon (rocket launcher)
	- in VS reload all, remove Weapon/ from includes in .h & .cpp
	- Do as you must from here

2: In ue5 Weapon/ make new BP for weapon, inherit from corresponding C++ class
	- set mesh
	- adjust area sphere radius 
	- raise pickup widget: 
		- space = screen, widget class = WBP_PickupWidget, draw at desired size
	- set all right side EditAnywhere variables 
		- (category: hs weap, crosshairs, weapon, combat, scatter, casing, etc) 

3: in Weapon/Casings/ create new BP inherit from C++ casing class
	- assign mesh
	- optionally duplicate material instance & inherit parent: M_Ammo for red heat emission
	- or simply reuse preexisting casing class

4: Manipulate weapon firing animation, if importing from fbx
	- create animation sequence
	- notify track 1: muzzle flash particle effect
	- notify trac 2: firing cue

5: Go to skeletal mesh and add sockets:
	- to root: LeftHandSocket
		- (note: socket best ends up where base of palm would meet gun, 
			left side of gun, green arrow halfway into mesh)
	- to root: MuzzleFlash
	- to root: AmmoEject 
		- (note: right click, add preview asset, manipulate transform appropriately)
		- X is direction ammo is ejected (red arrow)

*/