#pragma once

#include "CoreMinimal.h"
#include "Common/MCBasePC.h"
#include "Common/E_ObjectType.h"
#include "Common/S_PlayerInfo.h"
#include "MCGame_PC.generated.h"

class AMCChessObjectBase;
class AMCChessTable;
class AMCGame_GM;
class AMCTableSlot;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMCObjectClicked, AMCChessObjectBase*, Clicked);

// Port of BP_Game_PC.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCGame_PC : public AMCBasePC
{
	GENERATED_BODY()

public:
	AMCGame_PC();

	// Event dispatcher declared in the Blueprint; nothing binds or calls it.
	UPROPERTY(BlueprintAssignable)
	FMCObjectClicked ObjectClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TObjectPtr<AMCChessObjectBase> SelectedChessObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TObjectPtr<AMCChessObjectBase> NewSelectedChessObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AMCGame_GM> GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	int32 GlobalPlayerIndex = 0;

	// WBP_ObjectSelector; the widget stays a Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> ObjectSelectorWidgetClass;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ObjectSelected(AMCChessObjectBase* NewSelectedObject);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SwitchSelection(AMCChessObjectBase* SelectedObject, AMCChessObjectBase* NewSelectedObject);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DoCastling(AMCChessObjectBase* KingObject, FName KingNewSlot, AMCChessObjectBase* RookObject, FName RookNewSlot);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ClickedToObject(AMCGame_PC* PlayerController, AMCChessObjectBase* NewSelectedObject);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ClickedToSlot(AMCTableSlot* NewSlot);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetGlobalPlayerIndex(int32 InGlobalPlayerIndex);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SetGlobalPlayerIndex(int32 InGlobalPlayerIndex);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_CreatePawnReplacementSelector(FName SlotToSwitch, APlayerController* OwningPlayer);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TryPawnReplacement(FName Slot, E_ObjectType NewObjectType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCChessTable* GetChessTable() const;

	// Not called anywhere in the Blueprint.
	UFUNCTION(BlueprintCallable)
	bool DetectCastling(AMCChessObjectBase* NewSelectedObject, AMCChessObjectBase* SelectedObject);

	// Empty in the Blueprint.
	UFUNCTION(BlueprintCallable)
	void DetectSwitching(AMCChessObjectBase* NewSelectedObject);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsSlotEmpty(FName SlotName) const;

	UFUNCTION(BlueprintCallable)
	void SetPlayerInfo(const FS_PlayerInfo& PlayerInfo);

	UFUNCTION(BlueprintCallable)
	bool IsObjectBelongToPlayer(int32 PlayerIndex, AMCChessObjectBase* NewSelectedObject);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetSlotByName(FName SlotName, AMCTableSlot*& Slot, bool& bIsValid) const;
};
