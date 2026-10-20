#include "Game/MCGame_GM.h"
#include "Common/MCCommonFunctions.h"
#include "Common/S_ObjectMap.h"
#include "Game/Interactables/MCBishop.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCKing.h"
#include "Game/Interactables/MCKnight.h"
#include "Game/Interactables/MCPawn.h"
#include "Game/Interactables/MCQueen.h"
#include "Game/Interactables/MCRook.h"
#include "Game/Interactables/MCTableSlot.h"
#include "Game/MCGamePawn.h"
#include "Game/MCGame_GS.h"
#include "Game/MCGame_PC.h"
#include "Game/MCGame_PS.h"
#include "Lobby/MCLobby_PC.h"
#include "MultiplayerChess.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AMCGame_GM::AMCGame_GM()
{
	DefaultPawnClass = AMCGamePawn::StaticClass();
	PlayerControllerClass = AMCGame_PC::StaticClass();
	PlayerStateClass = AMCGame_PS::StaticClass();
	GameStateClass = AMCGame_GS::StaticClass();

	ObjectClasses.Add(E_ObjectType::Bishop, AMCBishop::StaticClass());
	ObjectClasses.Add(E_ObjectType::King, AMCKing::StaticClass());
	ObjectClasses.Add(E_ObjectType::Knight, AMCKnight::StaticClass());
	ObjectClasses.Add(E_ObjectType::Pawn, AMCPawn::StaticClass());
	ObjectClasses.Add(E_ObjectType::Queen, AMCQueen::StaticClass());
	ObjectClasses.Add(E_ObjectType::Rook, AMCRook::StaticClass());
}

void AMCGame_GM::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMCChessTable::StaticClass(), OutActors);
	R_Table = OutActors.IsValidIndex(0) ? Cast<AMCChessTable>(OutActors[0]) : nullptr;
}

bool AMCGame_GM::ReadyToStartMatch()
{
	return GetNumPlayers() == 2;
}

void AMCGame_GM::MoveObjectToEmptySlot(AMCTableSlot* NewSlot, AMCChessObjectBase* SelectedObject)
{
	if (!SelectedObject)
	{
		MC_ACCESSED_NONE("SelectedObject");
		return;
	}

	AMCTableSlot* CurrentSlot = nullptr;
	bool bCurrentValid = false;
	SelectedObject->GetCurrentSlot(CurrentSlot, bCurrentValid);
	if (CurrentSlot)
	{
		CurrentSlot->Server_SetAttachedObject(nullptr);
	}
	else
	{
		MC_ACCESSED_NONE("GetCurrentSlot.Slot");
	}

	SelectedObject->AttachToActor(NewSlot, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true));
	if (NewSlot)
	{
		NewSlot->Server_SetAttachedObject(SelectedObject);
		SelectedObject->SetSocketName(NewSlot->SlotName);
	}
	else
	{
		MC_ACCESSED_NONE("NewSlot");
		SelectedObject->SetSocketName(NAME_None);
	}
	SelectedObject->Server_IsSelected(false);
	SelectedObject->S_SetIsMoved(true);
	ProcessCheckingPossibility(SelectedObject);
}

void AMCGame_GM::ResetSelectedObject(AMCGame_PC* Player)
{
	if (Player)
	{
		Player->SelectedChessObject = nullptr;
		Player->NewSelectedChessObject = nullptr;
	}
	else
	{
		MC_ACCESSED_NONE("Player");
	}
	if (AMCGame_GS* GS = GetGS())
	{
		GS->SetNewTurnIndex();
	}
}

void AMCGame_GM::BeatObject(AMCGame_PC* BeaterPlayer, AMCChessObjectBase* ObjectToBeat, AMCGame_PC* BeatenPlayer)
{
	if (AMCGame_GS* GS = GetGS())
	{
		GS->ProcessObjectBeatingPoints(BeaterPlayer, ObjectToBeat ? ObjectToBeat->Value : 0, BeatenPlayer);
	}
	if (ObjectToBeat)
	{
		ObjectToBeat->Destroy();
	}
}

AMCGame_PC* AMCGame_GM::GetOpponentPlayer(AMCGame_PC* ItemToFind)
{
	// Select(PC[0], PC[1], Index = (Find(ItemToFind) == 0))
	const int32 Index = PlayerControllers.Find(ItemToFind) == 0 ? 1 : 0;
	return PlayerControllers.IsValidIndex(Index) ? PlayerControllers[Index].Get() : nullptr;
}

void AMCGame_GM::ObjectClicked(AMCGame_PC* Player, AMCChessObjectBase* ObjectToBeat)
{
	AMCTableSlot* TempSlot = nullptr;
	if (ObjectToBeat)
	{
		bool bSlotValid = false;
		ObjectToBeat->GetCurrentSlot(TempSlot, bSlotValid);
	}
	BeatObject(Player, ObjectToBeat, GetOpponentPlayer(Player));
	if (TempSlot)
	{
		TempSlot->Server_SetAttachedObject(nullptr);
	}
	SlotClicked(Player, TempSlot, Player ? Player->SelectedChessObject.Get() : nullptr);
}

