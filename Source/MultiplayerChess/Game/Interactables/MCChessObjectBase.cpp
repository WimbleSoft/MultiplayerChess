#include "Game/Interactables/MCChessObjectBase.h"
#include "Common/MCCommonFunctions.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCKing.h"
#include "Game/Interactables/MCTableSlot.h"
#include "Game/MCGame_PC.h"
#include "Game/MCSlotInterface.h"
#include "MultiplayerChess.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

AMCChessObjectBase::AMCChessObjectBase()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	HomeMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Geometry/Materials/MI_HomeChessObject.MI_HomeChessObject")));
	AwayMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Geometry/Materials/MI_AwayChessObject.MI_AwayChessObject")));
}

void AMCChessObjectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCChessObjectBase, ObjectType);
	DOREPLIFETIME(AMCChessObjectBase, IsAway);
	DOREPLIFETIME(AMCChessObjectBase, SlotName);
	DOREPLIFETIME(AMCChessObjectBase, IsSelected);
	DOREPLIFETIME(AMCChessObjectBase, PossibleSlots);
	DOREPLIFETIME(AMCChessObjectBase, IsMoved);
	DOREPLIFETIME(AMCChessObjectBase, IsChecking);
}

void AMCChessObjectBase::FindBlueprintComponents()
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component->GetFName() == TEXT("SM_ChessObject"))
		{
			SM_ChessObjectComponent = Cast<UStaticMeshComponent>(Component);
		}
		else if (Component->GetFName() == TEXT("TextRender"))
		{
			TextRenderComponent = Cast<UTextRenderComponent>(Component);
		}
	}
}

void AMCChessObjectBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FindBlueprintComponents();
	ApplyAwayMaterial();
}

void AMCChessObjectBase::BeginPlay()
{
	Super::BeginPlay();

	FindBlueprintComponents();
	if (SM_ChessObjectComponent)
	{
		SM_ChessObjectComponent->OnBeginCursorOver.AddUniqueDynamic(this, &AMCChessObjectBase::OnMeshBeginCursorOver);
		SM_ChessObjectComponent->OnEndCursorOver.AddUniqueDynamic(this, &AMCChessObjectBase::OnMeshEndCursorOver);
		SM_ChessObjectComponent->OnClicked.AddUniqueDynamic(this, &AMCChessObjectBase::OnMeshClicked);
	}

	Server_RotText();
}

void AMCChessObjectBase::SetIsAwayNotify(bool bNewValue)
{
	IsAway = bNewValue;
	OnRep_IsAway();
}

void AMCChessObjectBase::SetSlotNameNotify(FName NewValue)
{
	SlotName = NewValue;
	OnRep_SlotName();
}

void AMCChessObjectBase::SetIsSelectedNotify(bool bNewValue)
{
	IsSelected = bNewValue;
	OnRep_IsSelected();
}

void AMCChessObjectBase::ClickGlow(bool IsGlowing)
{
	if (!DMI_Mesh)
	{
		MC_ACCESSED_NONE("DMI_Mesh");
		return;
	}
	DMI_Mesh->SetScalarParameterValue(TEXT("Glow"), IsGlowing ? 10.f : 0.f);
	const FLinearColor TeamColor = IsAway ? FLinearColor(0.f, 0.f, 0.f, 1.f) : FLinearColor(1.f, 1.f, 1.f, 1.f);
	DMI_Mesh->SetVectorParameterValue(TEXT("Color"), IsGlowing ? FLinearColor(1.f, 0.f, 0.f, 1.f) : TeamColor);
}

AMCChessTable* AMCChessObjectBase::GetChessTable() const
{
	return Cast<AMCChessTable>(GetParentActor());
}

void AMCChessObjectBase::GetSlotColRow(FName InName, int32& Column, int32& Row) const
{
	const FString Name = InName.ToString();
	Column = UMCCommonFunctions::GetColumnLetters().Find(Name.Mid(0, 1));
	Row = FCString::Atoi(*Name.Mid(1, 1));
}

void AMCChessObjectBase::HoverGlow(bool Glow)
{
	if (Glow)
	{
		if (!DMI_Mesh)
		{
			MC_ACCESSED_NONE("DMI_Mesh");
			return;
		}
		DMI_Mesh->SetScalarParameterValue(TEXT("Glow"), 10.f);
		DMI_Mesh->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.f, 0.f, 1.f, 1.f));
	}
	else
	{
		ClickGlow(IsSelected);
	}
}

