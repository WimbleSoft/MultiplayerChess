#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCPawn.generated.h"

// Port of BP_Pawn (the chess pawn, not an Unreal APawn). Movement comes from FMCChessRules through AMCChessObjectBase::CalcPossibility.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCPawn : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCPawn();
};
