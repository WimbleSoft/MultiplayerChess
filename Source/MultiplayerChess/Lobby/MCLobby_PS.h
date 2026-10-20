#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Common/S_PlayerInfo.h"
#include "MCLobby_PS.generated.h"

class UMCSave;

// Port of BP_Lobby_PS.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCLobby_PS : public APlayerState
{
	GENERATED_BODY()

public:
	// Replicated like in the Blueprint, although a SaveGame object can't actually be sent over the network.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TObjectPtr<UMCSave> R_SaveGame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FS_PlayerInfo PlayerInfo;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_SaveGame();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_LoadGame();

	UFUNCTION(BlueprintCallable)
	void SwichPlayerReady();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SetPlayerRole(int32 PlayerIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetPlayerRole(int32 PlayerIndex);
};
