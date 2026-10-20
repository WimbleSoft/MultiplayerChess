#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCKnight.generated.h"

// Port of BP_Knight.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCKnight : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCKnight();

	virtual void CalcPossibility() override;
	virtual bool CalcCheckingKing() override;
	virtual bool CanCheckToSlot(FName InSlotName) override;

	// Output pin name kept from the Blueprint ("KingPossibilities").
	UFUNCTION(BlueprintCallable)
	TArray<FName> GetKnightPossibilities() const;

private:
	// Port of the CheckNextPosition macro. Returns true for the "addSlot" output, false for "then".
	bool CheckNextPosition(int32 Col, int32 Row, FName& OutSlot) const;
};
