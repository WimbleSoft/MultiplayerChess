#include "Game/Interactables/MCRook.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AMCRook::AMCRook()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Geometry/Meshes/ChessObjects/SM_Rook.SM_Rook"));
	SM_ChessObject->SetStaticMesh(Mesh.Object);
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
