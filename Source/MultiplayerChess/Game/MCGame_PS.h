#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Common/S_PlayerInfo.h"
#include "MCGame_PS.generated.h"

class AMCGame_PC;

// Port of BP_Game_PS.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCGame_PS : public APlayerState
{
	GENERATED_BODY()

public:
	AMCGame_PS();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FS_PlayerInfo PlayerInfo;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCGame_PC* GetPC() const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetPlayerInfo(const FS_PlayerInfo& InPlayerInfo);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SetPlayerInfo(const FS_PlayerInfo& InPlayerInfo);
};
