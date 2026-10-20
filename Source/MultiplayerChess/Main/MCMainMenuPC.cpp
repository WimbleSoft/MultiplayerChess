#include "Main/MCMainMenuPC.h"
#include "Common/MCGameInstance.h"
#include "MultiplayerChess.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AMCMainMenuPC::AMCMainMenuPC()
{
	bShowMouseCursor = true;
	MainMenuWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UI/Main/UI_MainMenu.UI_MainMenu_C")));
}

void AMCMainMenuPC::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GameInst = Cast<UMCGameInstance>(GetGameInstance());
	if (!GameInst)
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Cannot cast to game instance in MainMenuPC"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
	}
}

void AMCMainMenuPC::BeginPlay()
{
	Super::BeginPlay();

	CreateWidget();
	LoadGame();
}

void AMCMainMenuPC::CreateSession()
{
	if (!GameInst)
	{
		MC_ACCESSED_NONE("GameInst");
		return;
	}
	GameInst->CreateSession(this);
}

void AMCMainMenuPC::FindSessions(UUserWidget* UI_SessionList)
{
	if (!GameInst)
	{
		MC_ACCESSED_NONE("GameInst");
		return;
	}
	GameInst->FindSessions(this, UI_SessionList);
}

void AMCMainMenuPC::JoinSession(const FBlueprintSessionResult& SearchResult)
{
	if (!GameInst)
	{
		MC_ACCESSED_NONE("GameInst");
		return;
	}
	GameInst->JoinSession(this, SearchResult);
}

void AMCMainMenuPC::CreateWidget()
{
	if (UClass* WidgetClass = MainMenuWidgetClass.LoadSynchronous())
	{
		R_WBP_MainMenu = ::CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(this, 0), WidgetClass);
	}
	ShowWidget(R_WBP_MainMenu);
}
