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
class USkeletalMeshComponent;
class UTextRenderComponent;

// Port of BP_ChessTable.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCChessTable : public AActor
{
	GENERATED_BODY()

public:
	AMCChessTable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SM_Table;

	// Parent of the 32 coordinate labels (A_Home..H_Away, 1_Home..8_Away).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Texts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<UTextRenderComponent>> Labels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<TObjectPtr<AMCTableSlot>> AllSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> DT_ObjectMapper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<TObjectPtr<AMCChessObjectBase>> AllObjects;

	// Spawned for each of the 64 squares (BP_TableSlot_C in the Blueprint).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMCTableSlot> TableSlotClass;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

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
	UChildActorComponent* AddChildActorComponent(TSubclassOf<AActor> ChildClass);

	// Child actor components that BeginPlay hands back out, in creation order, instead of adding new ones.
	TArray<TObjectPtr<UChildActorComponent>> ReusableChildActorComponents;
};
