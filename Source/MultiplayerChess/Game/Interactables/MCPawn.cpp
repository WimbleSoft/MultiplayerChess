#include "Game/Interactables/MCPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AMCPawn::AMCPawn()
{
	ObjectType = E_ObjectType::Pawn;
	Value = 100;
}
