#include "ItemTraceComponent.h"
#include "CrosshairComponent.h"
#include "Components/WidgetComponent.h"
#include "Shooter_3D/Character/ShooterCharacter.h"
#include "Shooter_3D/Item/Item.h"

UItemTraceComponent::UItemTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UItemTraceComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UItemTraceComponent::IncrementOverlappedItemCount(int8 Amount)
{
	if (GetOverlappedItemCount() + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

void UItemTraceComponent::TraceForItems(FHitResult ItemTraceResult)
{
	if (bShouldTraceForItems)
	{
		if (ItemTraceResult.bBlockingHit)
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.Actor);
			if (HitItem)
			{
				if (HitItem->GetPickupWidget())
				{
					HitItem->GetPickupWidget()->SetVisibility(true);
				}
			}

			if (TraceHitItemLastFrame != nullptr)
			{
				if (HitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = HitItem;
		}
	}
	else if (TraceHitItemLastFrame != nullptr)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}