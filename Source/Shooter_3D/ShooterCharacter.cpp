#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "ShooterCharacter/CrosshairComponent.h"

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
	ZoomInterpolationSpeed(20.f)
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
	UAnimInstance* animationInstance = GetMesh()->GetAnimInstance();
	if (animationInstance && HipFireMontage)
	{
		animationInstance->Montage_Play(HipFireMontage);
		animationInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	//Get current size of the viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get Screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get World position and direction of cross hairs
	bool bIsScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	
	if (bIsScreenToWorld)
	{
		FHitResult screenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };
		OutBeamLocation = End;
		GetWorld()->LineTraceSingleByChannel(screenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (screenTraceHit.bBlockingHit)
		{
			OutBeamLocation = screenTraceHit.Location;
		}

		// perform a second trace from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ MuzzleSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
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

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetCurrentCameraFieldOfView(DeltaTime);
	SetLookRates();
	CrosshairComponent->CalculateCrosshairSpread(DeltaTime, GetVelocity(), GetCharacterMovement()->IsFalling(), bIsAiming);
}

// Called to bind functionality to input
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

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);
	
	PlayerInputComponent->BindAction("AimButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}