#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MCLobbyPawn.generated.h"

// Port of BP_LobbyPawn. The components (ChildActor showing BP_King, Camera) stay in the Blueprint.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCLobbyPawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAway = false;

	virtual void OnConstruction(const FTransform& Transform) override;
};
