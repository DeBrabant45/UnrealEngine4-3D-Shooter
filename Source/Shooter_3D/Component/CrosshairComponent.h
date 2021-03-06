#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrosshairComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_3D_API UCrosshairComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCrosshairComponent();

protected:
	virtual void BeginPlay() override;
	void CalculateCrosshairInAirFactor(float DeltaTime, bool bIsFalling);
	void CalculateCrosshairAimFactor(float DeltaTime, bool bIsAiming);
	void CalculateCrosshairVelocityFactor(FVector CharacterVelocity);
	void CalculateCrosshairShootingFactor(float DeltaTime);
	UFUNCTION()
	void FinishCrosshairBulletFire();
	void GetViewportSize();
	bool IsScreenToWorld();
	void SetCrosshairLocation();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier{ 0.f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor{ 0.f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor{ 0.f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor{ 0.f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor{ 0.f };
	UPROPERTY()
	bool bIsFiringBullet{ false };
	UPROPERTY()
	float ShootTimeDuration{ 0.05f };
	UPROPERTY()
	FTimerHandle CrosshairShootTimer{};
	UPROPERTY()
	FVector2D ViewportSize;
	UPROPERTY()
	FVector CrosshairWorldPosition;
	UPROPERTY()
	FVector CrosshairWorldDirection;
	UPROPERTY()
	FVector2D CrosshairLocation;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CalculateCrosshairSpread(float DeltaTime, FVector CharacterVelocity, bool bIsFalling, bool bIsAiming);
	void StartCrosshairBulletFireTimer();
	bool IsTraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier; }
};