void AMCChessObjectBase::ApplyAwayMaterial()
{
	UMaterialInterface* Parent = IsAway ? AwayMaterial.LoadSynchronous() : HomeMaterial.LoadSynchronous();
	DMI_Mesh = UMaterialInstanceDynamic::Create(Parent, this);
	if (SM_ChessObjectComponent)
	{
		SM_ChessObjectComponent->SetMaterial(0, DMI_Mesh);
	}
}

void AMCChessObjectBase::OnRep_IsAway()
{
	if (!SM_ChessObjectComponent)
	{
		FindBlueprintComponents();
	}
	ApplyAwayMaterial();
}

void AMCChessObjectBase::OnRep_IsSelected()
{
	ClickGlow(IsSelected);
	if (IsSelected)
	{
		Server_CalcPossibility();
	}
	else
	{
		Server_DeCalcPossibility();
	}
}

void AMCChessObjectBase::OnRep_SlotName()
{
	if (!TextRenderComponent)
	{
		FindBlueprintComponents();
	}
	if (TextRenderComponent)
	{
		TextRenderComponent->SetText(FText::FromName(SlotName));
	}
}

void AMCChessObjectBase::CalcHover(bool IsHovering)
{
	Client_Hover(IsHovering);
}

void AMCChessObjectBase::CalcPossibility()
{
}

void AMCChessObjectBase::GetCurrentSlot(AMCTableSlot*& Slot, bool& bIsValid) const
{
	GetSlotByName(SlotName, Slot, bIsValid);
}

void AMCChessObjectBase::AddCastlingPossibilities()
{
	TArray<FName> KingFrom, KingTo, RookFrom, RookTo;
	bool IsCastlingPossible = false;
	GetCastlingPossibilities(KingFrom, KingTo, IsCastlingPossible, RookFrom, RookTo);
	if (IsCastlingPossible)
	{
		for (const FName& Element : KingTo)
		{
			Server_AddPossibleSlot(Element);
		}
	}
}

// The three "...ByParams" functions walk one direction and stop at the first occupied slot.
// Own piece: stop without adding. Enemy piece: stop and add. Empty: add and continue.
// AllInRange lists are only filled when this object is a King, exactly like the Blueprint.
void AMCChessObjectBase::GetColPossbilitiesByParams(int32 ToCol, int32 MaxMovement, TArray<FName>& ColPossibilities, TArray<FName>& AllColSurroundings) const
{
	TArray<FName> ValidPossibilities;
	TArray<FName> InRangePossibilities;
	int32 Col = 0, Row = 0;
	GetSlotColRow(SlotName, Col, Row);
	AMCChessTable* Table = GetChessTable();

	bool bBreak = false;
	for (int32 Index = 1; Index <= MaxMovement && !bBreak; ++Index)
	{
		FName Name;
		bool InRange = false;
		GetSlotNameByColRow(Col + Index * ToCol, Row, Name, InRange);
		if (!InRange)
		{
			continue;
		}
		if (ObjectType == E_ObjectType::King)
		{
			InRangePossibilities.AddUnique(Name);
		}
		AMCTableSlot* Slot = Table ? Table->GetTableSlotBySocketName(Name) : nullptr;
		AMCChessObjectBase* Attached = Slot ? Slot->AttachedObject.Get() : nullptr;
		bool IsEmpty = true, IsFull = false;
		if (Slot)
		{
			Slot->GetIsEmpty(IsEmpty, IsFull);
		}
		if (IsValid(Attached) && IsFull)
		{
			bBreak = true;
			if (!(!IsAway == Attached->IsAway))
			{
				continue;
			}
		}
		ValidPossibilities.AddUnique(Name);
	}

	ColPossibilities = ValidPossibilities;
	AllColSurroundings = InRangePossibilities;
}

