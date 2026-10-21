#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/MCSlotInterface.h"
#include "MCTableSlot.generated.h"

class AMCChessObjectBase;
class AMCChessTable;
class AMCGame_GM;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

// Port of BP_TableSlot.
UCLASS(Blueprintable)
class MULTIPLAYERCHESS_API AMCTableSlot : public AActor, public IMCSlotInterface
{
	GENERATED_BODY()

public:
	AMCTableSlot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SM_Slot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> TextRender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> DMI_Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsAway)
	bool IsAway = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsActive)
	bool IsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsClicked)
	bool IsClicked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsHover = false;

	// Replicated with a notify so clients show the square's name too (the Blueprint only set it on the server).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_SlotName)
	FName SlotName = TEXT("00");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TObjectPtr<AMCChessObjectBase> AttachedObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HomeColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AwayColor = FLinearColor(0.f, 0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ActiveColor = FLinearColor(0.f, 1.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ClickColor = FLinearColor(0.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HoverColor = FLinearColor(1.f, 1.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CheckedColor = FLinearColor(1.f, 0.f, 0.f, 1.f);

	// Materials the Blueprint referenced as literals.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> HomeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> AwayMaterial;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	void SetIsAwayNotify(bool bNewValue);
	void SetIsActiveNotify(bool bNewValue);
	void SetIsClickedNotify(bool bNewValue);

	UFUNCTION()
	void OnRep_IsAway();

	UFUNCTION()
	void OnRep_IsActive();

	UFUNCTION()
	void OnRep_IsClicked();

	UFUNCTION()
	void OnRep_SlotName();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetIsEmpty(bool& IsEmpty, bool& IsFull) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCGame_GM* GetGameMode() const;

	UFUNCTION(BlueprintCallable)
	AMCChessTable* GetTable();

	UFUNCTION(BlueprintCallable)
	void SetSlotName(FName InSlotName);

	UFUNCTION(BlueprintCallable)
	void Glow();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetIsAway(bool InIsAway);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SetIsAway(bool InIsAway);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetIsActive(bool InIsActive);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SetIsActive(bool InIsActive);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SlotClicked(bool InIsClicked);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SlotClicked(bool InIsClicked);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetAttachedObject(AMCChessObjectBase* NewObject);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SetAttachedObject(AMCChessObjectBase* NewObject);

	// BPI_Slot
	virtual void ShowPossibles_Implementation(FName InSlotName) override;
	virtual void HidePossibles_Implementation(FName InSlotName) override {}

protected:
	void ApplyAwayMaterial();

	UFUNCTION()
	void OnSlotBeginCursorOver(UPrimitiveComponent* TouchedComponent);
	UFUNCTION()
	void OnSlotEndCursorOver(UPrimitiveComponent* TouchedComponent);
	UFUNCTION()
	void OnSlotClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
};
