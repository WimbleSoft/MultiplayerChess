#include "Game/MCGamePawn.h"
#include "Game/MCGame_PC.h"
#include "Camera/CameraComponent.h"

AMCGamePawn::AMCGamePawn()
{
	bReplicates = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(DefaultSceneRoot);
	Camera->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	Camera->SetFieldOfView(25.f);
}

void AMCGamePawn::BeginPlay()
{
	Super::BeginPlay();

	AMCGame_PC* GamePC = Cast<AMCGame_PC>(GetController());
	if (GamePC && Camera)
	{
		Camera->SetRelativeRotation(GamePC->GlobalPlayerIndex == 0 ? FRotator(-90.f, 0.f, 0.f) : FRotator(-90.f, 0.f, 180.f));
	}
}
