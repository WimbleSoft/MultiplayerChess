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

void AMCGame_PC::Server_ClickedToObject_Implementation(AMCGame_PC* PlayerController, AMCChessObjectBase* NewSelectedObject)
{
	FString Reason;
	if (!GameMode || !GameMode->CanPlayerAct(this, Reason))
	{
		Client_ShowMessage(GameMode ? Reason : FString(TEXT("The game is not ready.")));
		return;
	}
	if (IsObjectBelongToPlayer(GlobalPlayerIndex, NewSelectedObject))
	{
		Server_ObjectSelected(NewSelectedObject);
		return;
	}

	// Clicking an enemy piece is an attempt to capture it with the selected piece.
	AMCTableSlot* Slot = nullptr;
	bool bSlotValid = false;
	if (NewSelectedObject)
	{
		NewSelectedObject->GetCurrentSlot(Slot, bSlotValid);
	}
	if (bSlotValid && IsValid(SelectedChessObject))
	{
		Server_ClickedToSlot(Slot);
	}
	else
	{
		Client_ShowMessage(TEXT("This doesn't belong to you."));
	}
}

void AMCGame_PC::Server_ClickedToSlot_Implementation(AMCTableSlot* NewSlot)
{
	FString Reason;
	if (!GameMode || !GameMode->CanPlayerAct(this, Reason))
	{
		Client_ShowMessage(GameMode ? Reason : FString(TEXT("The game is not ready.")));
		return;
	}
	if (!NewSlot || !IsValid(SelectedChessObject))
	{
		return;
	}
	if (!GameMode->TryMovePiece(this, SelectedChessObject, NewSlot->SlotName))
	{
		Client_ShowMessage(TEXT("That move is not allowed."));
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
	if (!GameMode)
	{
		MC_ACCESSED_NONE("GameMode");
		return;
	}
	GameMode->CompletePromotion(this, Slot, NewObjectType);
}

void AMCGame_PC::Client_ShowMessage_Implementation(const FString& Message)
{
	UKismetSystemLibrary::PrintString(this, Message, true, true, FLinearColor(1.f, 0.41f, 0.f, 1.f), 3.f);
}

AMCChessTable* AMCGame_PC::GetChessTable() const
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMCChessTable::StaticClass(), OutActors);
	return OutActors.IsValidIndex(0) ? Cast<AMCChessTable>(OutActors[0]) : nullptr;
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
