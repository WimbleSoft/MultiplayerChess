#include "Game/MCGame_GM.h"
#include "Common/MCCommonFunctions.h"
#include "Common/MCGameInstance.h"
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
#include "Game/MCChessRules.h"
#include "Game/MCGamePawn.h"
#include "Game/MCGame_GS.h"
#include "Game/MCGame_PC.h"
#include "Game/MCGame_PS.h"
#include "Lobby/MCLobby_PC.h"
#include "MultiplayerChess.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	const TCHAR* SideName(bool bAway)
	{
		return bAway ? TEXT("Black (AWAY)") : TEXT("White (HOME)");
	}
}

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

	R_Table = UMCCommonFunctions::FindChessTable(this);
}

AMCChessTable* AMCGame_GM::GetTable() const
{
	return R_Table ? R_Table.Get() : UMCCommonFunctions::FindChessTable(this);
}

void AMCGame_GM::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Match rules come with the travel URL; without one (e.g. PIE straight into Game) use the game instance's
	// match settings, which start from the host's saved preferences.
	if (UGameplayStatics::HasOption(Options, TEXT("ShowOpponentMoves")))
	{
		bShowMovesToOpponent = UGameplayStatics::GetIntOption(Options, TEXT("ShowOpponentMoves"), 0) != 0;
	}
	else if (const UMCGameInstance* GI = Cast<UMCGameInstance>(GetGameInstance()))
	{
		bShowMovesToOpponent = GI->MatchSettings.ShowMovesToOpponent;
	}
}

void AMCGame_GM::InitGameState()
{
	Super::InitGameState();

	if (AMCGame_GS* GS = GetGS())
	{
		GS->bShowMovesToOpponent = bShowMovesToOpponent;
	}
}

void AMCGame_GM::BroadcastSelection(AMCGame_PC* SelectingPlayer, AMCChessObjectBase* Piece)
{
	if (!IsValid(Piece))
	{
		return;
	}
	const bool bSelected = Piece->IsSelected;
	TArray<FName> MoveSlots;
	AMCChessTable* Table = GetTable();
	int32 Col = 0, Row = 0;
	if (bSelected && Table && FMCChessRules::ParseSquare(Piece->SlotName, Col, Row))
	{
		TArray<FMCMove> Moves;
		Table->BuildRules().GenerateLegalMoves(Col, Row, Moves);
		for (const FMCMove& Move : Moves)
		{
			MoveSlots.Add(FMCChessRules::MakeSquareName(Move.ToCol, Move.ToRow));
		}
	}
	for (AMCGame_PC* Player : PlayerControllers)
	{
		// Clearing goes to everybody, so nobody keeps a stale highlight.
		if (Player && (Player == SelectingPlayer || !bSelected || bShowMovesToOpponent))
		{
			Player->Client_ShowSelection(Piece, bSelected, MoveSlots);
		}
	}
}

bool AMCGame_GM::ReadyToStartMatch()
{
	return GetNumPlayers() == 2;
}

bool AMCGame_GM::CanPlayerAct(AMCGame_PC* Player, FString& OutReason) const
{
	const AMCGame_GS* GS = GetGS();
	if (!Player || !GS)
	{
		OutReason = TEXT("The game is not ready.");
		return false;
	}
	if (GS->bGameOver)
	{
		OutReason = TEXT("The game is over.");
		return false;
	}
	if (PendingPromotionPlayer)
	{
		OutReason = PendingPromotionPlayer == Player ? TEXT("Pick a piece for your pawn first.") : TEXT("Waiting for the other player to promote a pawn.");
		return false;
	}
	if (PlayerControllers.Num() < 2)
	{
		OutReason = TEXT("Waiting for the second player.");
		return false;
	}
	if (GS->PlayerTurnIndex != Player->GlobalPlayerIndex)
	{
		OutReason = TEXT("It's not your turn.");
		return false;
	}
	return true;
}

