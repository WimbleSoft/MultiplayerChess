#include "Lobby/MCLobbyPawn.h"
#include "Game/Interactables/MCKing.h"
#include "Components/ChildActorComponent.h"

void AMCLobbyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TArray<UChildActorComponent*> ChildActors;
	GetComponents(ChildActors);
	for (UChildActorComponent* ChildActor : ChildActors)
	{
		if (ChildActor->GetFName() == TEXT("ChildActor"))
		{
			if (AMCKing* King = Cast<AMCKing>(ChildActor->GetChildActor()))
			{
				// A Blueprint "Set" on a RepNotify variable also calls its OnRep function.
				King->SetIsAwayNotify(IsAway);
			}
		}
	}
}
