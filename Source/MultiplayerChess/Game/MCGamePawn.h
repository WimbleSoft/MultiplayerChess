#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MCGamePawn.generated.h"

class UCameraComponent;

// Port of BP_GamePawn.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCGamePawn : public APawn
{
	GENERATED_BODY()

public:
	AMCGamePawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	virtual void BeginPlay() override;
};
