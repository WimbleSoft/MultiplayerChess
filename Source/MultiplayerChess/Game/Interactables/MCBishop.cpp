#include "Game/Interactables/MCBishop.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AMCBishop::AMCBishop()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Geometry/Meshes/ChessObjects/SM_Bishop.SM_Bishop"));
	SM_ChessObject->SetStaticMesh(Mesh.Object);
	ObjectType = E_ObjectType::Bishop;
	Value = 300;
}
