#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MCLobbyPawn.generated.h"

class UCameraComponent;
class UChildActorComponent;

// Port of BP_LobbyPawn (not used by any game mode or map).
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCLobbyPawn : public APawn
{
	GENERATED_BODY()

public:
	AMCLobbyPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> ChildActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAway = false;

	virtual void OnConstruction(const FTransform& Transform) override;
};
