#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCRook.generated.h"

// Port of BP_Rook.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCRook : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCRook();

	virtual void CalcPossibility() override;
	virtual bool CalcCheckingKing() override;
	virtual bool CanCheckToSlot(FName InSlotName) override;
};
