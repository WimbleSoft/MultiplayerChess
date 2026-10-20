#pragma once

#include "CoreMinimal.h"
#include "Common/MCBasePC.h"
#include "Common/S_PlayerInfo.h"
#include "MCLobby_PC.generated.h"

class UUserWidget;

// Port of BP_Lobby_PC.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCLobby_PC : public AMCBasePC
{
	GENERATED_BODY()

public:
	AMCLobby_PC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> UI_LobbyMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<FS_PlayerInfo> PlayerInfos;

	// UI_LobbyMenu; the widget stays a Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> LobbyMenuWidgetClass;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	FS_PlayerInfo GetPlayerInfo();

	UFUNCTION(BlueprintCallable)
	void LoadPlayerState();

	UFUNCTION(BlueprintCallable)
	void Ready();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_Init();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RefreshPlayerCards(const TArray<FS_PlayerInfo>& InPlayerInfos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void LoadPlayerState_Server();

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void SwitchReady_OnServer();

	// Named "Multicast" in the Blueprint, but it is a reliable Server event there too.
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SwitchReady_Multicast();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetPlayerInfos();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SetPlayerInfos(const TArray<FS_PlayerInfo>& InPlayerInfos);
};
