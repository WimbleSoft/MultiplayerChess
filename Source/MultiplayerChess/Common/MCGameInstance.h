#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/EngineBaseTypes.h"
#include "FindSessionsCallbackProxy.h"
#include "Common/S_MatchSettings.h"
#include "MCGameInstance.generated.h"

class AMCBasePC;
class UMCSave;
class UUserWidget;

// Port of BP_GameInstance.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API UMCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMCGameInstance();

	// Keep UGameInstance's own JoinSession overloads visible next to the Blueprint-callable one below.
	using UGameInstance::JoinSession;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBlueprintSessionResult> FoundSessions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBlueprintSessionResult JoinedSession;

	// Rules of the match the host is setting up. Filled from the host's save game and passed to the Game map as
	// URL options by ServerTravel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FS_MatchSettings MatchSettings;

	// Copies the match settings from a save game (the host's preferences).
	UFUNCTION(BlueprintCallable)
	void LoadMatchSettingsFrom(UMCSave* Save);

	// For the match settings widget. Set also stores the choice in the host's save game as the new preference.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetShowMovesToOpponent() const { return MatchSettings.ShowMovesToOpponent; }

	UFUNCTION(BlueprintCallable)
	void SetShowMovesToOpponent(bool bShow);

	// UI_Kicked; the widget stays a Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> KickedWidgetClass;

	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable)
	void CreateSession(AMCBasePC* PlayerController);

	UFUNCTION(BlueprintCallable)
	void FindSessions(AMCBasePC* PlayerController, UUserWidget* UI_SessionList);

	UFUNCTION(BlueprintCallable)
	void DestroySession(APlayerController* PlayerController, const FString& Reason);

	UFUNCTION(BlueprintCallable)
	void JoinSession(APlayerController* PlayerController, const FBlueprintSessionResult& SearchResult);

	UFUNCTION(BlueprintCallable)
	void ServerTravel(const FString& LevelName, APlayerController* SpecificPlayer);

	UFUNCTION(BlueprintCallable)
	void ChangeSessionDetails(APlayerController* PlayerController);

private:
	UFUNCTION()
	void OnCreateSessionSuccess();
	UFUNCTION()
	void OnCreateSessionFailure();
	UFUNCTION()
	void OnFindSessionsSuccess(const TArray<FBlueprintSessionResult>& Results);
	UFUNCTION()
	void OnFindSessionsFailure(const TArray<FBlueprintSessionResult>& Results);
	UFUNCTION()
	void OnDestroySessionSuccess();
	UFUNCTION()
	void OnDestroySessionFailure();
	UFUNCTION()
	void OnJoinSessionSuccess();
	UFUNCTION()
	void OnJoinSessionFailure();

	void HandleNetworkError(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelError(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	// Inputs that the latent Blueprint nodes held on to until their callbacks fired.
	TWeakObjectPtr<UUserWidget> PendingSessionList;
	TWeakObjectPtr<APlayerController> PendingDestroyController;
	FString PendingDestroyReason;

	FDelegateHandle NetworkFailureHandle;
	FDelegateHandle TravelFailureHandle;
};
