#include "Game/MCGame_PC.h"
#include "Common/MCCommonFunctions.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCTableSlot.h"
#include "Game/MCGame_GM.h"
#include "Game/MCGame_PS.h"
#include "MultiplayerChess.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AMCGame_PC::AMCGame_PC()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	ObjectSelectorWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UI/Game/WBP_ObjectSelector.WBP_ObjectSelector_C")));
}

void AMCGame_PC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCGame_PC, SelectedChessObject);
	DOREPLIFETIME(AMCGame_PC, NewSelectedChessObject);
	DOREPLIFETIME(AMCGame_PC, GlobalPlayerIndex);
}

void AMCGame_PC::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GameMode = Cast<AMCGame_GM>(UGameplayStatics::GetGameMode(this));
}

void AMCGame_PC::BeginPlay()
{
	Super::BeginPlay();

	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, nullptr, EMouseLockMode::DoNotLock, true, false);
}

void AMCGame_PC::Server_ObjectSelected_Implementation(AMCChessObjectBase* NewSelectedObject)
{
	if (IsValid(SelectedChessObject))
	{
		if (SelectedChessObject == NewSelectedObject)
		{
			SelectedChessObject->Server_IsSelected(!SelectedChessObject->IsSelected);
			SelectedChessObject = nullptr;
		}
		else
		{
			Server_SwitchSelection(SelectedChessObject, NewSelectedObject);
		}
	}
	else
	{
		SelectedChessObject = NewSelectedObject;
		if (!SelectedChessObject || !NewSelectedObject)
		{
			MC_ACCESSED_NONE("SelectedChessObject");
			return;
		}
		SelectedChessObject->Server_IsSelected(!SelectedChessObject->IsSelected);
		NewSelectedObject->CalcPossibility();
	}
}

void AMCGame_PC::Server_SwitchSelection_Implementation(AMCChessObjectBase* SelectedObject, AMCChessObjectBase* NewSelectedObject)
{
	if (SelectedObject)
	{
		SelectedObject->Server_IsSelected(!SelectedObject->IsSelected);
	}
	else
	{
		MC_ACCESSED_NONE("SelectedObject");
	}
	SelectedChessObject = NewSelectedObject;
	if (!SelectedChessObject)
	{
		MC_ACCESSED_NONE("SelectedChessObject");
		return;
	}
	SelectedChessObject->Server_IsSelected(!SelectedChessObject->IsSelected);
	if (SelectedChessObject->IsSelected)
	{
		SelectedChessObject->CalcPossibility();
	}
}

void AMCGame_PC::Server_DoCastling_Implementation(AMCChessObjectBase* KingObject, FName KingNewSlot, AMCChessObjectBase* RookObject, FName RookNewSlot)
{
	if (!GameMode)
	{
		MC_ACCESSED_NONE("GameMode");
	}
	else
	{
		AMCTableSlot* KingSlot = nullptr;
		AMCTableSlot* RookSlot = nullptr;
		bool bValid = false;
		GetSlotByName(KingNewSlot, KingSlot, bValid);
		GameMode->MoveObjectToEmptySlot(KingSlot, KingObject);
		GetSlotByName(RookNewSlot, RookSlot, bValid);
		GameMode->MoveObjectToEmptySlot(RookSlot, RookObject);
	}
	SelectedChessObject = nullptr;
}

void AMCGame_PC::Server_ClickedToObject_Implementation(AMCGame_PC* PlayerController, AMCChessObjectBase* NewSelectedObject)
{
	AMCGame_GM* GM = Cast<AMCGame_GM>(UGameplayStatics::GetGameMode(this));
	const int32 PlayerIndex = GM ? GM->PlayerControllers.Find(PlayerController) : INDEX_NONE;
	if (IsObjectBelongToPlayer(PlayerIndex, NewSelectedObject))
	{
		Server_ObjectSelected(NewSelectedObject);
		return;
	}

	AMCTableSlot* Slot = nullptr;
	bool bSlotValid = false;
	if (NewSelectedObject)
	{
		NewSelectedObject->GetCurrentSlot(Slot, bSlotValid);
	}
	if (bSlotValid && Slot->IsActive)
	{
		Server_ClickedToSlot(Slot);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, TEXT("This doesn't belong to you."), true, false, FLinearColor(1.f, 0.41f, 0.f, 1.f), 2.f);
	}
}