void AMCChessObjectBase::GetDiagonalPossbilitiesByParams(int32 A, int32 B, int32 MaxMovement, TArray<FName>& Possibilities, TArray<FName>& AllInRangePossibilities) const
{
	TArray<FName> ValidPossibilities;
	TArray<FName> InRangePossibilities;
	int32 Col = 0, Row = 0;
	GetSlotColRow(SlotName, Col, Row);
	AMCChessTable* Table = GetChessTable();

	bool bBreak = false;
	for (int32 Index = 1; Index <= MaxMovement && !bBreak; ++Index)
	{
		FName Name;
		bool InRange = false;
		GetSlotNameByColRow(Col + A * Index, Row + Index * B, Name, InRange);
		if (!InRange)
		{
			continue;
		}
		if (ObjectType == E_ObjectType::King)
		{
			InRangePossibilities.AddUnique(Name);
		}
		AMCTableSlot* Slot = Table ? Table->GetTableSlotBySocketName(Name) : nullptr;
		AMCChessObjectBase* Attached = Slot ? Slot->AttachedObject.Get() : nullptr;
		bool IsEmpty = true, IsFull = false;
		if (Slot)
		{
			Slot->GetIsEmpty(IsEmpty, IsFull);
		}
		if (IsValid(Attached) && IsFull)
		{
			bBreak = true;
			if (!(!IsAway == Attached->IsAway))
			{
				continue;
			}
		}
		ValidPossibilities.AddUnique(Name);
	}

	Possibilities = ValidPossibilities;
	AllInRangePossibilities = InRangePossibilities;
}

void AMCChessObjectBase::GetRowPossibilitiesByParams(int32 ToRow, int32 LastIndex, TArray<FName>& RowPossibilities, TArray<FName>& AllRowSurroundings) const
{
	TArray<FName> ValidPossilities;
	TArray<FName> InRangePossilities;
	int32 Col = 0, Row = 0;
	GetSlotColRow(SlotName, Col, Row);
	AMCChessTable* Table = GetChessTable();

	bool bBreak = false;
	for (int32 Index = 1; Index <= LastIndex && !bBreak; ++Index)
	{
		FName Name;
		bool InRange = false;
		GetSlotNameByColRow(Col, Row + Index * ToRow, Name, InRange);
		if (!InRange)
		{
			continue;
		}
		if (ObjectType == E_ObjectType::King)
		{
			InRangePossilities.AddUnique(Name);
		}
		AMCTableSlot* Slot = Table ? Table->GetTableSlotBySocketName(Name) : nullptr;
		AMCChessObjectBase* Attached = Slot ? Slot->AttachedObject.Get() : nullptr;
		bool IsEmpty = true, IsFull = false;
		if (Slot)
		{
			Slot->GetIsEmpty(IsEmpty, IsFull);
		}
		if (IsValid(Attached) && IsFull)
		{
			bBreak = true;
			if (!(!IsAway == Attached->IsAway))
			{
				continue;
			}
		}
		// Array_Add here (the Col/Diagonal versions use Array_AddUnique).
		ValidPossilities.Add(Name);
	}

	RowPossibilities = ValidPossilities;
	AllRowSurroundings = InRangePossilities;
}

void AMCChessObjectBase::GetDiagonalPossibilities(int32 MaxMovement, TArray<FName>& ValidDiagonalPossibilities, TArray<FName>& AllDiagonalPossibilities) const
{
	TArray<FName> ValidPossibilities;
	TArray<FName> InvalidPossibilities;

	const int32 Directions[4][2] = {
		{ IsAway ? 1 : -1, IsAway ? -1 : 1 },
		{ IsAway ? -1 : 1, IsAway ? 1 : -1 },
		{ IsAway ? 1 : -1, IsAway ? 1 : -1 },
		{ IsAway ? -1 : 1, IsAway ? -1 : 1 },
	};

	for (const auto& Direction : Directions)
	{
		TArray<FName> Possibilities, AllInRangePossibilities;
		GetDiagonalPossbilitiesByParams(Direction[0], Direction[1], MaxMovement, Possibilities, AllInRangePossibilities);
		for (const FName& Element : Possibilities)
		{
			if (ObjectType != E_ObjectType::King)
			{
				ValidPossibilities.AddUnique(Element);
			}
			else
			{
				AMCKing* OpponentKing = nullptr;
				bool bKingValid = false;
				GetOpponentKing(OpponentKing, bKingValid);
				if (bKingValid && !OpponentKing->GetKingSurroundings().Contains(Element))
				{
					ValidPossibilities.AddUnique(Element);
				}
			}
		}
		for (const FName& Element : AllInRangePossibilities)
		{
			InvalidPossibilities.AddUnique(Element);
		}
	}

	ValidDiagonalPossibilities = ValidPossibilities;
	AllDiagonalPossibilities = InvalidPossibilities;
}

