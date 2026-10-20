#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCBishop.generated.h"

// Port of BP_Bishop.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCBishop : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCBishop();

	virtual void CalcPossibility() override;
	virtual bool CalcCheckingKing() override;
	virtual bool CanCheckToSlot(FName InSlotName) override;
};
