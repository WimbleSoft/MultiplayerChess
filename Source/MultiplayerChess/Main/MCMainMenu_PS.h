#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MCMainMenu_PS.generated.h"

// Port of BP_MainMenu_PS (empty BeginPlay/Tick in the Blueprint).
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCMainMenu_PS : public APlayerState
{
	GENERATED_BODY()
};
