#include "CrosshairComponent.h"

UCrosshairComponent::UCrosshairComponent() :
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	ShootTimeDuration(0.05f),
	bIsFiringBullet(false)
{
	PrimaryComponentTick.bCanEverTick = true;
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

float UCrosshairComponent::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
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

void UCrosshairComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}