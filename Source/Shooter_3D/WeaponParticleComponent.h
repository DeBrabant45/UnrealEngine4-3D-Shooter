

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponParticleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_3D_API UWeaponParticleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponParticleComponent();
	void SpawnImpactParticlesByEndBeamPoint(FVector& BeamEndPoint);
	void SpawnGunBeamParticle(const FTransform& SocketTransform, FVector& BeamEndPoint);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;
	
};
