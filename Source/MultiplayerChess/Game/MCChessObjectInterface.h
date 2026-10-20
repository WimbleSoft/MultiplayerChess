#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MCChessObjectInterface.generated.h"

// Port of BPI_ChessObject.
UINTERFACE(BlueprintType, MinimalAPI)
class UMCChessObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class MULTIPLAYERCHESS_API IMCChessObjectInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void MoveToSocket(FName Socket);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Click();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Hover(bool IsOnHover);
};
