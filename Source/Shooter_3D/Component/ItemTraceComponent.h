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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItemCurrentFrame;


public:
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE bool GetShouldTraceForItems() const { return bShouldTraceForItems; }
	FORCEINLINE AItem* GetTraceHitItemCurrentFrame() const { return TraceHitItemCurrentFrame; }
	void IncrementOverlappedItemCount(int8 Amount);
	void TraceForItems(FHitResult ItemTraceResult);
	void ResetTraceHitItems();
};
