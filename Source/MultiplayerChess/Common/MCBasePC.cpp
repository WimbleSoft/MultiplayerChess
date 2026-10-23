#include "Common/MCBasePC.h"
#include "Common/MCGameInstance.h"
#include "Common/MCSave.h"
#include "MultiplayerChess.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void AMCBasePC::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GameInstance = Cast<UMCGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Cannot cast game instance in Lobby PC"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
	}
}

void AMCBasePC::RemoveAllWidgetsFromPlayer_Implementation()
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);
}

void AMCBasePC::DestroySession_Implementation(APlayerController* PlayerController)
{
	if (!GameInstance)
	{
		MC_ACCESSED_NONE("GameInstance");
		return;
	}
	GameInstance->DestroySession(PlayerController, TEXT("Season is closed by server."));
}

void AMCBasePC::KickPlayer_Implementation(APlayerController* PlayerController)
{
	if (!GameInstance)
	{
		MC_ACCESSED_NONE("GameInstance");
		return;
	}
	GameInstance->DestroySession(PlayerController, TEXT("Kicked by server."));
}

void AMCBasePC::ShowWidget(UUserWidget* Widget)
{
	if (IsValid(Widget) && !Widget->IsInViewport())
	{
		Widget->AddToViewport(0);
	}
}

void AMCBasePC::HideWidget(UUserWidget* Widget)
{
	if (IsValid(Widget) && Widget->IsInViewport())
	{
		Widget->RemoveFromParent();
	}
}

void AMCBasePC::SaveGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("Save"), 0))
	{
		R_SaveGame = Cast<UMCSave>(UGameplayStatics::CreateSaveGameObject(UMCSave::StaticClass()));
	}
	UGameplayStatics::SaveGameToSlot(R_SaveGame, TEXT("Save"), 0);
}

void AMCBasePC::LoadGame()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Save"), 0))
	{
		if (UMCSave* Save = Cast<UMCSave>(UGameplayStatics::LoadGameFromSlot(TEXT("Save"), 0)))
		{
			R_SaveGame = Save;
		}
	}
}

