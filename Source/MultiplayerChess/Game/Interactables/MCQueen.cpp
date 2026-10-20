#include "Game/Interactables/MCQueen.h"

AMCQueen::AMCQueen()
{
	ObjectType = E_ObjectType::Queen;
}

void AMCQueen::CalcPossibility()
{
	Super::CalcPossibility();
	AddDiagonalPossibilities(8);
	AddStraightPossibilities(8);
}

bool AMCQueen::CalcCheckingKing()
{
	Super::CalcCheckingKing();
	// As in the Blueprint: it asks about slot "None", so this is always false.
	return CanCheckToSlot(NAME_None);
}

bool AMCQueen::CanCheckToSlot(FName InSlotName)
{
	// As in the Blueprint: this reads the "All..." lists, which are only filled for a King,
	// so the queen never reports a check.
	TArray<FName> ValidDiagonalPossibilities, AllDiagonalPossibilities;
	GetDiagonalPossibilities(8, ValidDiagonalPossibilities, AllDiagonalPossibilities);
	if (AllDiagonalPossibilities.Contains(InSlotName))
	{
		return true;
	}
	TArray<FName> ValidStraightPossibilities, AllInRangeStraightPossibilities;
	GetStraightPossibilities(8, ValidStraightPossibilities, AllInRangeStraightPossibilities);
	return AllInRangeStraightPossibilities.Contains(InSlotName);
}
