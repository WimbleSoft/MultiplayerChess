#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/E_ObjectType.h"
#include "S_ObjectMap.generated.h"

class AMCChessObjectBase;

// Port of /Game/Blueprints/Common/S_ObjectMap. Row struct of DT_ObjectMap.
USTRUCT(BlueprintType)
struct FS_ObjectMap : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMCChessObjectBase> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAway = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	E_ObjectType ObjectType = E_ObjectType::Bishop;
};