bool AMCChessObjectBase::CalcCheckingKing()
{
	return false;
}

bool AMCChessObjectBase::CanCheckToSlot(FName InSlotName)
{
	return false;
}

void AMCChessObjectBase::GetSlotNameByColRow(int32 Col, int32 Row, FName& SocketName, bool& InRange) const
{
	if (Col >= 1 && Col <= 8 && Row >= 1 && Row <= 8)
	{
		SocketName = FName(UMCCommonFunctions::GetColumnLetters()[Col] + FString::FromInt(Row));
		InRange = true;
	}
	else
	{
		SocketName = NAME_None;
		InRange = false;
	}
}

bool AMCChessObjectBase::GetIsObjectBelongToPlayer(int32 PlayerIndex)
{
	if (!(PlayerIndex >= 0 && PlayerIndex <= 1))
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Cannot find Game PC in PC list of Game GM"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 3.f);
		return false;
	}
	return (PlayerIndex == 1) == IsAway;
}

void AMCChessObjectBase::AddDiagonalPossibilities(int32 MaxMovement)
{
	TArray<FName> ValidDiagonalPossibilities, AllDiagonalPossibilities;
	GetDiagonalPossibilities(MaxMovement, ValidDiagonalPossibilities, AllDiagonalPossibilities);
	for (const FName& Element : ValidDiagonalPossibilities)
	{
		Server_AddPossibleSlot(Element);
	}
}

void AMCChessObjectBase::AddStraightPossibilities(int32 MaxMovement)
{
	TArray<FName> ValidStraightPossibilities, AllInRangeStraightPossibilities;
	GetStraightPossibilities(MaxMovement, ValidStraightPossibilities, AllInRangeStraightPossibilities);
	for (const FName& Element : ValidStraightPossibilities)
	{
		Server_AddPossibleSlot(Element);
	}
}

void AMCChessObjectBase::GetCastlingPossibilities(TArray<FName>& KingFrom, TArray<FName>& KingTo, bool& IsCastlingPossible, TArray<FName>& RookFrom, TArray<FName>& RookTo) const
{
	TArray<FName> KingFromArray, KingToArray, RookFromArray, RookToArray;

	// HOME SIDE CASTLING
	if (SlotName == TEXT("E1") && !IsMoved && IsObjectOnSlotWithType(TEXT("H1"), TEXT("H1"), E_ObjectType::Rook) && (IsSlotEmpty(TEXT("F1")) && IsSlotEmpty(TEXT("G1"))))
	{
		KingFromArray.AddUnique(TEXT("E1"));
		KingToArray.AddUnique(TEXT("G1"));
		RookFromArray.AddUnique(TEXT("H1"));
		RookToArray.AddUnique(TEXT("F1"));
	}
	if (SlotName == TEXT("E1") && !IsMoved && IsObjectOnSlotWithType(TEXT("A1"), TEXT("A1"), E_ObjectType::Rook) && (IsSlotEmpty(TEXT("D1")) && IsSlotEmpty(TEXT("C1")) && IsSlotEmpty(TEXT("B1"))))
	{
		KingFromArray.AddUnique(TEXT("E1"));
		KingToArray.AddUnique(TEXT("B1"));
		RookFromArray.AddUnique(TEXT("A1"));
		RookToArray.AddUnique(TEXT("C1"));
	}
	// AWAY SIDE CASTLING
	if (SlotName == TEXT("E8") && !IsMoved && IsObjectOnSlotWithType(TEXT("H8"), TEXT("H8"), E_ObjectType::Rook) && (IsSlotEmpty(TEXT("F8")) && IsSlotEmpty(TEXT("G8"))))
	{
		KingFromArray.AddUnique(TEXT("E8"));
		KingToArray.AddUnique(TEXT("G8"));
		RookFromArray.AddUnique(TEXT("H8"));
		RookToArray.AddUnique(TEXT("F8"));
	}
	if (SlotName == TEXT("E8") && !IsMoved && IsObjectOnSlotWithType(TEXT("A8"), TEXT("A8"), E_ObjectType::Rook) && (IsSlotEmpty(TEXT("D8")) && IsSlotEmpty(TEXT("C8")) && IsSlotEmpty(TEXT("B8"))))
	{
		KingFromArray.AddUnique(TEXT("E8"));
		KingToArray.AddUnique(TEXT("B8"));
		RookFromArray.AddUnique(TEXT("A8"));
		RookToArray.AddUnique(TEXT("C8"));
	}

	KingFrom = KingFromArray;
	KingTo = KingToArray;
	IsCastlingPossible = KingFromArray.Num() > 0;
	RookFrom = RookFromArray;
	RookTo = RookToArray;
}

