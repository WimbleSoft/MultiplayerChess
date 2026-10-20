#include "Game/MCGamePawn.h"
#include "Game/MCGame_PC.h"
#include "Camera/CameraComponent.h"

AMCGamePawn::AMCGamePawn()
{
	bReplicates = true;
}

void AMCGamePawn::BeginPlay()
{
	Super::BeginPlay();

	if (AMCGame_PC* GamePC = Cast<AMCGame_PC>(GetController()))
	{
		if (UCameraComponent* Camera = FindComponentByClass<UCameraComponent>())
		{
			Camera->SetRelativeRotation(GamePC->GlobalPlayerIndex == 0 ? FRotator(-90.f, 0.f, 0.f) : FRotator(-90.f, 0.f, 180.f));
		}
	}
}
