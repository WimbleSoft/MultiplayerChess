#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Common/S_PlayerInfo.h"
#include "MCGame_GS.generated.h"

class AMCGame_PC;

// Port of BP_Game_GS.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCGame_GS : public AGameStateBase
{
	GENERATED_BODY()

public:
	// GlobalPlayerIndex of the player to move: 0 = HOME (white, starts), 1 = AWAY.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	int32 PlayerTurnIndex = 0;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bGameOver = false;

	// Match setting from the game mode, so clients can show it (e.g. "moves are hidden in this match").
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bShowMovesToOpponent = false;

	// GlobalPlayerIndex of the winner, -1 for a draw.
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 WinnerIndex = -1;

	// Never used by the Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, int32> PlayerPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<AMCGame_PC>, FS_PlayerInfo> PlayerInfos;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void ProcessObjectBeatingPoints(AMCGame_PC* BeatingPlayer, int32 PointsToAdd, AMCGame_PC* BeatenPlayer);

	UFUNCTION(BlueprintCallable)
	TArray<FS_PlayerInfo> GetPlayerInfos();

	UFUNCTION(BlueprintCallable)
	void SetNewTurnIndex();

	// Shows a message on every player's screen.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_ShowMessage(const FString& Message, FLinearColor Color, float Duration);
};
