#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCQueen.generated.h"

// Port of BP_Queen.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCQueen : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCQueen();

	virtual void CalcPossibility() override;
	virtual bool CalcCheckingKing() override;
	virtual bool CanCheckToSlot(FName InSlotName) override;
};