bool AMCChessObjectBase::IsSlotEmpty(FName InSlotName) const
{
	AMCChessTable* Table = GetChessTable();
	if (!Table)
	{
		MC_ACCESSED_NONE("GetChessTable");
		return true;
	}
	const bool* Occupied = Table->GetAllSlotsOccupationInfo().Find(InSlotName);
	return !(Occupied && *Occupied);
}

void AMCChessObjectBase::GetSlotByName(FName InSlotName, AMCTableSlot*& Slot, bool& bIsValid) const
{
	Slot = nullptr;
	bIsValid = false;
	if (AMCChessTable* Table = GetChessTable())
	{
		Table->GetSlotByName(InSlotName, Slot, bIsValid);
	}
	else
	{
		MC_ACCESSED_NONE("GetChessTable");
	}
}

bool AMCChessObjectBase::IsObjectOnSlotWithType(FName CurrentSlot, FName SlotToCheck, E_ObjectType ObjectTypeToCheck) const
{
	AMCTableSlot* Slot = nullptr;
	bool bSlotValid = false;
	GetSlotByName(CurrentSlot, Slot, bSlotValid);
	if (!bSlotValid)
	{
		return false;
	}
	AMCChessObjectBase* Attached = Slot->AttachedObject;
	if (!IsValid(Attached))
	{
		return false;
	}
	if (IsAway != Attached->IsAway)
	{
		return false;
	}
	return !Attached->IsMoved && Attached->SlotName == SlotToCheck && Attached->ObjectType == ObjectTypeToCheck;
}

void AMCChessObjectBase::GetStraightPossibilities(int32 MaxMovement, TArray<FName>& ValidStraightPossibilities, TArray<FName>& AllInRangeStraightPossibilities) const
{
	TArray<FName> ValidPossibilities;
	TArray<FName> InvalidPossibilities;

	auto AddFiltered = [this, &ValidPossibilities](const TArray<FName>& Candidates)
	{
		for (const FName& Element : Candidates)
		{
			if (ObjectType == E_ObjectType::King)
			{
				AMCKing* OpponentKing = nullptr;
				bool bKingValid = false;
				GetOpponentKing(OpponentKing, bKingValid);
				if (bKingValid && !OpponentKing->GetKingSurroundings().Contains(Element))
				{
					ValidPossibilities.AddUnique(Element);
				}
			}
			else
			{
				ValidPossibilities.AddUnique(Element);
			}
		}
	};
	auto AddAll = [&InvalidPossibilities](const TArray<FName>& Candidates)
	{
		for (const FName& Element : Candidates)
		{
			InvalidPossibilities.AddUnique(Element);
		}
	};

	TArray<FName> Possibilities, Surroundings;
	GetRowPossibilitiesByParams(IsAway ? -1 : 1, MaxMovement, Possibilities, Surroundings);
	AddFiltered(Possibilities);
	AddAll(Surroundings);

	GetColPossbilitiesByParams(IsAway ? -1 : 1, MaxMovement, Possibilities, Surroundings);
	AddFiltered(Possibilities);
	AddAll(Surroundings);

	GetRowPossibilitiesByParams(IsAway ? 1 : -1, MaxMovement, Possibilities, Surroundings);
	AddFiltered(Possibilities);
	AddAll(Surroundings);

	GetColPossbilitiesByParams(IsAway ? 1 : -1, MaxMovement, Possibilities, Surroundings);
	AddFiltered(Possibilities);
	AddAll(Surroundings);

	ValidStraightPossibilities = ValidPossibilities;
	AllInRangeStraightPossibilities = InvalidPossibilities;
}

