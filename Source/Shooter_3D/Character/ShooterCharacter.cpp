#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter_3D/Component/CrosshairComponent.h"
#include "Shooter_3D/Component/ItemTraceComponent.h"
#include "Shooter_3D/Component/WeaponParticleComponent.h"
#include "Shooter_3D/Item/Weapon.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter() : 
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	ControllerHipTurnRate(90.f),
	ControllerHipLookUpRate(90.f),
	ControllerAimingTurnRate(20.f),
	ControllerAimingLookUpRate(20.f),
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	bIsAiming(false),
	CameraDefaultFieldOfView(0.f),
	CameraZoomedFieldOfView(40.f),
	CameraCurrentFieldOfView(0.f),
	ZoomInterpolationSpeed(20.f),
	AutomaticFireRate(0.1f),
	bShouldFire(true),
	bIsFireButtonPressed(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller 
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the control rotates, controller only affects camera not player
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure Character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	WeaponParticleComponent = CreateDefaultSubobject<UWeaponParticleComponent>(TEXT("WeaponParticleComponent"));
	CrosshairComponent = CreateDefaultSubobject<UCrosshairComponent>(TEXT("CrosshairComponent"));
	ItemTraceComponent = CreateDefaultSubobject<UItemTraceComponent>(TEXT("ItemTrace"));
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (FollowCamera != nullptr)
	{
		CameraDefaultFieldOfView = GetFollowCamera()->FieldOfView;
		CameraCurrentFieldOfView = CameraDefaultFieldOfView;
	}
	EquipWeapon(SpawnDefaultWeapon());
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0))
	{
		// find out which way is forward
		const FRotator ControllerRotation{ Controller->GetControlRotation() };
		const FRotator YawControllerRotation{ 0, ControllerRotation.Yaw, 0 };
		const FVector ControllerXDirection{ FRotationMatrix{YawControllerRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(ControllerXDirection, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0))
	{
		// find out which way is right
		const FRotator ControllerRotation{ Controller->GetControlRotation() };
		const FRotator YawControllerRotation{ 0, ControllerRotation.Yaw, 0 };

		const FVector ControllerYDirection{ FRotationMatrix{YawControllerRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(ControllerYDirection, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::MouseTurnAtRate(float Value)
{
	float TurnScaleFactor;
	if (bIsAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::MouseLookUpAtRate(float Value)
{
	float LookScaleFactor;
	if (bIsAiming)
	{
		LookScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket != nullptr)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		SpawnWeaponParticles(SocketTransform);
		SpawnMuzzleFlash(SocketTransform);
		PlayFireSound();
		PlayFireAnimation();
		CrosshairComponent->StartCrosshairBulletFireTimer();
	}
}

void AShooterCharacter::SpawnWeaponParticles(const FTransform& SocketTransform)
{
	FVector BeamEnd;
	bool bIsBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd );
	if (bIsBeamEnd)
	{
		WeaponParticleComponent->SpawnImpactParticlesByEndBeamPoint(BeamEnd);
		WeaponParticleComponent->SpawnGunBeamParticle(SocketTransform, BeamEnd);
	}
}

void AShooterCharacter::SpawnMuzzleFlash(const FTransform& SocketTransform)
{
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
	}
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::PlayFireAnimation()
{
	UAnimInstance* AnimationInstance = GetMesh()->GetAnimInstance();
	if (AnimationInstance && HipFireMontage)
	{
		AnimationInstance->Montage_Play(HipFireMontage);
		AnimationInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

//Refactor all of this below
bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{

	FHitResult CrosshairHitResult;
	bool bCrosshairHit = CrosshairComponent->IsTraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}

	// perform a second trace from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ OutBeamLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bIsAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bIsAiming = false;
}

void AShooterCharacter::SetCurrentCameraFieldOfView(float DeltaTime)
{
	if (bIsAiming)
	{
		CameraCurrentFieldOfView = FMath::FInterpTo(
			CameraCurrentFieldOfView,
			CameraZoomedFieldOfView,
			DeltaTime,
			ZoomInterpolationSpeed);
	}
	else
	{
		CameraCurrentFieldOfView = FMath::FInterpTo(
			CameraCurrentFieldOfView,
			CameraDefaultFieldOfView,
			DeltaTime,
			ZoomInterpolationSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFieldOfView);
}

void AShooterCharacter::SetLookRates()
{
	if (bIsAiming)
	{
		BaseTurnRate = ControllerAimingTurnRate;
		BaseLookUpRate = ControllerAimingLookUpRate;
	}
	else
	{
		BaseTurnRate = ControllerHipTurnRate;
		BaseLookUpRate = ControllerHipLookUpRate;
	}
}

void AShooterCharacter::FireButtonPressed()
{
	bIsFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bIsFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}	
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bIsFireButtonPressed)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::TraceForItems()
{
	if (ItemTraceComponent->GetShouldTraceForItems())
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		CrosshairComponent->IsTraceUnderCrosshairs(ItemTraceResult, HitLocation);
		ItemTraceComponent->TraceForItems(ItemTraceResult);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	return (DefaultWeaponClass != nullptr) ? GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass) : nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip != nullptr)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket != nullptr)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::Eis_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon != nullptr)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::Eis_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	DropWeapon();
}

void AShooterCharacter::SelectButtonReleased()
{
	if (ItemTraceComponent->GetTraceHitItemCurrentFrame() != nullptr)
	{
		auto TraceHitWeapon = Cast<AWeapon>(ItemTraceComponent->GetTraceHitItemCurrentFrame());
		SwapWeapon(TraceHitWeapon);
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	ItemTraceComponent->ResetTraceHitItems();
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetCurrentCameraFieldOfView(DeltaTime);
	SetLookRates();
	CrosshairComponent->CalculateCrosshairSpread(DeltaTime, GetVelocity(), GetCharacterMovement()->IsFalling(), bIsAiming);
	TraceForItems();
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::MouseTurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::MouseLookUpAtRate);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);
}