bool AMCGame_GM::TryMovePiece(AMCGame_PC* Player, AMCChessObjectBase* Piece, FName TargetSlot)
{
	FString Reason;
	AMCChessTable* Table = GetTable();
	if (!CanPlayerAct(Player, Reason) || !IsValid(Piece) || !Table || Piece->IsAway != (Player->GlobalPlayerIndex == 1))
	{
		return false;
	}

	int32 FromCol = 0, FromRow = 0, ToCol = 0, ToRow = 0;
	FMCMove Move;
	if (!FMCChessRules::ParseSquare(Piece->SlotName, FromCol, FromRow) || !FMCChessRules::ParseSquare(TargetSlot, ToCol, ToRow)
		|| !Table->BuildRules().FindLegalMove(FromCol, FromRow, ToCol, ToRow, Move))
	{
		return false;
	}

	AMCTableSlot* TargetSlotActor = nullptr;
	bool bSlotValid = false;
	Table->GetSlotByName(TargetSlot, TargetSlotActor, bSlotValid);
	if (!bSlotValid)
	{
		return false;
	}

	AMCGame_PC* Opponent = GetOpponentPlayer(Player);
	if (Move.Kind == EMCMoveKind::EnPassant)
	{
		AMCChessObjectBase* Captured = nullptr;
		bool bCapturedValid = false;
		Table->GetObjectBySlotName(FMCChessRules::MakeSquareName(ToCol, FromRow), Captured, bCapturedValid);
		if (bCapturedValid)
		{
			AMCTableSlot* CapturedSlot = nullptr;
			bool bCapturedSlotValid = false;
			Captured->GetCurrentSlot(CapturedSlot, bCapturedSlotValid);
			BeatObject(Player, Captured, Opponent);
			if (CapturedSlot)
			{
				CapturedSlot->Server_SetAttachedObject(nullptr);
			}
		}
	}
	else if (Move.bCapture && IsValid(TargetSlotActor->AttachedObject))
	{
		BeatObject(Player, TargetSlotActor->AttachedObject, Opponent);
		TargetSlotActor->Server_SetAttachedObject(nullptr);
	}

	MoveObjectToEmptySlot(TargetSlotActor, Piece);

	if (Move.Kind == EMCMoveKind::CastleKingSide || Move.Kind == EMCMoveKind::CastleQueenSide)
	{
		const bool bKingSide = Move.Kind == EMCMoveKind::CastleKingSide;
		AMCChessObjectBase* Rook = nullptr;
		bool bRookValid = false;
		Table->GetObjectBySlotName(FMCChessRules::MakeSquareName(bKingSide ? 8 : 1, FromRow), Rook, bRookValid);
		AMCTableSlot* RookTarget = nullptr;
		bool bRookTargetValid = false;
		Table->GetSlotByName(FMCChessRules::MakeSquareName(bKingSide ? 6 : 4, FromRow), RookTarget, bRookTargetValid);
		if (bRookValid && bRookTargetValid)
		{
			MoveObjectToEmptySlot(RookTarget, Rook);
		}
	}

	Table->EnPassantSquare = Move.Kind == EMCMoveKind::DoublePawnStep ? FMCChessRules::MakeSquareName(FromCol, (FromRow + ToRow) / 2) : NAME_None;
	ResetSelectedObject(Player);

	if (Move.bPromotion)
	{
		// The turn waits until the player picks the new piece in WBP_ObjectSelector.
		PendingPromotionPlayer = Player;
		PendingPromotionSlot = TargetSlot;
		Player->Client_CreatePawnReplacementSelector(TargetSlot, Player);
		UpdateCheckState();
		return true;
	}

	FinishTurn();
	return true;
}

void AMCGame_GM::CompletePromotion(AMCGame_PC* Player, FName Slot, E_ObjectType NewObjectType)
{
	if (!Player || Player != PendingPromotionPlayer || Slot != PendingPromotionSlot)
	{
		return;
	}
	if (NewObjectType != E_ObjectType::Queen && NewObjectType != E_ObjectType::Rook && NewObjectType != E_ObjectType::Bishop && NewObjectType != E_ObjectType::Knight)
	{
		return;
	}
	AMCChessTable* Table = GetTable();
	AMCChessObjectBase* PawnObject = nullptr;
	bool bPawnValid = false;
	if (Table)
	{
		Table->GetObjectBySlotName(Slot, PawnObject, bPawnValid);
	}
	if (!bPawnValid || PawnObject->ObjectType != E_ObjectType::Pawn)
	{
		return;
	}

	PendingPromotionPlayer = nullptr;
	PendingPromotionSlot = NAME_None;
	ReplacePawnWithNewObject(Slot, PawnObject, NewObjectType, PawnObject->IsAway);
	FinishTurn();
}