void AMCChessObjectBase::GetKingPossibilities(TArray<FName>& Possibilities, TArray<FName>& NotValidPossibilities) const
{
	TArray<FName> TempPossibilities;
	TArray<FName> InvalidPossibilities;

	TArray<FName> ValidDiagonal, AllDiagonal;
	GetDiagonalPossibilities(1, ValidDiagonal, AllDiagonal);
	for (const FName& Element : ValidDiagonal)
	{
		TempPossibilities.AddUnique(Element);
	}
	for (const FName& Element : AllDiagonal)
	{
		InvalidPossibilities.AddUnique(Element);
	}

	TArray<FName> ValidStraight, AllStraight;
	GetStraightPossibilities(1, ValidStraight, AllStraight);
	for (const FName& Element : ValidStraight)
	{
		TempPossibilities.AddUnique(Element);
	}
	for (const FName& Element : AllStraight)
	{
		InvalidPossibilities.AddUnique(Element);
	}

	Possibilities = TempPossibilities;
	NotValidPossibilities = InvalidPossibilities;
}

void AMCChessObjectBase::GetOpponentKing(AMCKing*& OpponentKing, bool& bIsValid) const
{
	OpponentKing = nullptr;
	bIsValid = false;
	AMCChessTable* Table = GetChessTable();
	if (!Table)
	{
		MC_ACCESSED_NONE("GetChessTable");
		return;
	}
	for (AMCChessObjectBase* Element : Table->AllObjects)
	{
		if (IsValid(Element) && Element->ObjectType == E_ObjectType::King && Element->IsAway != IsAway)
		{
			OpponentKing = Cast<AMCKing>(Element);
			bIsValid = OpponentKing != nullptr;
			return;
		}
	}
}

TArray<FName> AMCChessObjectBase::GetDiagonalSurroundings(int32 MaxMovement) const
{
	TArray<FName> Surroundings;
	const int32 Directions[4][2] = {
		{ IsAway ? 1 : -1, IsAway ? -1 : 1 },
		{ IsAway ? -1 : 1, IsAway ? 1 : -1 },
		{ IsAway ? 1 : -1, IsAway ? 1 : -1 },
		{ IsAway ? -1 : 1, IsAway ? -1 : 1 },
	};
	for (const auto& Direction : Directions)
	{
		TArray<FName> Possibilities, AllInRangePossibilities;
		GetDiagonalPossbilitiesByParams(Direction[0], Direction[1], MaxMovement, Possibilities, AllInRangePossibilities);
		for (const FName& Element : AllInRangePossibilities)
		{
			Surroundings.AddUnique(Element);
		}
	}
	return Surroundings;
}

TArray<FName> AMCChessObjectBase::GetStraightSurroundings(int32 MaxMovement) const
{
	TArray<FName> InvalidPossibilities;
	TArray<FName> Possibilities, Surroundings;

	GetRowPossibilitiesByParams(IsAway ? -1 : 1, MaxMovement, Possibilities, Surroundings);
	for (const FName& Element : Surroundings)
	{
		InvalidPossibilities.AddUnique(Element);
	}
	GetColPossbilitiesByParams(IsAway ? -1 : 1, MaxMovement, Possibilities, Surroundings);
	for (const FName& Element : Surroundings)
	{
		InvalidPossibilities.AddUnique(Element);
	}
	GetRowPossibilitiesByParams(IsAway ? 1 : -1, MaxMovement, Possibilities, Surroundings);
	for (const FName& Element : Surroundings)
	{
		InvalidPossibilities.AddUnique(Element);
	}
	GetColPossbilitiesByParams(IsAway ? 1 : -1, MaxMovement, Possibilities, Surroundings);
	for (const FName& Element : Surroundings)
	{
		InvalidPossibilities.AddUnique(Element);
	}
	return InvalidPossibilities;
}

// ---- EventGraph ----

void AMCChessObjectBase::OnMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	CalcHover(true);
}

void AMCChessObjectBase::OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	CalcHover(false);
}

