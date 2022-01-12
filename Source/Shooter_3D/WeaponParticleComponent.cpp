#include "WeaponParticleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values for this component's properties
UWeaponParticleComponent::UWeaponParticleComponent()
{
	
}

void UWeaponParticleComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponParticleComponent::SpawnImpactParticlesByEndBeamPoint(FVector& BeamEndPoint)
{
	if (ImpactParticles != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndPoint);
	}
}

void UWeaponParticleComponent::SpawnGunBeamParticle(const FTransform& SocketTransform, FVector& BeamEndPoint)
{
	if (BeamParticles != nullptr)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if (Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
		}
	}
}

