#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Common/S_PlayerInfo.h"
#include "MCLobby_GM.generated.h"

// Port of BP_Lobby_GM.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCLobby_GM : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMCLobby_GM();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<APlayerController>> PlayerControllers;

	UFUNCTION(BlueprintCallable)
	TArray<FS_PlayerInfo> GetPlayerInfos();

	UFUNCTION(BlueprintCallable)
	void Server_StartMatch();

	UFUNCTION(BlueprintCallable)
	void FillPlayerCards();

	UFUNCTION(BlueprintCallable)
	void SetPlayerRole(APlayerController* NewPlayer);

protected:
	// Event K2_PostLogin
	virtual void OnPostLogin(AController* NewPlayer) override;
	// Event K2_OnLogout
	virtual void Logout(AController* Exiting) override;
	// Event K2_OnSwapPlayerControllers
	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;
};
