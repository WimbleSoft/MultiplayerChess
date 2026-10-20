#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MCSlotInterface.generated.h"

// Port of BPI_Slot.
UINTERFACE(BlueprintType, MinimalAPI)
class UMCSlotInterface : public UInterface
{
	GENERATED_BODY()
};

class MULTIPLAYERCHESS_API IMCSlotInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowPossibles(FName SlotName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HidePossibles(FName SlotName);
};
