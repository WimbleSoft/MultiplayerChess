#include "Lobby/MCLobby_PC.h"
#include "Common/MCCommonFunctions.h"
#include "Lobby/MCLobby_GM.h"
#include "Lobby/MCLobby_PS.h"
#include "MultiplayerChess.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AMCLobby_PC::AMCLobby_PC()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	LobbyMenuWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UI/Lobby/UI_LobbyMenu.UI_LobbyMenu_C")));
}

void AMCLobby_PC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCLobby_PC, PlayerInfos);
}

FS_PlayerInfo AMCLobby_PC::GetPlayerInfo()
{
	if (AMCLobby_PS* LobbyPS = Cast<AMCLobby_PS>(PlayerState))
	{
		return LobbyPS->PlayerInfo;
	}
	UKismetSystemLibrary::PrintText(this, INVTEXT("Cannot cast to LobbyPS in GetPlayerInfo function of LobbyPC"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
	return FS_PlayerInfo();
}

void AMCLobby_PC::LoadPlayerState()
{
	// Switch Has Authority: Remote first asks the server, then both paths call LoadGame on the player state.
	if (!HasAuthority())
	{
		LoadPlayerState_Server();
	}
	AMCLobby_PS* LobbyPS = Cast<AMCLobby_PS>(PlayerState);
	if (!LobbyPS)
	{
		MC_ACCESSED_NONE("AsBP Lobby PS");
		return;
	}
	LobbyPS->LoadGame();
}

void AMCLobby_PC::Ready()
{
	if (UKismetSystemLibrary::IsServer(this))
	{
		if (AMCLobby_PS* LobbyPS = Cast<AMCLobby_PS>(PlayerState))
		{
			LobbyPS->PlayerInfo.IsReady = true;
		}
		else
		{
			MC_ACCESSED_NONE("AsBP Lobby PS");
		}
		if (AMCLobby_GM* LobbyGM = Cast<AMCLobby_GM>(UGameplayStatics::GetGameMode(this)))
		{
			LobbyGM->Server_StartMatch();
		}
	}
	else
	{
		SwitchReady_OnServer();
	}
}

void AMCLobby_PC::Client_Init_Implementation()
{
	if (UClass* WidgetClass = LobbyMenuWidgetClass.LoadSynchronous())
	{
		UI_LobbyMenu = ::CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(this, 0), WidgetClass);
	}
	ShowWidget(UI_LobbyMenu);
}

void AMCLobby_PC::Client_RefreshPlayerCards_Implementation(const TArray<FS_PlayerInfo>& InPlayerInfos)
{
	PlayerInfos = InPlayerInfos;
	if (!UI_LobbyMenu)
	{
		MC_ACCESSED_NONE("UI_LobbyMenu");
		return;
	}
	UMCCommonFunctions::CallFunctionByName(UI_LobbyMenu, TEXT("ClearPlayerCards"));
	for (int32 Index = 0; Index < PlayerInfos.Num(); ++Index)
	{
		UMCCommonFunctions::CallFunctionByName(UI_LobbyMenu, TEXT("AddNewPlayer"), TEXT("PlayerIndex"), Index);
	}
}

void AMCLobby_PC::LoadPlayerState_Server_Implementation()
{
	LoadPlayerState();
}

void AMCLobby_PC::SwitchReady_OnServer_Implementation()
{
	SwitchReady_Multicast();
}

void AMCLobby_PC::SwitchReady_Multicast_Implementation()
{
	AMCLobby_PS* LobbyPS = Cast<AMCLobby_PS>(PlayerState);
	if (!LobbyPS)
	{
		MC_ACCESSED_NONE("AsBP Lobby PS");
		return;
	}
	LobbyPS->SwichPlayerReady();
}

void AMCLobby_PC::Server_SetPlayerInfos_Implementation()
{
	if (AMCLobby_GM* LobbyGM = Cast<AMCLobby_GM>(UGameplayStatics::GetGameMode(this)))
	{
		PlayerInfos = LobbyGM->GetPlayerInfos();
	}
	else
	{
		MC_ACCESSED_NONE("AsBP Lobby GM");
		PlayerInfos.Reset();
	}
	Multicast_SetPlayerInfos(PlayerInfos);
}

void AMCLobby_PC::Multicast_SetPlayerInfos_Implementation(const TArray<FS_PlayerInfo>& InPlayerInfos)
{
	PlayerInfos = InPlayerInfos;
}