void AMCChessObjectBase::OnMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0));
	if (!GamePC)
	{
		MC_ACCESSED_NONE("AsBP Game PC");
		return;
	}
	GamePC->Server_ClickedToObject(GamePC, this);
}

void AMCChessObjectBase::Server_IsSelected_Implementation(bool InIsSelected)
{
	Multicast_IsSelected(InIsSelected);
}

void AMCChessObjectBase::Multicast_IsSelected_Implementation(bool InIsSelected)
{
	SetIsSelectedNotify(InIsSelected);
}

void AMCChessObjectBase::Client_Hover_Implementation(bool IsHovering)
{
	HoverGlow(IsHovering);
}

void AMCChessObjectBase::Server_SetIsAway_Implementation(bool InIsAway)
{
	Multi_SetIsAway(InIsAway);
}

void AMCChessObjectBase::Multi_SetIsAway_Implementation(bool InIsAway)
{
	SetIsAwayNotify(InIsAway);
}

void AMCChessObjectBase::ShowPossibilities()
{
	AMCChessTable* Table = GetChessTable();
	for (const FName& Possible : PossibleSlots)
	{
		if (!Table)
		{
			MC_ACCESSED_NONE("GetChessTable");
			return;
		}
		for (AMCTableSlot* Slot : Table->AllSlots)
		{
			if (Slot && Slot->SlotName == Possible && Slot->GetClass()->ImplementsInterface(UMCSlotInterface::StaticClass()))
			{
				IMCSlotInterface::Execute_ShowPossibles(Slot, Slot->SlotName);
			}
		}
	}
}

void AMCChessObjectBase::Server_CalcPossibility_Implementation()
{
	CalcPossibility();
	Multicast_CalcPossibility();
}

void AMCChessObjectBase::Server_DeCalcPossibility_Implementation()
{
	Multicast_DeCalcPossibility();
}

void AMCChessObjectBase::Multicast_CalcPossibility_Implementation()
{
	ShowPossibilities();
}

void AMCChessObjectBase::Multicast_DeCalcPossibility_Implementation()
{
	Server_ClearPossibleSlots();
	if (AMCChessTable* Table = GetChessTable())
	{
		for (AMCTableSlot* Slot : Table->AllSlots)
		{
			if (Slot && Slot->GetClass()->ImplementsInterface(UMCSlotInterface::StaticClass()))
			{
				IMCSlotInterface::Execute_ShowPossibles(Slot, NAME_None);
			}
		}
	}
}

void AMCChessObjectBase::Server_AddPossibleSlot_Implementation(FName Socket)
{
	Multicast_AddPossibleSlot(Socket);
}

void AMCChessObjectBase::Multicast_AddPossibleSlot_Implementation(FName Slot)
{
	PossibleSlots.AddUnique(Slot);
}

void AMCChessObjectBase::Server_ClearPossibleSlots_Implementation()
{
	Multicast_ClearPossibleSlots();
}

void AMCChessObjectBase::Multicast_ClearPossibleSlots_Implementation()
{
	PossibleSlots.Empty();
	PossibleSlots.AddUnique(NAME_None);
}

void AMCChessObjectBase::SetSocketName_Implementation(FName SocketName)
{
	SetSlotNameNotify(SocketName);
}

void AMCChessObjectBase::Server_RotText_Implementation()
{
	if (AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Client_RotText(GamePC->GlobalPlayerIndex);
	}
}

void AMCChessObjectBase::Client_RotText_Implementation(int32 A)
{
	if ((A == 1) == IsAway)
	{
		const bool bPick = (A == 1) && IsAway;
		const FRotator WhenAway = bPick ? FRotator(90.f, 180.f, 0.f) : FRotator(90.f, 0.f, 0.f);
		const FRotator WhenHome = bPick ? FRotator(90.f, 0.f, 0.f) : FRotator(90.f, 180.f, 0.f);
		if (TextRenderComponent)
		{
			TextRenderComponent->SetRelativeRotation(IsAway ? WhenAway : WhenHome);
		}
	}
}

void AMCChessObjectBase::S_SetIsMoved_Implementation(bool InIsMoved)
{
	M_SetIsMoved(InIsMoved);
}

void AMCChessObjectBase::M_SetIsMoved_Implementation(bool InIsMoved)
{
	IsMoved = InIsMoved;
}