void AMCGame_PC::Server_ClickedToSlot_Implementation(AMCTableSlot* NewSlot)
{
	if (!NewSlot)
	{
		MC_ACCESSED_NONE("NewSlot");
		return;
	}

	if (IsSlotEmpty(NewSlot->SlotName))
	{
		if (IsValid(SelectedChessObject) && SelectedChessObject->ObjectType == E_ObjectType::King)
		{
			TArray<FName> KingFrom, KingTo, RookFrom, RookTo;
			bool IsCastlingPossible = false;
			SelectedChessObject->GetCastlingPossibilities(KingFrom, KingTo, IsCastlingPossible, RookFrom, RookTo);
			if (IsCastlingPossible)
			{
				// As in the Blueprint: while castling is possible, a king click on any other empty slot does nothing.
				const int32 Index = KingTo.Find(NewSlot->SlotName);
				AMCChessTable* Table = UMCCommonFunctions::FindChessTable(this);
				AMCChessObjectBase* Rook = nullptr;
				bool bRookValid = false;
				if (Table && RookFrom.IsValidIndex(Index))
				{
					Table->GetObjectBySlotName(RookFrom[Index], Rook, bRookValid);
				}
				if (KingTo.Contains(NewSlot->SlotName) && bRookValid)
				{
					Server_DoCastling(SelectedChessObject, NewSlot->SlotName, Rook, RookTo.IsValidIndex(Index) ? RookTo[Index] : NAME_None);
				}
				return;
			}
		}

		if (!GameMode)
		{
			MC_ACCESSED_NONE("GameMode");
			return;
		}
		GameMode->SlotClicked(this, NewSlot, SelectedChessObject);
	}
	else
	{
		AMCChessObjectBase* Attached = NewSlot->AttachedObject;
		const bool bAttachedIsAway = Attached ? Attached->IsAway : false;
		if (bAttachedIsAway == (0 == GlobalPlayerIndex))
		{
			if (!GameMode)
			{
				MC_ACCESSED_NONE("GameMode");
				return;
			}
			GameMode->ObjectClicked(this, Attached);
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, TEXT("This slot is not empty and filled with your own object."), true, false, FLinearColor(1.f, 0.41f, 0.f, 1.f), 5.f);
		}
	}
}

void AMCGame_PC::Server_SetGlobalPlayerIndex_Implementation(int32 InGlobalPlayerIndex)
{
	Multi_SetGlobalPlayerIndex(InGlobalPlayerIndex);
}

void AMCGame_PC::Multi_SetGlobalPlayerIndex_Implementation(int32 InGlobalPlayerIndex)
{
	GlobalPlayerIndex = InGlobalPlayerIndex;
}

void AMCGame_PC::Client_CreatePawnReplacementSelector_Implementation(FName SlotToSwitch, APlayerController* OwningPlayer)
{
	UUserWidget* Widget = nullptr;
	if (UClass* WidgetClass = ObjectSelectorWidgetClass.LoadSynchronous())
	{
		Widget = CreateWidget<UUserWidget>(OwningPlayer, WidgetClass);
		// "Expose on Spawn" variable of WBP_ObjectSelector.
		UMCCommonFunctions::SetPropertyByName(Widget, TEXT("SlotToSwitch"), SlotToSwitch);
	}
	ShowWidget(Widget);
}

void AMCGame_PC::TryPawnReplacement_Implementation(FName Slot, E_ObjectType NewObjectType)
{
	AMCTableSlot* TargetSlot = nullptr;
	bool bSlotValid = false;
	GetSlotByName(Slot, TargetSlot, bSlotValid);
	if (!bSlotValid || !IsValid(TargetSlot->AttachedObject))
	{
		return;
	}
	if (TargetSlot->AttachedObject->IsAway == (GlobalPlayerIndex == 1))
	{
		if (!GameMode)
		{
			MC_ACCESSED_NONE("GameMode");
			return;
		}
		GameMode->ReplacePawnWithNewObject(Slot, TargetSlot->AttachedObject, NewObjectType, GlobalPlayerIndex == 1);
	}
}

