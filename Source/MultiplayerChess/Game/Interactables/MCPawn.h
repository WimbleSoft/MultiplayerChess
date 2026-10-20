#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCPawn.generated.h"

// Port of BP_Pawn (the chess pawn, not an Unreal APawn).
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCPawn : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCPawn();

	virtual void CalcPossibility() override;
	virtual bool CalcCheckingKing() override;
	virtual bool CanCheckToSlot(FName InSlotName) override;

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetPawnPossibilities() const;
};
