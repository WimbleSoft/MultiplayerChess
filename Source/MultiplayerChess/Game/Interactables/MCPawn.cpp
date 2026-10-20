#include "Game/Interactables/MCPawn.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCKing.h"
#include "Game/Interactables/MCTableSlot.h"

AMCPawn::AMCPawn()
{
	ObjectType = E_ObjectType::Pawn;
	Value = 100;
}

void AMCPawn::CalcPossibility()
{
	Super::CalcPossibility();

	for (const FName& Element : GetPawnPossibilities())
	{
		Server_AddPossibleSlot(Element);
	}
}

TArray<FName> AMCPawn::GetPawnPossibilities() const
{
	TArray<FName> TempPossibilities;
	AMCChessTable* Table = GetChessTable();
	int32 Col = 0, Row = 0;
	GetSlotColRow(SlotName, Col, Row);

	auto SlotAt = [Table](FName Name) -> AMCTableSlot*
	{
		return Table ? Table->GetTableSlotBySocketName(Name) : nullptr;
	};
	auto IsSlotFull = [](AMCTableSlot* Slot)
	{
		bool IsEmpty = true, IsFull = false;
		if (Slot)
		{
			Slot->GetIsEmpty(IsEmpty, IsFull);
		}
		return IsFull;
	};
	auto IsSlotEmptyHere = [](AMCTableSlot* Slot)
	{
		bool IsEmpty = true, IsFull = false;
		if (Slot)
		{
			Slot->GetIsEmpty(IsEmpty, IsFull);
		}
		else
		{
			IsEmpty = false; // GetIsEmpty on None returns the default (false)
		}
		return IsEmpty;
	};

	// Diagonal Right
	{
		FName Name;
		bool InRange = false;
		GetSlotNameByColRow(Col + (IsAway ? 1 : -1), Row + (IsAway ? -1 : 1), Name, InRange);
		if (InRange)
		{
			AMCTableSlot* Slot = SlotAt(Name);
			AMCChessObjectBase* Attached = Slot ? Slot->AttachedObject.Get() : nullptr;
			if (IsValid(Attached) && IsSlotFull(Slot) && (!IsAway == Attached->IsAway))
			{
				TempPossibilities.Add(Name);
			}
		}
	}
	// Diagonal Left
	{
		FName Name;
		bool InRange = false;
		GetSlotNameByColRow(Col + (IsAway ? -1 : 1), Row + (IsAway ? -1 : 1), Name, InRange);
		if (InRange)
		{
			AMCTableSlot* Slot = SlotAt(Name);
			AMCChessObjectBase* Attached = Slot ? Slot->AttachedObject.Get() : nullptr;
			if (IsValid(Attached) && IsSlotFull(Slot) && (!IsAway == Attached->IsAway))
			{
				TempPossibilities.Add(Name);
			}
		}
	}
	// One Step Ahead Calc
	{
		FName Name;
		bool InRange = false;
		GetSlotNameByColRow(Col, Row + (IsAway ? -1 : 1), Name, InRange);
		if (InRange && IsSlotEmptyHere(SlotAt(Name)))
		{
			TempPossibilities.Add(Name);
		}
	}
	// Two Step Ahead Calc
	{
		FName TwoStep;
		bool InRange = false;
		GetSlotNameByColRow(Col, Row + (IsAway ? -2 : 2), TwoStep, InRange);
		if (InRange && IsSlotEmptyHere(SlotAt(TwoStep)) && (IsAway ? 7 : 2) == Row)
		{
			FName OneStep;
			bool OneStepInRange = false;
			GetSlotNameByColRow(Col, Row + (IsAway ? -1 : 1), OneStep, OneStepInRange);
			if (OneStepInRange && IsSlotEmptyHere(SlotAt(OneStep)))
			{
				TempPossibilities.Add(TwoStep);
			}
		}
	}
	return TempPossibilities;
}

bool AMCPawn::CalcCheckingKing()
{
	Super::CalcCheckingKing();

	AMCKing* OpponentKing = nullptr;
	bool bKingValid = false;
	GetOpponentKing(OpponentKing, bKingValid);
	if (bKingValid)
	{
		return CanCheckToSlot(OpponentKing->SlotName);
	}
	return false;
}

bool AMCPawn::CanCheckToSlot(FName InSlotName)
{
	return GetPawnPossibilities().Contains(InSlotName);
}
