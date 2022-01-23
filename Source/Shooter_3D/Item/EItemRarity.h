#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Eir_Damaged UMETA(DisplayName = "Damaged"),
	Eir_Common UMETA(DisplayName = "Common"),
	Eir_Uncommon UMETA(DisplayName = "Uncommon"),
	Eir_Rare UMETA(DisplayName = "Rare"),
	Eir_Legendary UMETA(DisplayName = "Legendary"),
	Eir_Max UMETA(DisplayName = "DefaultMax")
};