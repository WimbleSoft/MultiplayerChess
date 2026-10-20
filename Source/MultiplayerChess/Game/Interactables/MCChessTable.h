#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/S_ObjectMap.h"
#include "MCChessTable.generated.h"

class AMCChessObjectBase;
class AMCTableSlot;
class UChildActorComponent;
class UDataTable;
class USceneComponent;

// Port of BP_ChessTable. Its components (SM_Table skeletal mesh, the coordinate TextRenders) stay in the Blueprint.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCChessTable : public AActor
{
	GENERATED_BODY()

public:
	AMCChessTable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<TObjectPtr<AMCTableSlot>> AllSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> DT_ObjectMapper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<TObjectPtr<AMCChessObjectBase>> AllObjects;

	// BP_TableSlot; spawned for each of the 64 squares.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AMCTableSlot> TableSlotClass;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable)
	void ClearSlotsObjects();

	UFUNCTION(BlueprintCallable)
	AMCTableSlot* GetTableSlotBySocketName(FName SocketName) const;

	UFUNCTION(BlueprintCallable)
	void SpawnObjects();

	UFUNCTION(BlueprintCallable)
	void SpawnSlots();

	UFUNCTION(BlueprintCallable)
	void GetObjectBySlotName(FName SlotName, AMCChessObjectBase*& ChessObject, bool& bIsValid) const;

	UFUNCTION(BlueprintCallable)
	void GetSlotByName(FName QuerySlotName, AMCTableSlot*& Slot, bool& bIsValid) const;

	UFUNCTION(BlueprintCallable)
	void SpawnObject(const FS_ObjectMap& S_ObjectMap);

	UFUNCTION(BlueprintCallable)
	TMap<FName, bool> GetAllSlotsOccupationInfo() const;

private:
	// Equivalent of the Blueprint "Add Child Actor Component" node.
	UChildActorComponent* AddChildActorComponent(TSubclassOf<AActor> ChildClass, FName ComponentName);

	USceneComponent* FindSceneComponent(FName ComponentName) const;

	// True while OnConstruction runs, so components made there are owned by the construction script like in the Blueprint.
	bool bRunningConstructionScript = false;
};
