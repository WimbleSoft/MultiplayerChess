#include "Game/Interactables/MCKnight.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCTableSlot.h"

AMCKnight::AMCKnight()
{
	ObjectType = E_ObjectType::Knight;
}

void AMCKnight::CalcPossibility()
{
	Super::CalcPossibility();

	for (const FName& Element : GetKnightPossibilities())
	{
		Server_AddPossibleSlot(Element);
	}
}

bool AMCKnight::CheckNextPosition(int32 Col, int32 Row, FName& OutSlot) const
{
	bool InRange = false;
	GetSlotNameByColRow(Col, Row, OutSlot, InRange);
	if (!InRange)
	{
		return false;
	}
	AMCChessTable* Table = GetChessTable();
	AMCTableSlot* Slot = Table ? Table->GetTableSlotBySocketName(OutSlot) : nullptr;
	AMCChessObjectBase* Attached = Slot ? Slot->AttachedObject.Get() : nullptr;
	if (!IsValid(Attached))
	{
		return true;
	}
	bool IsEmpty = true, IsFull = false;
	Slot->GetIsEmpty(IsEmpty, IsFull);
	if (!IsFull)
	{
		return true;
	}
	// Own piece -> "then" (not added); enemy piece -> "addSlot".
	return !(IsFull && !((!IsAway == Attached->IsAway)));
}

TArray<FName> AMCKnight::GetKnightPossibilities() const
{
	TArray<FName> TempPossibilities;
	int32 Col = 0, Row = 0;
	GetSlotColRow(SlotName, Col, Row);

	// Each pair: the second jump is only checked when the first one is NOT addable ("then" output),
	// exactly like the Blueprint wiring.
	const int32 Pairs[4][4] = {
		{ 1, 2, 2, 1 },
		{ 1, -2, -2, 1 },
		{ -1, 2, 2, -1 },
		{ -1, -2, -2, -1 },
	};
	for (const auto& Pair : Pairs)
	{
		FName First;
		if (CheckNextPosition(Col + Pair[0], Row + Pair[1], First))
		{
			TempPossibilities.Add(First);
		}
		else
		{
			FName Second;
			if (CheckNextPosition(Col + Pair[2], Row + Pair[3], Second))
			{
				TempPossibilities.Add(Second);
			}
		}
	}
	return TempPossibilities;
}

bool AMCKnight::CalcCheckingKing()
{
	Super::CalcCheckingKing();
	// As in the Blueprint: it asks about slot "None", so this is always false.
	return CanCheckToSlot(NAME_None);
}

bool AMCKnight::CanCheckToSlot(FName InSlotName)
{
	return GetKnightPossibilities().Contains(InSlotName);
}
