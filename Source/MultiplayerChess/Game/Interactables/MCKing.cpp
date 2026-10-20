#include "Game/Interactables/MCKing.h"
#include "Game/Interactables/MCTableSlot.h"
#include "Net/UnrealNetwork.h"

AMCKing::AMCKing()
{
	ObjectType = E_ObjectType::King;
}

void AMCKing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCKing, IsKingChecked);
}

void AMCKing::CalcPossibility()
{
	Super::CalcPossibility();

	if (!IsKingChecked)
	{
		AddCastlingPossibilities();
	}
	AddDiagonalPossibilities(1);
	AddStraightPossibilities(1);
}

TArray<FName> AMCKing::GetKingSurroundings() const
{
	TArray<FName> TempSurroundings;
	for (const FName& Element : GetDiagonalSurroundings(1))
	{
		TempSurroundings.AddUnique(Element);
	}
	for (const FName& Element : GetStraightSurroundings(1))
	{
		TempSurroundings.AddUnique(Element);
	}
	return TempSurroundings;
}

void AMCKing::MarkChecked(bool IsChecked)
{
	IsKingChecked = IsChecked;
	OnRep_IsKingChecked();
}

void AMCKing::OnRep_IsKingChecked()
{
	AMCTableSlot* Slot = nullptr;
	bool bSlotValid = false;
	GetCurrentSlot(Slot, bSlotValid);
	if (bSlotValid)
	{
		Slot->Glow();
	}
}

bool AMCKing::CanCheckToSlot(FName InSlotName)
{
	return GetKingSurroundings().Contains(InSlotName);
}

void AMCKing::S_MarkChecked_Implementation(bool IsChecked)
{
	IsKingChecked = IsChecked;
	OnRep_IsKingChecked();
}

void AMCKing::M_MarkChecked(bool IsChecked)
{
}
