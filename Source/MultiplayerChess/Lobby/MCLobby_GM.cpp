#include "Lobby/MCLobby_GM.h"
#include "Common/MCBasePC.h"
#include "Common/MCGameInstance.h"
#include "Lobby/MCLobby_GS.h"
#include "Lobby/MCLobby_PC.h"
#include "Lobby/MCLobby_PS.h"
#include "MultiplayerChess.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"

AMCLobby_GM::AMCLobby_GM()
{
	DefaultPawnClass = APawn::StaticClass();
	PlayerControllerClass = AMCLobby_PC::StaticClass();
	PlayerStateClass = AMCLobby_PS::StaticClass();
	GameStateClass = AMCLobby_GS::StaticClass();
}

TArray<FS_PlayerInfo> AMCLobby_GM::GetPlayerInfos()
{
	TArray<FS_PlayerInfo> TempPlayerInfos;
	for (APlayerController* Element : PlayerControllers)
	{
		AMCLobby_PC* LobbyPC = Cast<AMCLobby_PC>(Element);
		AMCLobby_PS* LobbyPS = LobbyPC ? Cast<AMCLobby_PS>(LobbyPC->PlayerState) : nullptr;
		if (!LobbyPS)
		{
			MC_ACCESSED_NONE("AsBP Lobby PS");
			TempPlayerInfos.Add(FS_PlayerInfo());
			continue;
		}
		TempPlayerInfos.Add(LobbyPS->PlayerInfo);
	}
	return TempPlayerInfos;
}

void AMCLobby_GM::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	PlayerControllers.Add(Cast<APlayerController>(NewPlayer));
	if (AMCLobby_PC* LobbyPC = Cast<AMCLobby_PC>(NewPlayer))
	{
		LobbyPC->LoadPlayerState();
		SetPlayerRole(LobbyPC);
		LobbyPC->Client_Init();
		FillPlayerCards();
	}
}

void AMCLobby_GM::Server_StartMatch()
{
	if (!PlayerControllers.IsValidIndex(1) || !IsValid(PlayerControllers[1]))
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Away player doesn't exist."), true, false, FLinearColor(1.f, 0.f, 0.f, 1.f), 5.f);
		return;
	}

	AMCLobby_PS* AwayPS = Cast<AMCLobby_PS>(PlayerControllers[1]->PlayerState);
	if (!AwayPS)
	{
		MC_ACCESSED_NONE("AsBP Lobby PS");
	}
	if (AwayPS && AwayPS->PlayerInfo.IsReady)
	{
		for (APlayerController* Element : PlayerControllers)
		{
			if (AMCBasePC* BasePC = Cast<AMCBasePC>(Element))
			{
				BasePC->RemoveAllWidgetsFromPlayer();
			}
		}
		if (UMCGameInstance* GI = Cast<UMCGameInstance>(GetGameInstance()))
		{
			GI->ServerTravel(TEXT("Game"), nullptr);
		}
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Away player is not ready."), true, false, FLinearColor(1.f, 0.f, 0.f, 1.f), 5.f);
	}
}

void AMCLobby_GM::FillPlayerCards()
{
	for (APlayerController* Element : PlayerControllers)
	{
		AMCLobby_PC* LobbyPC = Cast<AMCLobby_PC>(Element);
		if (!LobbyPC)
		{
			MC_ACCESSED_NONE("AsBP Lobby PC");
			continue;
		}
		LobbyPC->Client_RefreshPlayerCards(GetPlayerInfos());
	}
}

void AMCLobby_GM::SetPlayerRole(APlayerController* NewPlayer)
{
	AMCLobby_PS* LobbyPS = NewPlayer ? Cast<AMCLobby_PS>(NewPlayer->PlayerState) : nullptr;
	if (!LobbyPS)
	{
		MC_ACCESSED_NONE("AsBP Lobby PS");
		return;
	}
	LobbyPS->Server_SetPlayerRole(PlayerControllers.Find(NewPlayer));
}

void AMCLobby_GM::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (AMCLobby_PC* LobbyPC = Cast<AMCLobby_PC>(Exiting))
	{
		PlayerControllers.Remove(LobbyPC);
		FillPlayerCards();
	}
}

void AMCLobby_GM::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
	Super::SwapPlayerControllers(OldPC, NewPC);

	if (AMCBasePC* BasePC = Cast<AMCBasePC>(OldPC))
	{
		BasePC->RemoveAllWidgetsFromPlayer();
	}
}
