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

// Port of BP_Game_GM. Moves are validated and executed here with FMCChessRules.
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

	// Piece classes used by ReplacePawnWithNewObject.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<E_ObjectType, TSubclassOf<AMCChessObjectBase>> ObjectClasses;

	// Player who still has to pick the piece for a promoted pawn, and the pawn's slot. The turn waits for it.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMCGame_PC> PendingPromotionPlayer;

	UPROPERTY(BlueprintReadOnly)
	FName PendingPromotionSlot;

	// Whether the opponent also sees the selected piece and its possible moves. Read from the host's save game
	// (match settings) when the game starts; the URL option ?ShowOpponentMoves=0/1 overrides it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMovesToOpponent = false;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// Sends the piece's selection state and legal moves to its owner, and to the opponent when allowed.
	UFUNCTION(BlueprintCallable)
	void BroadcastSelection(AMCGame_PC* SelectingPlayer, AMCChessObjectBase* Piece);

	virtual void OnConstruction(const FTransform& Transform) override;

	// Never called by the engine for a GameModeBase; kept as a plain function like in the Blueprint.
	UFUNCTION(BlueprintCallable)
	bool ReadyToStartMatch();

	// True when it is this player's turn, the game is running and no promotion is waiting.
	UFUNCTION(BlueprintCallable)
	bool CanPlayerAct(AMCGame_PC* Player, FString& OutReason) const;

	// Validates the move with the chess rules and performs it (captures, castling, en passant, promotion).
	UFUNCTION(BlueprintCallable)
	bool TryMovePiece(AMCGame_PC* Player, AMCChessObjectBase* Piece, FName TargetSlot);

	// Replaces the waiting pawn with the chosen piece and passes the turn.
	UFUNCTION(BlueprintCallable)
	void CompletePromotion(AMCGame_PC* Player, FName Slot, E_ObjectType NewObjectType);

	UFUNCTION(BlueprintCallable)
	void MoveObjectToEmptySlot(AMCTableSlot* NewSlot, AMCChessObjectBase* SelectedObject);

	UFUNCTION(BlueprintCallable)
	void ResetSelectedObject(AMCGame_PC* Player);

	UFUNCTION(BlueprintCallable)
	void BeatObject(AMCGame_PC* BeaterPlayer, AMCChessObjectBase* ObjectToBeat, AMCGame_PC* BeatenPlayer);

	UFUNCTION(BlueprintCallable)
	AMCGame_PC* GetOpponentPlayer(AMCGame_PC* ItemToFind);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCGame_GS* GetGS() const;

	// "PIE ONLY" custom event; nothing calls it.
	UFUNCTION(BlueprintCallable)
	void SpawnPlayer(AController* Player);

	// Server event of the Blueprint. The game mode only exists on the server, so it is a plain call here.
	UFUNCTION(BlueprintCallable)
	void Server_OnLogin(UObject* Object);

	UFUNCTION(BlueprintCallable)
	void ReplacePawnWithNewObject(FName Slot, AMCChessObjectBase* PawnObject, E_ObjectType NewObjectType, bool IsAway);

	// Marks each king as checked or not for the current position.
	UFUNCTION(BlueprintCallable)
	void UpdateCheckState();

protected:
	// Event K2_PostLogin
	virtual void OnPostLogin(AController* NewPlayer) override;
	// Event K2_OnSwapPlayerControllers
	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

	AMCChessTable* GetTable() const;

	// Passes the turn, updates check marks and ends the game on checkmate or stalemate.
	void FinishTurn();
	void EndGame(int32 WinnerIndex, const FString& Message);
};