AMCChessTable* AMCGame_PC::GetChessTable() const
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMCChessTable::StaticClass(), OutActors);
	return OutActors.IsValidIndex(0) ? Cast<AMCChessTable>(OutActors[0]) : nullptr;
}

bool AMCGame_PC::DetectCastling(AMCChessObjectBase* NewSelectedObject, AMCChessObjectBase* SelectedObject)
{
	if (!NewSelectedObject || !SelectedObject)
	{
		MC_ACCESSED_NONE("SelectedObject");
		return false;
	}
	auto Is = [](AMCChessObjectBase* Object, const TCHAR* Slot, E_ObjectType Type)
	{
		return !Object->IsMoved && Object->SlotName == FName(Slot) && Object->ObjectType == Type;
	};
	auto Side = [&](const TCHAR* E, const TCHAR* H, const TCHAR* A, const TCHAR* F, const TCHAR* G, const TCHAR* D, const TCHAR* C, const TCHAR* B)
	{
		const bool bShortEmpty = IsSlotEmpty(F) && IsSlotEmpty(G);
		const bool bLongEmpty = IsSlotEmpty(D) && IsSlotEmpty(C) && IsSlotEmpty(B);
		return (Is(SelectedObject, E, E_ObjectType::King) && Is(NewSelectedObject, H, E_ObjectType::Rook) && bShortEmpty)
			|| (bShortEmpty && Is(SelectedObject, H, E_ObjectType::Rook) && Is(NewSelectedObject, E, E_ObjectType::King))
			|| (Is(SelectedObject, E, E_ObjectType::King) && Is(NewSelectedObject, A, E_ObjectType::Rook) && bLongEmpty)
			|| (bLongEmpty && Is(SelectedObject, A, E_ObjectType::Rook) && Is(NewSelectedObject, E, E_ObjectType::King));
	};
	// Only same sides of selections can make castling.
	return SelectedObject->IsAway == NewSelectedObject->IsAway
		&& (Side(TEXT("E1"), TEXT("H1"), TEXT("A1"), TEXT("F1"), TEXT("G1"), TEXT("D1"), TEXT("C1"), TEXT("B1"))
			|| Side(TEXT("E8"), TEXT("H8"), TEXT("A8"), TEXT("F8"), TEXT("G8"), TEXT("D8"), TEXT("C8"), TEXT("B8")));
}

void AMCGame_PC::DetectSwitching(AMCChessObjectBase* NewSelectedObject)
{
}

bool AMCGame_PC::IsSlotEmpty(FName SlotName) const
{
	AMCChessTable* Table = UMCCommonFunctions::FindChessTable(this);
	if (!Table)
	{
		MC_ACCESSED_NONE("GetChessTableCommon");
		return true;
	}
	const bool* Occupied = Table->GetAllSlotsOccupationInfo().Find(SlotName);
	return !(Occupied && *Occupied);
}

void AMCGame_PC::SetPlayerInfo(const FS_PlayerInfo& PlayerInfo)
{
	AMCGame_PS* GamePS = Cast<AMCGame_PS>(PlayerState);
	if (!GamePS)
	{
		MC_ACCESSED_NONE("AsBP Game PS");
		return;
	}
	GamePS->Server_SetPlayerInfo(PlayerInfo);
}

bool AMCGame_PC::IsObjectBelongToPlayer(int32 PlayerIndex, AMCChessObjectBase* NewSelectedObject)
{
	if (!(PlayerIndex >= 0 && PlayerIndex <= 1))
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Cannot find Game PC in PC list of Game GM"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 3.f);
		return false;
	}
	const bool bObjectIsAway = NewSelectedObject ? NewSelectedObject->IsAway : false;
	return (PlayerIndex == 1) == bObjectIsAway;
}

void AMCGame_PC::GetSlotByName(FName SlotName, AMCTableSlot*& Slot, bool& bIsValid) const
{
	Slot = nullptr;
	bIsValid = false;
	if (AMCChessTable* Table = GetChessTable())
	{
		Table->GetSlotByName(SlotName, Slot, bIsValid);
	}
	else
	{
		MC_ACCESSED_NONE("GetChessTable");
	}
}
