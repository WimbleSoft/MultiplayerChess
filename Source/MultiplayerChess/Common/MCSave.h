#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Common/S_PlayerInfo.h"
#include "MCSave.generated.h"

// Port of BP_Save.
UCLASS(BlueprintType, Blueprintable)
class MULTIPLAYERCHESS_API UMCSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool UseCustomColors = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FLinearColor HomeColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FLinearColor AwayColor = FLinearColor(0.f, 0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool UseLan = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 PublicConnections = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FText ServerName = NSLOCTEXT("BP_Save", "DefaultServerName", "DefaultServerName");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 MaxServerResults = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FS_PlayerInfo PlayerInfo;
};
