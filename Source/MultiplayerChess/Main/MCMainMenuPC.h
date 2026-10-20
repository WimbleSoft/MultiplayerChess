#pragma once

#include "CoreMinimal.h"
#include "Common/MCBasePC.h"
#include "FindSessionsCallbackProxy.h"
#include "MCMainMenuPC.generated.h"

class UUserWidget;

// Port of BP_MainMenuPC.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCMainMenuPC : public AMCBasePC
{
	GENERATED_BODY()

public:
	AMCMainMenuPC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBlueprintSessionResult> FoundSessions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> R_WBP_MainMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> R_WBP_UISettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> R_WBP_UIMultiplayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> R_WBP_UILegal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMCGameInstance> GameInst;

	// UI_MainMenu; the widget stays a Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> MainMenuWidgetClass;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void CreateSession();

	UFUNCTION(BlueprintCallable)
	void FindSessions(UUserWidget* UI_SessionList);

	UFUNCTION(BlueprintCallable)
	void JoinSession(const FBlueprintSessionResult& SearchResult);

	// Custom event "CreateWidget" of the Blueprint.
	UFUNCTION(BlueprintCallable)
	void CreateWidget();
};