AMCGame_GS* AMCGame_GM::GetGS() const
{
	return GetGameState<AMCGame_GS>();
}

void AMCGame_GM::SlotClicked(AMCGame_PC* Player, AMCTableSlot* NewSlot, AMCChessObjectBase* SelectedObject)
{
	if (!IsValid(SelectedObject))
	{
		return;
	}

	int32 Row = 0, Col = 0;
	UMCCommonFunctions::ResolveSlotByNameCommon(NewSlot ? NewSlot->SlotName : NAME_None, Row, Col);
	if (SelectedObject->ObjectType == E_ObjectType::Pawn && Row == (SelectedObject->IsAway ? 1 : 8))
	{
		if (Player)
		{
			Player->Client_CreatePawnReplacementSelector(NewSlot ? NewSlot->SlotName : NAME_None, Player);
		}
	}
	MoveObjectToEmptySlot(NewSlot, SelectedObject);
	ResetSelectedObject(Player);
}

void AMCGame_GM::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	Server_OnLogin(NewPlayer);
}

void AMCGame_GM::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
	Super::SwapPlayerControllers(OldPC, NewPC);

	if (AMCLobby_PC* LobbyPC = Cast<AMCLobby_PC>(OldPC))
	{
		LobbyPC->RemoveAllWidgetsFromPlayer();
		if (HasAuthority() && !Cast<AMCGame_PC>(NewPC))
		{
			UKismetSystemLibrary::PrintString(this, TEXT("New PC is not GamePC"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
		}
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Old PC is not LobbyPC"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
	}
}

void AMCGame_GM::SpawnPlayer(AController* Player)
{
	// The Blueprint also had an unconnected "GetAllActorsOfClassWithTag -> RestartPlayerAtPlayerStart" branch that never ran.
	RestartPlayerAtPlayerStart(Player, UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()));
}

void AMCGame_GM::Server_OnLogin(UObject* Object)
{
	AMCGame_PC* GamePC = Cast<AMCGame_PC>(Object);
	if (!GamePC)
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Logged in controller wasn't a BP_Game_PC"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 2.f);
		return;
	}

	const int32 Index = PlayerControllers.Add(GamePC);
	GamePC->Server_SetGlobalPlayerIndex(Index);

	FS_PlayerInfo Info;
	FFormatNamedArguments Args;
	Args.Add(TEXT("Index"), FText::AsNumber(Index));
	Info.PlayerName = FText::Format(NSLOCTEXT("BP_Game_GM", "PlayerName", "Player_{Index}"), Args);
	Info.PlayerPoints = 100;
	Info.PlayerRole = (Index >= 0 && Index <= 1) ? (Index == 1 ? E_PlayerRole::AWAY : E_PlayerRole::HOME) : E_PlayerRole::SPECTATOR;
	Info.IsReady = false;

	GamePC->SetPlayerInfo(Info);
	if (AMCGame_GS* GS = GetGS())
	{
		GS->PlayerInfos.Add(GamePC, Info);
	}
	else
	{
		MC_ACCESSED_NONE("GetGS");
	}
}

void AMCGame_GM::ReplacePawnWithNewObject(FName Slot, AMCChessObjectBase* PawnObject, E_ObjectType NewObjectType, bool IsAway)
{
	if (PawnObject)
	{
		PawnObject->Destroy();
	}

	AMCChessTable* Table = UMCCommonFunctions::FindChessTable(this);
	if (!Table)
	{
		MC_ACCESSED_NONE("GetChessTableCommon");
		return;
	}

	FS_ObjectMap ObjectMap;
	ObjectMap.Class = ObjectClasses.FindRef(NewObjectType);
	ObjectMap.SlotName = Slot;
	ObjectMap.IsAway = IsAway;
	ObjectMap.ObjectType = NewObjectType;
	Table->SpawnObject(ObjectMap);

	AMCTableSlot* NewSlot = nullptr;
	bool bSlotValid = false;
	Table->GetSlotByName(Slot, NewSlot, bSlotValid);
	if (bSlotValid)
	{
		ProcessCheckingPossibility(NewSlot->AttachedObject);
	}
}

void AMCGame_GM::ProcessCheckingPossibility(AMCChessObjectBase* AttackerObject)
{
	if (!AttackerObject)
	{
		MC_ACCESSED_NONE("AttackerObject");
		return;
	}
	AMCKing* OpponentKing = nullptr;
	bool bKingValid = false;
	AttackerObject->GetOpponentKing(OpponentKing, bKingValid);
	if (bKingValid)
	{
		OpponentKing->S_MarkChecked(AttackerObject->CanCheckToSlot(OpponentKing->SlotName));
	}
}
