#include "Game/Interactables/MCTableSlot.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCKing.h"
#include "Game/MCGame_GM.h"
#include "Game/MCGame_PC.h"
#include "MultiplayerChess.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

AMCTableSlot::AMCTableSlot()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	SM_Slot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Slot"));
	RootComponent = SM_Slot;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SlotMesh(TEXT("/Game/Geometry/Meshes/Table/SM_Slot.SM_Slot"));
	SM_Slot->SetStaticMesh(SlotMesh.Object);

	TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
	TextRender->SetupAttachment(SM_Slot);
	TextRender->SetRelativeLocation(FVector(0.f, 0.f, 0.2f));
	TextRender->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	TextRender->SetRelativeScale3D(FVector(0.1f));
	TextRender->SetText(INVTEXT("00"));
	TextRender->SetWorldSize(26.f);
	TextRender->SetHorizontalAlignment(EHTA_Center);
	TextRender->SetVerticalAlignment(EVRTA_TextCenter);
	TextRender->SetTextRenderColor(FColor(255, 0, 0, 255));
	TextRender->SetCastShadow(false);
	TextRender->SetGenerateOverlapEvents(false);
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

void AMCTableSlot::ApplyAwayMaterial()
{
	UMaterialInterface* Parent = IsAway ? AwayMaterial.LoadSynchronous() : HomeMaterial.LoadSynchronous();
	DMI_Slot = UMaterialInstanceDynamic::Create(Parent, this);
	if (SM_Slot)
	{
		SM_Slot->SetMaterial(0, DMI_Slot);
	}
}

void AMCTableSlot::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyAwayMaterial();
}

void AMCTableSlot::BeginPlay()
{
	Super::BeginPlay();

	if (SM_Slot)
	{
		SM_Slot->OnBeginCursorOver.AddUniqueDynamic(this, &AMCTableSlot::OnSlotBeginCursorOver);
		SM_Slot->OnEndCursorOver.AddUniqueDynamic(this, &AMCTableSlot::OnSlotEndCursorOver);
		SM_Slot->OnClicked.AddUniqueDynamic(this, &AMCTableSlot::OnSlotClicked);
	}

	AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0));
	if (!GamePC)
	{
		MC_ACCESSED_NONE("AsBP Game PC");
	}
	const bool bIsAwayPlayer = GamePC && GamePC->GlobalPlayerIndex == 1;
	if (TextRender)
	{
		TextRender->SetRelativeRotation(bIsAwayPlayer ? FRotator(90.f, 0.f, 0.f) : FRotator(90.f, 180.f, 0.f));
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
	if (TextRender)
	{
		TextRender->SetText(FText::FromName(SlotName));
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
	// Refresh the colour, so a slot a checked king leaves stops showing CheckedColor.
	Glow();
}
