// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Game/GDKMetaData.h"
#include "GameFramework/Actor.h"
#include "Materials/Material.h"
#include "Camera/CameraShake.h"
#include "Weapon.generated.h"

USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	// Location of the hit in world space.
	UPROPERTY()
		FVector Location;

	// Actor that was hit, or nullptr if nothing was hit.
	UPROPERTY()
		AActor* HitActor;

	FInstantHitInfo() :
		Location(FVector{ 0,0,0 }),
		HitActor(nullptr)
	{}
};

enum class EWeaponState : uint8
{
	Idle,
	Firing
};

// Tag for weapon line trace visualization.
const FName kTraceTag("GDKTrace");

DECLARE_DELEGATE_OneParam(FShotDelegate, bool);
UCLASS(Abstract)
class GDKSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void Tick(float DeltaSeconds) override;

	// [client] Starts firing the weapon.
	virtual void StartFire() PURE_VIRTUAL(AWeapon::StartFire,);

	// [client] Stops firing the weapon.
	virtual void StopFire();

	// [client] Fire the weapon if not on cooldown.
	virtual void TryShooting() PURE_VIRTUAL(AWeapon::TryShooting, );

	class AGDKShooterCharacter* GetOwningCharacter() const;
	void SetOwningCharacter(class AGDKShooterCharacter* NewCharacter);

	AActor* GetWeilder() const { return GetAttachmentReplication().AttachParent; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MetaData)
		FGDKMetaData MetaData;

	UFUNCTION(BlueprintImplementableEvent)
	void OnMetaDataUpdated();

	void SetMetaData(FGDKMetaData MetaData);

	void EnableShadows(bool bShadows);

	void SetIsActive(bool bNewValue) { bIsActive = bNewValue; }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
		bool IsFirstPerson();

	void SetFirstPerson(bool bFirstPerson_);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float AimingFoV = 55;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float AimingRotationSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UMaterialInstance* Reticle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FVector2D ReticleSize = FVector2D(16, 16);

	void AddShotListener(FShotDelegate Listener);
	void RemoveShotListener();

	//Recoil applied as input to the CharacterController rotations
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilYawMax;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilYawMin;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilRightBias = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilPitchMax;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilPitchMin;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilUpBias = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilCrouchModifier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilAimModifier = 0.5f;

	//Rate at which recoil is recovered from
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RecoilRecoveryRate = 1;
	//Recoil applied as a CameraShake
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UCameraShake> RecoilShake;

	UFUNCTION(BlueprintCallable)
	FVector BulletSpawnPoint();

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	EWeaponState GetWeaponState() const;
	void SetWeaponState(EWeaponState NewState);

	UFUNCTION()
		virtual void OnRep_IsActive();

	virtual void AnnounceShot(bool bHit);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName BarrelSocketName = FName(TEXT("WP_Barrel"));

	// Weapon mesh.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;

	virtual FVector GetLineTraceDirection();

	// [client] Performs a line trace and populates OutHitInfo based on the results.
	// Returns true if it hits anything, false otherwise.
	bool DoLineTrace(FInstantHitInfo& OutHitInfo);

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
		bool bIsActive;

	// Maximum range of the weapon's hitscan.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
		float MaxRange;

private:

	bool bHasAttached;

	void TryToAttach();

	EWeaponState CurrentState;

	bool bFirstPerson;

	FShotDelegate ShotCallback;

	// Set up a root component so this actor can have a position in the world.
	class USceneComponent* LocationComponent;

	// Character that currently owns this weapon.
	UPROPERTY(Replicated)
		class AGDKShooterCharacter* OwningCharacter;

	UFUNCTION()
		void OnRep_MetaData();

	// Channel to use for raytrace on shot
	UPROPERTY(EditAnywhere, Category = "Weapons")
		TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

	// If true, draws debug line traces for hitscan shots.
	UPROPERTY(EditAnywhere, Category = "Weapons")
		bool bDrawDebugLineTrace;
	
};
