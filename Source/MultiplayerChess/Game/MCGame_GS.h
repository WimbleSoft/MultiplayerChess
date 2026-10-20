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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	int32 PlayerTurnIndex = 0;

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
};
