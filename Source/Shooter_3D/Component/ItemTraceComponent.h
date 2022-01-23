#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemTraceComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_3D_API UItemTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemTraceComponent();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	bool bShouldTraceForItems;
	UPROPERTY()
	int8 OverlappedItemCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;


public:
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE bool GetShouldTraceForItems() const { return bShouldTraceForItems; }
	void IncrementOverlappedItemCount(int8 Amount);
	void TraceForItems(FHitResult ItemTraceResult);
};
