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
	void CalculateCrosshairInAirFactor(float DeltaTime, bool IsFalling);
	void CalculateCrosshairAimFactor(float DeltaTime, bool bIsAiming);
	void CalculateCrosshairVelocityFactor(FVector CharacterVelocity);
	void CalculateCrosshairShootingFactor(float DeltaTime);
	UFUNCTION()
	void FinishCrosshairBulletFire();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	bool bIsFiringBullet;
	
	float ShootTimeDuration;
	
	FTimerHandle CrosshairShootTimer;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void CalculateCrosshairSpread(float DeltaTime, FVector CharacterVelocity, bool bIsFalling, bool bIsAiming);

	void StartCrosshairBulletFireTimer();
	
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
};
