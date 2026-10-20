#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCQueen.generated.h"

// Port of BP_Queen. Movement comes from FMCChessRules through AMCChessObjectBase::CalcPossibility.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCQueen : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCQueen();
};