void AMCGame_GM::FinishTurn()
{
	AMCGame_GS* GS = GetGS();
	AMCChessTable* Table = GetTable();
	if (!GS || !Table)
	{
		return;
	}
	GS->SetNewTurnIndex();
	UpdateCheckState();

	const bool bSideToMoveAway = GS->PlayerTurnIndex == 1;
	const FMCChessRules Rules = Table->BuildRules();
	const bool bInCheck = Rules.IsInCheck(bSideToMoveAway);
	if (!Rules.HasAnyLegalMove(bSideToMoveAway))
	{
		if (bInCheck)
		{
			EndGame(bSideToMoveAway ? 0 : 1, FString::Printf(TEXT("Checkmate! %s wins."), SideName(!bSideToMoveAway)));
		}
		else
		{
			EndGame(-1, TEXT("Stalemate. The game is a draw."));
		}
		return;
	}
	GS->Multicast_ShowMessage(FString::Printf(TEXT("%s to move%s"), SideName(bSideToMoveAway), bInCheck ? TEXT(" - CHECK!") : TEXT("")),
		bInCheck ? FLinearColor(1.f, 0.2f, 0.2f, 1.f) : FLinearColor(0.6f, 1.f, 0.6f, 1.f), 4.f);
}

void AMCGame_GM::EndGame(int32 WinnerIndex, const FString& Message)
{
	if (AMCGame_GS* GS = GetGS())
	{
		GS->bGameOver = true;
		GS->WinnerIndex = WinnerIndex;
		GS->Multicast_ShowMessage(Message, FLinearColor(1.f, 0.85f, 0.f, 1.f), 600.f);
	}
}

void AMCGame_GM::UpdateCheckState()
{
	AMCChessTable* Table = GetTable();
	if (!Table)
	{
		return;
	}
	const FMCChessRules Rules = Table->BuildRules();
	for (AMCChessObjectBase* Object : Table->AllObjects)
	{
		AMCKing* King = Cast<AMCKing>(Object);
		if (IsValid(King))
		{
			King->MarkChecked(Rules.IsInCheck(King->IsAway));
		}
	}
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
	BroadcastSelection(nullptr, SelectedObject);
	SelectedObject->S_SetIsMoved(true);
}

void AMCGame_GM::ResetSelectedObject(AMCGame_PC* Player)
{
	if (Player)
	{
		Player->SelectedChessObject = nullptr;
		Player->NewSelectedChessObject = nullptr;
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
		if (AMCChessTable* Table = GetTable())
		{
			Table->AllObjects.Remove(ObjectToBeat);
		}
		ObjectToBeat->Destroy();
	}
}

AMCGame_PC* AMCGame_GM::GetOpponentPlayer(AMCGame_PC* ItemToFind)
{
	// Select(PC[0], PC[1], Index = (Find(ItemToFind) == 0))
	const int32 Index = PlayerControllers.Find(ItemToFind) == 0 ? 1 : 0;
	return PlayerControllers.IsValidIndex(Index) ? PlayerControllers[Index].Get() : nullptr;
}

AMCGame_GS* AMCGame_GM::GetGS() const
{
	return GetGameState<AMCGame_GS>();
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
	AMCChessTable* Table = GetTable();
	if (PawnObject)
	{
		if (Table)
		{
			Table->AllObjects.Remove(PawnObject);
		}
		PawnObject->Destroy();
	}

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

	AMCChessObjectBase* NewObject = nullptr;
	bool bNewValid = false;
	Table->GetObjectBySlotName(Slot, NewObject, bNewValid);
	if (bNewValid)
	{
		NewObject->S_SetIsMoved(true);
	}
}
