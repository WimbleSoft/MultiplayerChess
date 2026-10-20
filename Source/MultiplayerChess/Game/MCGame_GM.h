#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Common/E_ObjectType.h"
#include "MCGame_GM.generated.h"

class AMCChessObjectBase;
class AMCChessTable;
class AMCGame_GS;
class AMCGame_PC;
class AMCTableSlot;

// Port of BP_Game_GM.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCGame_GM : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMCGame_GM();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<AMCGame_PC>> PlayerControllers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AMCChessTable> R_Table;

	// Piece classes used by ReplacePawnWithNewObject (the Blueprint picked BP_Bishop_C, BP_King_C, ... with a Select node).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<E_ObjectType, TSubclassOf<AMCChessObjectBase>> ObjectClasses;

	virtual void OnConstruction(const FTransform& Transform) override;

	// Never called by the engine for a GameModeBase; kept as a plain function like in the Blueprint.
	UFUNCTION(BlueprintCallable)
	bool ReadyToStartMatch();

	UFUNCTION(BlueprintCallable)
	void MoveObjectToEmptySlot(AMCTableSlot* NewSlot, AMCChessObjectBase* SelectedObject);

	UFUNCTION(BlueprintCallable)
	void ResetSelectedObject(AMCGame_PC* Player);

	UFUNCTION(BlueprintCallable)
	void BeatObject(AMCGame_PC* BeaterPlayer, AMCChessObjectBase* ObjectToBeat, AMCGame_PC* BeatenPlayer);

	UFUNCTION(BlueprintCallable)
	AMCGame_PC* GetOpponentPlayer(AMCGame_PC* ItemToFind);

	UFUNCTION(BlueprintCallable)
	void ObjectClicked(AMCGame_PC* Player, AMCChessObjectBase* ObjectToBeat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCGame_GS* GetGS() const;

	UFUNCTION(BlueprintCallable)
	void SlotClicked(AMCGame_PC* Player, AMCTableSlot* NewSlot, AMCChessObjectBase* SelectedObject);

	// "PIE ONLY" custom event; nothing calls it.
	UFUNCTION(BlueprintCallable)
	void SpawnPlayer(AController* Player);

	// Server event of the Blueprint. The game mode only exists on the server, so it is a plain call here.
	UFUNCTION(BlueprintCallable)
	void Server_OnLogin(UObject* Object);

	UFUNCTION(BlueprintCallable)
	void ReplacePawnWithNewObject(FName Slot, AMCChessObjectBase* PawnObject, E_ObjectType NewObjectType, bool IsAway);

	UFUNCTION(BlueprintCallable)
	void ProcessCheckingPossibility(AMCChessObjectBase* AttackerObject);

protected:
	// Event K2_PostLogin
	virtual void OnPostLogin(AController* NewPlayer) override;
	// Event K2_OnSwapPlayerControllers
	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;
};
