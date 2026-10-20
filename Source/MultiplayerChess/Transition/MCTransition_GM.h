#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MCTransition_GM.generated.h"

// Port of BP_Transition_GM (empty BeginPlay/Tick in the Blueprint).
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCTransition_GM : public AGameModeBase
{
	GENERATED_BODY()
};
