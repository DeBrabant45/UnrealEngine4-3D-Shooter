#include "CrosshairComponent.h"
#include "Kismet/GameplayStatics.h"

UCrosshairComponent::UCrosshairComponent()
{

}

void UCrosshairComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCrosshairComponent::CalculateCrosshairInAirFactor(float DeltaTime, bool IsFalling)
{
	if (IsFalling)
	{
		const float TargetValue = 2.25f;
		const float SpeedValue = 2.25f;
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, TargetValue, DeltaTime, SpeedValue); 
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
}

void UCrosshairComponent::CalculateCrosshairAimFactor(float DeltaTime, bool bIsAiming)
{
	if (bIsAiming)
	{
		float targetValue = 1.f;
		float targetSpeed = 30.f;
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, targetValue, DeltaTime, targetSpeed);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
}

void UCrosshairComponent::CalculateCrosshairVelocityFactor(FVector CharacterVelocity)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	//FVector CharacterVelocity{  };
	CharacterVelocity.Z = 0.f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		CharacterVelocity.Size());
}

void UCrosshairComponent::CalculateCrosshairShootingFactor(float DeltaTime)
{
	if (bIsFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}
}

void UCrosshairComponent::CalculateCrosshairSpread(float DeltaTime, FVector CharacterVelocity, bool bIsFalling, bool bIsAiming)
{
	CalculateCrosshairVelocityFactor(CharacterVelocity);
	CalculateCrosshairInAirFactor(DeltaTime, bIsFalling);
	CalculateCrosshairAimFactor(DeltaTime, bIsAiming);
	CalculateCrosshairShootingFactor(DeltaTime);
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void UCrosshairComponent::StartCrosshairBulletFireTimer()
{
	bIsFiringBullet = true;
	GetWorld()->GetTimerManager().SetTimer(
		CrosshairShootTimer,
		this,
		&UCrosshairComponent::FinishCrosshairBulletFire,
		ShootTimeDuration);
}

void UCrosshairComponent::FinishCrosshairBulletFire()
{
	bIsFiringBullet = false;
}

void UCrosshairComponent::GetViewportSize()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
}

bool UCrosshairComponent::IsScreenToWorld()
{
	return UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
}

void UCrosshairComponent::SetCrosshairLocation()
{
	FVector2D Temp(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation = Temp;
}

bool UCrosshairComponent::IsTraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	GetViewportSize();
	SetCrosshairLocation();
	if (IsScreenToWorld())
	{
		const FVector Start { CrosshairWorldPosition };
		const FVector End { Start + CrosshairWorldDirection * 50000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void UCrosshairComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}