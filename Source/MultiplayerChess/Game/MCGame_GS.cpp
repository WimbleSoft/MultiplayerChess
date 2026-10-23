#include "Game/MCGame_GS.h"
#include "Game/MCGame_PC.h"
#include "Game/MCGame_PS.h"
#include "MultiplayerChess.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

void AMCGame_GS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCGame_GS, PlayerTurnIndex);
	DOREPLIFETIME(AMCGame_GS, bGameOver);
	DOREPLIFETIME(AMCGame_GS, WinnerIndex);
	DOREPLIFETIME(AMCGame_GS, bShowMovesToOpponent);
}

void AMCGame_GS::ProcessObjectBeatingPoints(AMCGame_PC* BeatingPlayer, int32 PointsToAdd, AMCGame_PC* BeatenPlayer)
{
	if (const FS_PlayerInfo* Found = PlayerInfos.Find(BeatingPlayer))
	{
		FS_PlayerInfo Info = *Found;
		Info.PlayerPoints += PointsToAdd;
		PlayerInfos.Add(BeatingPlayer, Info);
		if (BeatingPlayer)
		{
			BeatingPlayer->SetPlayerInfo(PlayerInfos.FindRef(BeatingPlayer));
		}
	}

	if (const FS_PlayerInfo* Found = PlayerInfos.Find(BeatenPlayer))
	{
		FS_PlayerInfo Info = *Found;
		Info.PlayerPoints -= PointsToAdd;
		PlayerInfos.Add(BeatenPlayer, Info);
		if (BeatenPlayer)
		{
			BeatenPlayer->SetPlayerInfo(PlayerInfos.FindRef(BeatenPlayer));
		}
	}
}

TArray<FS_PlayerInfo> AMCGame_GS::GetPlayerInfos()
{
	TArray<FS_PlayerInfo> TempPlayerInfos;
	for (APlayerState* Element : PlayerArray)
	{
		AMCGame_PS* GamePS = Cast<AMCGame_PS>(Element);
		if (!GamePS)
		{
			MC_ACCESSED_NONE("AsBP Game PS");
			TempPlayerInfos.Add(FS_PlayerInfo());
			continue;
		}
		TempPlayerInfos.Add(GamePS->PlayerInfo);
	}
	return TempPlayerInfos;
}

void AMCGame_GS::SetNewTurnIndex()
{
	PlayerTurnIndex = (PlayerTurnIndex == 0) ? 1 : 0;
}

void AMCGame_GS::Multicast_ShowMessage_Implementation(const FString& Message, FLinearColor Color, float Duration)
{
	UKismetSystemLibrary::PrintString(this, Message, true, true, Color, Duration);
}
