#pragma once

#include "CoreMinimal.h"
#include "E_ObjectType.generated.h"

// Port of /Game/Blueprints/Common/E_ObjectType. Order matches the Blueprint enum (NewEnumerator0..5).
UENUM(BlueprintType)
enum class E_ObjectType : uint8
{
	Pawn,
	King,
	Queen,
	Rook,
	Knight,
	Bishop
};
