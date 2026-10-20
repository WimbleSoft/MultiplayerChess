#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MCGamePawn.generated.h"

// Port of BP_GamePawn. The Camera component stays in the Blueprint.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCGamePawn : public APawn
{
	GENERATED_BODY()

public:
	AMCGamePawn();

	virtual void BeginPlay() override;
};
