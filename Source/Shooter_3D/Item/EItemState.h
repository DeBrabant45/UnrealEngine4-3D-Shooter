#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemState : uint8
{
	Eis_Pickup UMETA(DisplayName = "Pickup"),
	Eis_EquipInterping UMETA(DisplayName = "EquipInterping"),
	Eis_PickedUp UMETA(DisplayName = "PickedUp"),
	Eis_Equipped UMETA(DisplayName = "Equipped"),
	Eis_Falling UMETA(DisplayName = "Falling"),
	Eis_Max UMETA(DisplayName = "DefaultMax")
};