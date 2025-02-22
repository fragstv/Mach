#pragma once

#include "GameFramework/Actor.h"
#include "MachWeapon.generated.h"

UENUM(WeaponState)
namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Charging,
		Equipping,
		Unequipped,
	};
}

/** Weapon burst mode. Full burst will continue refiring until the fire button is released. Semi burst mode will fire
for a short period of time and stop until the fire button is pressed again. Single burst mode will never refire,
similar to a pistol. Charge burst mode will charge up while the fire button is pressed and shoot when released. */
UENUM(WeaponBurstMode)
namespace EWeaponBurstMode
{
	enum Type
	{
		Full,
		Semi,
		Single,
		Charge
	};
}

/** The weapon model type determines the animation that is needed to hold it (pistol, rifle) */
UENUM(BlueprintType)
namespace EWeaponModelType
{
	enum Type
	{
		Rifle,
		Pistol,
	};
}

UENUM(BlueprintType)
namespace EWeaponResource
{
	enum Type
	{
		None,			// Weapon will continuously fire
		Ammo,			// Set ammo amount, must reload
		Energy,			// Energy depletes but recharges automatically, never runs out
		EnergyLimited,	// Same as energy but has a set energy amount before needing resupply
	};
}

USTRUCT()
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/** animation played on pawn (1st person view) */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* Pawn1P;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* Pawn3P;
};

/**
 * Base weapon class.
 */
UCLASS()
class AMachWeapon : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UArrowComponent* Arrow;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh3P;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	FName MuzzleAttachPoint;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<class AMachProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	FVector MuzzleOffset;

	/** Damage per shot / bullet */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float Damage;

	/** Splash damage radius (only for projectiles) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float DamageRadius;

	/** Max ammo per clip */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float MaxAmmo;

	/** Total number of clips that can be carried at a time */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float AmmoPerClip;

	/** Time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

	/** Time between two bursts in semi burst mode */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenSemiBursts;

	/** Weapon burst mode */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TEnumAsByte<EWeaponBurstMode::Type> BurstMode;

	/** Weapon resource mode */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TEnumAsByte<EWeaponResource::Type> Resource;

	/** Range of the weapon (Default: 12000) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float Range;

	/** Weapon spread, if greater than 0 will apply a random spread */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float Spread;

	/** Rate at which energy is consumed while weapon is firing (per second) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float EnergyRate;

	/** Rate at which energy is recharged while the weapon is not firing (per second) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float EnergyRechargeRate;

	/** This is awkard and weird, we should have animations for each weapon specifically */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = WeaponStat)
	TEnumAsByte<EWeaponModelType::Type> WeaponModelType;

	/** Impact effects */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<class AMachImpactEffect> ImpactTemplate;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName TrailTargetParam;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* MuzzleFX;

	/** spawned component for muzzle FX */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** spawned component for second muzzle FX (Needed for split screen) */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSCSecondary;

	/** fire animations */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim FireAnim;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	void SetOwningPawn(class AMachCharacter* NewPawn);
	void OnEquip();
	void OnUnequip();

	UFUNCTION()
	virtual void StartFire();

	UFUNCTION()
	virtual void StopFire();

	UFUNCTION()
	virtual void StartSecondaryFire();

	UFUNCTION()
	virtual void StopSecondaryFire();
	
	UFUNCTION()
	void Reload();

	void ProcessHit(const FHitResult& Impact, const FVector& StartTrace, const FVector& ShootDir);
	void ProcessHit(const float DealingDamage, const FHitResult& Impact, const FVector& StartTrace, const FVector& ShootDir);

	class USkeletalMeshComponent* GetWeaponMesh() const;

protected:
	UPROPERTY(Transient, Replicated)
	TEnumAsByte<EWeaponState::Type> CurrentState;

	UPROPERTY(Transient, Replicated)
	AMachCharacter* OwnerPawn;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category=Gameplay)
	float TotalAmmo;
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category=Gameplay)
	float Ammo;

	/** is muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	uint32 bLoopedMuzzleFX : 1;

	uint32 bPlayingFireAnim : 1;
	uint8 bIsEquipped : 1;
	uint8 bEquipPending : 1;
	uint8 bFireIntent : 1;
	uint8 bReloadIntent : 1;
	uint8 bRefiring : 1;

	float fLastFireTime;
	float fLastBurstTime;

	UPROPERTY(Transient, Replicated)
	FHitResult HitImpact;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	virtual void Tick(float DeltaSeconds);

	bool CanReload();
	bool CanFire();

	void OnEquipFinished();
	void UpdateWeaponState();
	void SetCurrentState(EWeaponState::Type);
	virtual void OnBurstStarted();
	virtual void OnBurstFinished();
	void OnReloadStarted();
	void OnReloadFinished();
	virtual void HandleFiring();
	FVector GetViewRotation();
	void GetViewPoint(FVector& start, FRotator& rotation);
	void SpawnImpactEffects(const FHitResult& impact);
	void SpawnTrailEffect(const FVector& EndPoint);

	void SimulateWeaponFiring();
	
	float PlayWeaponAnimation(const FWeaponAnim& Animation);
	void StopWeaponAnimation(const FWeaponAnim& Animation);
	
	virtual void FireWeapon();
	FHitResult WeaponTrace(const FVector& start, const FVector& end) const;
	void AttachMesh();
	void DetachMesh();
	virtual void ConsumeAmmo();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireProjectile();

	virtual void FireProjectile();

	FVector GetMuzzleLocation() const;
	FVector GetMuzzleDirection() const;

	void NotifyHit(const FHitResult& impact, const FVector& origin);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartSecondaryFire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopSecondaryFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	UFUNCTION(reliable, server, WithValidation)
	void ServerNotifyHit(const FHitResult impact, const FVector origin);

	/** Burst counter, used for replicating fire events to remote clients */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_BurstCounter)
	int32 BurstCounter;

	//////////////////////////////////////////////////////////////////////////
	// Replication
	
	UFUNCTION()
	void OnRep_BurstCounter();
};
