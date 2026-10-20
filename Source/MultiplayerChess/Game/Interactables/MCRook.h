#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCRook.generated.h"

// Port of BP_Rook. Movement comes from FMCChessRules through AMCChessObjectBase::CalcPossibility.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCRook : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCRook();
};
