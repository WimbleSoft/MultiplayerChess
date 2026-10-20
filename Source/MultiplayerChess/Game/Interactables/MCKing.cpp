#include "Game/Interactables/MCKing.h"
#include "Game/Interactables/MCTableSlot.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AMCKing::AMCKing()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Geometry/Meshes/ChessObjects/SM_King.SM_King"));
	SM_ChessObject->SetStaticMesh(Mesh.Object);
	ObjectType = E_ObjectType::King;
}

void AMCKing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCKing, IsKingChecked);
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

void AMCKing::S_MarkChecked_Implementation(bool IsChecked)
{
	MarkChecked(IsChecked);
}
