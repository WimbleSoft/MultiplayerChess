#pragma once

#include "CoreMinimal.h"
#include "S_MatchSettings.generated.h"

// Rules the host picks for a match. Kept on the game instance between maps, passed to the Game map as URL options
// (?ShowOpponentMoves=1) and read by the game mode in InitGame. The save game only remembers the host's last choice.
USTRUCT(BlueprintType)
struct FS_MatchSettings
{
	GENERATED_BODY()

	// Whether the opponent also sees the selected piece and its possible moves.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ShowMovesToOpponent = false;

	FString ToURLOptions() const
	{
		return FString::Printf(TEXT("?ShowOpponentMoves=%d"), ShowMovesToOpponent ? 1 : 0);
	}
};
