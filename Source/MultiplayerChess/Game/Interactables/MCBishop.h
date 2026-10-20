#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCBishop.generated.h"

// Port of BP_Bishop. Movement comes from FMCChessRules through AMCChessObjectBase::CalcPossibility.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCBishop : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCBishop();
};
