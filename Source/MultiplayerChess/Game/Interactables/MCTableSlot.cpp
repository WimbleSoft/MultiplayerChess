#include "Game/Interactables/MCTableSlot.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCKing.h"
#include "Game/MCGame_GM.h"
#include "Game/MCGame_PC.h"
#include "MultiplayerChess.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

AMCTableSlot::AMCTableSlot()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	HomeMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Materials/M_HomeSlot.M_HomeSlot")));
	AwayMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Materials/M_AwaySlot.M_AwaySlot")));
}

void AMCTableSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCTableSlot, IsAway);
	DOREPLIFETIME(AMCTableSlot, IsActive);
	DOREPLIFETIME(AMCTableSlot, IsClicked);
	DOREPLIFETIME(AMCTableSlot, SlotName);
	DOREPLIFETIME(AMCTableSlot, AttachedObject);
}

void AMCTableSlot::FindBlueprintComponents()
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component->GetFName() == TEXT("SM_Slot"))
		{
			SM_SlotComponent = Cast<UStaticMeshComponent>(Component);
		}
		else if (Component->GetFName() == TEXT("TextRender"))
		{
			TextRenderComponent = Cast<UTextRenderComponent>(Component);
		}
	}
}

void AMCTableSlot::ApplyAwayMaterial()
{
	UMaterialInterface* Parent = IsAway ? AwayMaterial.LoadSynchronous() : HomeMaterial.LoadSynchronous();
	DMI_Slot = UMaterialInstanceDynamic::Create(Parent, this);
	if (SM_SlotComponent)
	{
		SM_SlotComponent->SetMaterial(0, DMI_Slot);
	}
}

void AMCTableSlot::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FindBlueprintComponents();
	ApplyAwayMaterial();
}

void AMCTableSlot::BeginPlay()
{
	Super::BeginPlay();

	FindBlueprintComponents();
	if (SM_SlotComponent)
	{
		SM_SlotComponent->OnBeginCursorOver.AddUniqueDynamic(this, &AMCTableSlot::OnSlotBeginCursorOver);
		SM_SlotComponent->OnEndCursorOver.AddUniqueDynamic(this, &AMCTableSlot::OnSlotEndCursorOver);
		SM_SlotComponent->OnClicked.AddUniqueDynamic(this, &AMCTableSlot::OnSlotClicked);
	}

	AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0));
	if (!GamePC)
	{
		MC_ACCESSED_NONE("AsBP Game PC");
	}
	const bool bIsAwayPlayer = GamePC && GamePC->GlobalPlayerIndex == 1;
	if (TextRenderComponent)
	{
		TextRenderComponent->SetRelativeRotation(bIsAwayPlayer ? FRotator(90.f, 0.f, 0.f) : FRotator(90.f, 180.f, 0.f));
	}
}

void AMCTableSlot::SetIsAwayNotify(bool bNewValue)
{
	IsAway = bNewValue;
	OnRep_IsAway();
}

void AMCTableSlot::SetIsActiveNotify(bool bNewValue)
{
	IsActive = bNewValue;
	OnRep_IsActive();
}

void AMCTableSlot::SetIsClickedNotify(bool bNewValue)
{
	IsClicked = bNewValue;
	OnRep_IsClicked();
}

void AMCTableSlot::OnRep_IsAway()
{
	if (!SM_SlotComponent)
	{
		FindBlueprintComponents();
	}
	ApplyAwayMaterial();
}

void AMCTableSlot::OnRep_IsActive()
{
	Glow();
}

void AMCTableSlot::OnRep_IsClicked()
{
	Glow();
}

void AMCTableSlot::GetIsEmpty(bool& IsEmpty, bool& IsFull) const
{
	IsEmpty = !IsValid(AttachedObject);
	IsFull = IsValid(AttachedObject);
}

AMCGame_GM* AMCTableSlot::GetGameMode() const
{
	return Cast<AMCGame_GM>(UGameplayStatics::GetGameMode(this));
}

AMCChessTable* AMCTableSlot::GetTable()
{
	if (AMCChessTable* Table = Cast<AMCChessTable>(GetAttachParentActor()))
	{
		return Table;
	}
	UKismetSystemLibrary::PrintString(this, TEXT("Hello"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
	return nullptr;
}

void AMCTableSlot::SetSlotName(FName InSlotName)
{
	SlotName = InSlotName;
	if (!TextRenderComponent)
	{
		FindBlueprintComponents();
	}
	if (TextRenderComponent)
	{
		TextRenderComponent->SetText(FText::FromName(SlotName));
	}
}

void AMCTableSlot::Glow()
{
	if (!DMI_Slot)
	{
		MC_ACCESSED_NONE("DMI_Slot");
		return;
	}
	DMI_Slot->SetScalarParameterValue(TEXT("Glow"), (IsActive || IsClicked) ? 10.f : 0.f);

	// Same nesting as the Blueprint's Select nodes (hover shows ClickColor and click shows HoverColor there).
	FLinearColor Color = IsAway ? AwayColor : HomeColor;
	Color = IsActive ? ActiveColor : Color;
	Color = IsHover ? ClickColor : Color;
	Color = IsClicked ? HoverColor : Color;
	const AMCKing* King = Cast<AMCKing>(AttachedObject);
	const bool bKingChecked = King ? King->IsKingChecked : false;
	Color = bKingChecked ? CheckedColor : Color;
	DMI_Slot->SetVectorParameterValue(TEXT("Color"), Color);
}

void AMCTableSlot::OnSlotBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (IsActive)
	{
		IsHover = true;
		Glow();
	}
}

void AMCTableSlot::OnSlotEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	IsHover = false;
	Glow();
}

void AMCTableSlot::OnSlotClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (IsActive)
	{
		AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0));
		if (!GamePC)
		{
			MC_ACCESSED_NONE("AsBP Game PC");
			return;
		}
		GamePC->Server_ClickedToSlot(this);
	}
}

void AMCTableSlot::Server_SetIsAway_Implementation(bool InIsAway)
{
	Multi_SetIsAway(InIsAway);
}

void AMCTableSlot::Multi_SetIsAway_Implementation(bool InIsAway)
{
	SetIsAwayNotify(InIsAway);
}

void AMCTableSlot::Server_SetIsActive_Implementation(bool InIsActive)
{
	Multicast_SetIsActive(InIsActive);
}

void AMCTableSlot::Multicast_SetIsActive_Implementation(bool InIsActive)
{
	SetIsActiveNotify(InIsActive);
}

void AMCTableSlot::ShowPossibles_Implementation(FName InSlotName)
{
	Server_SetIsActive(InSlotName == SlotName);
}

void AMCTableSlot::Server_SlotClicked_Implementation(bool InIsClicked)
{
	Multi_SlotClicked(InIsClicked);
}

void AMCTableSlot::Multi_SlotClicked_Implementation(bool InIsClicked)
{
	SetIsClickedNotify(InIsClicked);
}

void AMCTableSlot::Server_SetAttachedObject_Implementation(AMCChessObjectBase* NewObject)
{
	Multi_SetAttachedObject(NewObject);
}

void AMCTableSlot::Multi_SetAttachedObject_Implementation(AMCChessObjectBase* NewObject)
{
	AttachedObject = NewObject;
}
