#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MCMainMenu_GM.generated.h"

// Port of BP_MainMenu_GM (no logic, only class defaults).
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCMainMenu_GM : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMCMainMenu_GM();
};
