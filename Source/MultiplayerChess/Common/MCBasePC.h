#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MCBasePC.generated.h"

class UMCSave;
class UMCGameInstance;
class UUserWidget;

// Port of BP_BasePC.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCBasePC : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMCSave> R_SaveGame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMCGameInstance> GameInstance;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void RemoveAllWidgetsFromPlayer();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void DestroySession(APlayerController* PlayerController);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void KickPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	void ShowWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable)
	void HideWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	// For the match settings widget: read and change the "show moves to opponent" setting in the save game.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetShowMovesToOpponent() const;

	UFUNCTION(BlueprintCallable)
	void SetShowMovesToOpponent(bool bShow);
};
