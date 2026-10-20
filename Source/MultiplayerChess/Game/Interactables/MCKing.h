#pragma once

#include "CoreMinimal.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "MCKing.generated.h"

// Port of BP_King.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCKing : public AMCChessObjectBase
{
	GENERATED_BODY()

public:
	AMCKing();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsKingChecked)
	bool IsKingChecked = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void CalcPossibility() override;
	virtual bool CanCheckToSlot(FName InSlotName) override;

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetKingSurroundings() const;

	UFUNCTION(BlueprintCallable)
	void MarkChecked(bool IsChecked);

	UFUNCTION()
	void OnRep_IsKingChecked();

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void S_MarkChecked(bool IsChecked);

	// Empty in the Blueprint.
	UFUNCTION(BlueprintCallable)
	void M_MarkChecked(bool IsChecked);
};
