#pragma once

#include "CoreMinimal.h"
#include "E_PlayerRole.generated.h"

// Port of /Game/Blueprints/Common/E_PlayerRole. Order matches the Blueprint enum (NewEnumerator0..2).
UENUM(BlueprintType)
enum class E_PlayerRole : uint8
{
	HOME,
	AWAY,
	SPECTATOR
};
