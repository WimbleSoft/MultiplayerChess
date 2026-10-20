#include "Lobby/MCLobbyPawn.h"
#include "Game/Interactables/MCKing.h"
#include "Camera/CameraComponent.h"
#include "Components/ChildActorComponent.h"

AMCLobbyPawn::AMCLobbyPawn()
{
	bReplicates = true;

	ChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor"));
	RootComponent = ChildActor;
	ChildActor->SetChildActorClass(AMCKing::StaticClass());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(ChildActor);
	Camera->SetFieldOfView(90.f);
}

void AMCLobbyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (AMCKing* King = ChildActor ? Cast<AMCKing>(ChildActor->GetChildActor()) : nullptr)
	{
		// A Blueprint "Set" on a RepNotify variable also calls its OnRep function.
		King->SetIsAwayNotify(IsAway);
	}
}
