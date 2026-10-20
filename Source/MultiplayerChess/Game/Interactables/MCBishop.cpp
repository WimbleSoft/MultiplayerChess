#include "Game/Interactables/MCBishop.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AMCBishop::AMCBishop()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Geometry/Meshes/ChessObjects/SM_Bishop.SM_Bishop"));
	SM_ChessObject->SetStaticMesh(Mesh.Object);
	ObjectType = E_ObjectType::Bishop;
}

void AMCBishop::CalcPossibility()
{
	Super::CalcPossibility();
	AddDiagonalPossibilities(8);
}

bool AMCBishop::CalcCheckingKing()
{
	Super::CalcCheckingKing();
	// As in the Blueprint: it asks about slot "None", so this is always false.
	return CanCheckToSlot(NAME_None);
}

bool AMCBishop::CanCheckToSlot(FName InSlotName)
{
	TArray<FName> ValidDiagonalPossibilities, AllDiagonalPossibilities;
	GetDiagonalPossibilities(8, ValidDiagonalPossibilities, AllDiagonalPossibilities);
	return ValidDiagonalPossibilities.Contains(InSlotName);
}
