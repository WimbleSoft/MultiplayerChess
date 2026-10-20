#include "Lobby/MCLobby_PS.h"
#include "Common/MCBasePC.h"
#include "Common/MCSave.h"
#include "MultiplayerChess.h"
#include "Net/UnrealNetwork.h"

void AMCLobby_PS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCLobby_PS, R_SaveGame);
	DOREPLIFETIME(AMCLobby_PS, PlayerInfo);
}

void AMCLobby_PS::SaveGame()
{
	AMCBasePC* BasePC = Cast<AMCBasePC>(GetPlayerController());
	if (!BasePC)
	{
		MC_ACCESSED_NONE("AsBP Base PC");
		return;
	}
	BasePC->SaveGame();
}

void AMCLobby_PS::LoadGame()
{
	AMCBasePC* BasePC = Cast<AMCBasePC>(GetPlayerController());
	if (!BasePC)
	{
		MC_ACCESSED_NONE("AsBP Base PC");
		return;
	}
	BasePC->LoadGame();
}

void AMCLobby_PS::Client_SaveGame_Implementation()
{
	SaveGame();
}

void AMCLobby_PS::Server_LoadGame_Implementation()
{
	LoadGame();
}

void AMCLobby_PS::SwichPlayerReady()
{
	PlayerInfo.IsReady = !PlayerInfo.IsReady;
}

void AMCLobby_PS::Multicast_SetPlayerRole_Implementation(int32 PlayerIndex)
{
	switch (PlayerIndex)
	{
	case 0:
		PlayerInfo.PlayerRole = E_PlayerRole::HOME;
		break;
	case 1:
		PlayerInfo.PlayerRole = E_PlayerRole::AWAY;
		break;
	default:
		PlayerInfo.PlayerRole = E_PlayerRole::SPECTATOR;
		break;
	}
}

void AMCLobby_PS::Server_SetPlayerRole_Implementation(int32 PlayerIndex)
{
	Multicast_SetPlayerRole(PlayerIndex);
}
