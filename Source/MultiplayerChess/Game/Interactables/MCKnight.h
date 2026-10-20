#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCKnight.generated.h"

// Port of BP_Knight. Movement comes from FMCChessRules through AMCChessObjectBase::CalcPossibility.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCKnight : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCKnight();
};
