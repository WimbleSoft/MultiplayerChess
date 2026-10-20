#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/E_ObjectType.h"
#include "Game/MCChessObjectInterface.h"
#include "MCChessObjectBase.generated.h"

class AMCChessTable;
class AMCKing;
class AMCTableSlot;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

// Port of BP_ChessObjectBase.
// The components (SM_ChessObject, TextRender) stay in the Blueprint, so meshes and per-piece overrides are kept;
// this class looks them up by name.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCChessObjectBase : public AActor, public IMCChessObjectInterface
{
	GENERATED_BODY()

public:
	AMCChessObjectBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> DMI_Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	E_ObjectType ObjectType = E_ObjectType::Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsAway, meta = (ExposeOnSpawn = "true"))
	bool IsAway = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_SlotName, meta = (ExposeOnSpawn = "true"))
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsSelected)
	bool IsSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<FName> PossibleSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	bool IsMoved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	bool IsChecking = false;

	// Materials the Blueprint referenced as literals.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> HomeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> AwayMaterial;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// Blueprint "Set w/ Notify": sets the variable and calls its OnRep function locally.
	void SetIsAwayNotify(bool bNewValue);
	void SetSlotNameNotify(FName NewValue);
	void SetIsSelectedNotify(bool bNewValue);

	UFUNCTION(BlueprintCallable)
	void ClickGlow(bool IsGlowing);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCChessTable* GetChessTable() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetSlotColRow(FName InName, int32& Column, int32& Row) const;

	UFUNCTION(BlueprintCallable)
	void HoverGlow(bool Glow);

	UFUNCTION()
	void OnRep_IsAway();

	UFUNCTION()
	void OnRep_IsSelected();

	UFUNCTION()
	void OnRep_SlotName();

	UFUNCTION(BlueprintCallable)
	void CalcHover(bool IsHovering);

	UFUNCTION(BlueprintCallable)
	virtual void CalcPossibility();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetCurrentSlot(AMCTableSlot*& Slot, bool& bIsValid) const;

	UFUNCTION(BlueprintCallable)
	void AddCastlingPossibilities();

	UFUNCTION(BlueprintCallable)
	void GetColPossbilitiesByParams(int32 ToCol, int32 MaxMovement, TArray<FName>& ColPossibilities, TArray<FName>& AllColSurroundings) const;

	UFUNCTION(BlueprintCallable)
	void GetDiagonalPossbilitiesByParams(int32 A, int32 B, int32 MaxMovement, TArray<FName>& Possibilities, TArray<FName>& AllInRangePossibilities) const;

	UFUNCTION(BlueprintCallable)
	void GetRowPossibilitiesByParams(int32 ToRow, int32 LastIndex, TArray<FName>& RowPossibilities, TArray<FName>& AllRowSurroundings) const;

	UFUNCTION(BlueprintCallable)
	void GetDiagonalPossibilities(int32 MaxMovement, TArray<FName>& ValidDiagonalPossibilities, TArray<FName>& AllDiagonalPossibilities) const;

	UFUNCTION(BlueprintCallable)
	virtual bool CalcCheckingKing();

	UFUNCTION(BlueprintCallable)
	virtual bool CanCheckToSlot(FName InSlotName);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetSlotNameByColRow(int32 Col, int32 Row, FName& SocketName, bool& InRange) const;

	UFUNCTION(BlueprintCallable)
	bool GetIsObjectBelongToPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable)
	void AddDiagonalPossibilities(int32 MaxMovement);

	UFUNCTION(BlueprintCallable)
	void AddStraightPossibilities(int32 MaxMovement);

	UFUNCTION(BlueprintCallable)
	void GetCastlingPossibilities(TArray<FName>& KingFrom, TArray<FName>& KingTo, bool& IsCastlingPossible, TArray<FName>& RookFrom, TArray<FName>& RookTo) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsSlotEmpty(FName InSlotName) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetSlotByName(FName InSlotName, AMCTableSlot*& Slot, bool& bIsValid) const;

	UFUNCTION(BlueprintCallable)
	bool IsObjectOnSlotWithType(FName CurrentSlot, FName SlotToCheck, E_ObjectType ObjectTypeToCheck) const;

	UFUNCTION(BlueprintCallable)
	void GetStraightPossibilities(int32 MaxMovement, TArray<FName>& ValidStraightPossibilities, TArray<FName>& AllInRangeStraightPossibilities) const;

	UFUNCTION(BlueprintCallable)
	void GetKingPossibilities(TArray<FName>& Possibilities, TArray<FName>& NotValidPossibilities) const;

	UFUNCTION(BlueprintCallable)
	void GetOpponentKing(AMCKing*& OpponentKing, bool& bIsValid) const;

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetDiagonalSurroundings(int32 MaxMovement) const;

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetStraightSurroundings(int32 MaxMovement) const;

	// ---- EventGraph ----
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_IsSelected(bool InIsSelected);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_IsSelected(bool InIsSelected);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_Hover(bool IsHovering);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetIsAway(bool InIsAway);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SetIsAway(bool InIsAway);

	UFUNCTION(BlueprintCallable)
	void ShowPossibilities();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_CalcPossibility();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DeCalcPossibility();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_CalcPossibility();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_DeCalcPossibility();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_AddPossibleSlot(FName Socket);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_AddPossibleSlot(FName Slot);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ClearPossibleSlots();

	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable)
	void Multicast_ClearPossibleSlots();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetSocketName(FName SocketName);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RotText();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RotText(int32 A);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void S_SetIsMoved(bool InIsMoved);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void M_SetIsMoved(bool InIsMoved);

	// BPI_ChessObject (empty implementations in the Blueprint)
	virtual void MoveToSocket_Implementation(FName Socket) override {}
	virtual void Click_Implementation() override {}
	virtual void Hover_Implementation(bool IsOnHover) override {}

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SM_ChessObjectComponent;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UTextRenderComponent> TextRenderComponent;

	void FindBlueprintComponents();

	UFUNCTION()
	void OnMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent);
	UFUNCTION()
	void OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent);
	UFUNCTION()
	void OnMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	// Shared body of OnRep_IsAway and the construction script.
	void ApplyAwayMaterial();
};
