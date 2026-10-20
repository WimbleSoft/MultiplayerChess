#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MCLobby_GS.generated.h"

// Port of BP_Lobby_GS (empty BeginPlay/Tick in the Blueprint).
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCLobby_GS : public AGameStateBase
{
	GENERATED_BODY()
};
