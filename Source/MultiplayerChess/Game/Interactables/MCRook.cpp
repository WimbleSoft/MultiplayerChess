#include "Game/Interactables/MCRook.h"

AMCRook::AMCRook()
{
	ObjectType = E_ObjectType::Rook;
}

void AMCRook::CalcPossibility()
{
	Super::CalcPossibility();
	AddStraightPossibilities(8);
}

bool AMCRook::CalcCheckingKing()
{
	Super::CalcCheckingKing();
	// As in the Blueprint: it asks about slot "None", so this is always false.
	return CanCheckToSlot(NAME_None);
}

bool AMCRook::CanCheckToSlot(FName InSlotName)
{
	TArray<FName> ValidStraightPossibilities, AllInRangeStraightPossibilities;
	GetStraightPossibilities(8, ValidStraightPossibilities, AllInRangeStraightPossibilities);
	return ValidStraightPossibilities.Contains(InSlotName);
}
