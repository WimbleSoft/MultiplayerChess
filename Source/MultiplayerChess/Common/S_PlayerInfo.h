#pragma once

#include "CoreMinimal.h"
#include "Common/E_PlayerRole.h"
#include "S_PlayerInfo.generated.h"

// Port of /Game/Blueprints/Common/S_PlayerInfo.
USTRUCT(BlueprintType)
struct FS_PlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName = NSLOCTEXT("S_PlayerInfo", "DefaultName", "Default Name");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerPoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	E_PlayerRole PlayerRole = E_PlayerRole::SPECTATOR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsReady = false;
};
