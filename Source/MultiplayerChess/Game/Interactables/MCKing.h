#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCKing.generated.h"

// Port of BP_King. Movement comes from FMCChessRules through AMCChessObjectBase::CalcPossibility.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCKing : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCKing();

	// Set by the game mode after every move; the king's slot glows CheckedColor while it is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsKingChecked)
	bool IsKingChecked = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void MarkChecked(bool IsChecked);

	UFUNCTION()
	void OnRep_IsKingChecked();

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void S_MarkChecked(bool IsChecked);
};
