#include "Game/Interactables/MCChessObjectBase.h"
#include "Common/MCCommonFunctions.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/Interactables/MCKing.h"
#include "Game/Interactables/MCTableSlot.h"
#include "Game/MCChessRules.h"
#include "Game/MCGame_PC.h"
#include "Game/MCSlotInterface.h"
#include "MultiplayerChess.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

AMCChessObjectBase::AMCChessObjectBase()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	SM_ChessObject = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_ChessObject"));
	RootComponent = SM_ChessObject;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PawnMesh(TEXT("/Game/Geometry/Meshes/ChessObjects/SM_Pawn.SM_Pawn"));
	SM_ChessObject->SetStaticMesh(PawnMesh.Object);

	TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
	TextRender->SetupAttachment(SM_ChessObject);
	TextRender->SetRelativeLocation(FVector(0.f, 0.f, 12.f));
	TextRender->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	TextRender->SetRelativeScale3D(FVector(0.1f));
	TextRender->SetText(INVTEXT("00"));
	TextRender->SetWorldSize(64.f);
	TextRender->SetHorizontalAlignment(EHTA_Center);
	TextRender->SetVerticalAlignment(EVRTA_TextCenter);
	TextRender->SetTextRenderColor(FColor(255, 0, 255, 255));

	HomeMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Geometry/Materials/MI_HomeChessObject.MI_HomeChessObject")));
	AwayMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Geometry/Materials/MI_AwayChessObject.MI_AwayChessObject")));
}

void AMCChessObjectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCChessObjectBase, ObjectType);
	DOREPLIFETIME(AMCChessObjectBase, IsAway);
	DOREPLIFETIME(AMCChessObjectBase, SlotName);
	DOREPLIFETIME(AMCChessObjectBase, IsSelected);
	DOREPLIFETIME(AMCChessObjectBase, PossibleSlots);
	DOREPLIFETIME(AMCChessObjectBase, IsMoved);
	DOREPLIFETIME(AMCChessObjectBase, IsChecking);
}

void AMCChessObjectBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyAwayMaterial();
}

void AMCChessObjectBase::BeginPlay()
{
	Super::BeginPlay();

	if (SM_ChessObject)
	{
		SM_ChessObject->OnBeginCursorOver.AddUniqueDynamic(this, &AMCChessObjectBase::OnMeshBeginCursorOver);
		SM_ChessObject->OnEndCursorOver.AddUniqueDynamic(this, &AMCChessObjectBase::OnMeshEndCursorOver);
		SM_ChessObject->OnClicked.AddUniqueDynamic(this, &AMCChessObjectBase::OnMeshClicked);
	}

	Server_RotText();
}

void AMCChessObjectBase::SetIsAwayNotify(bool bNewValue)
{
	IsAway = bNewValue;
	OnRep_IsAway();
}

void AMCChessObjectBase::SetSlotNameNotify(FName NewValue)
{
	SlotName = NewValue;
	OnRep_SlotName();
}

void AMCChessObjectBase::SetIsSelectedNotify(bool bNewValue)
{
	IsSelected = bNewValue;
	OnRep_IsSelected();
}

void AMCChessObjectBase::ClickGlow(bool IsGlowing)
{
	if (!DMI_Mesh)
	{
		MC_ACCESSED_NONE("DMI_Mesh");
		return;
	}
	DMI_Mesh->SetScalarParameterValue(TEXT("Glow"), IsGlowing ? 10.f : 0.f);
	const FLinearColor TeamColor = IsAway ? FLinearColor(0.f, 0.f, 0.f, 1.f) : FLinearColor(1.f, 1.f, 1.f, 1.f);
	DMI_Mesh->SetVectorParameterValue(TEXT("Color"), IsGlowing ? FLinearColor(1.f, 0.f, 0.f, 1.f) : TeamColor);
}

AMCChessTable* AMCChessObjectBase::GetChessTable() const
{
	return Cast<AMCChessTable>(GetParentActor());
}

void AMCChessObjectBase::HoverGlow(bool Glow)
{
	if (Glow)
	{
		if (!DMI_Mesh)
		{
			MC_ACCESSED_NONE("DMI_Mesh");
			return;
		}
		DMI_Mesh->SetScalarParameterValue(TEXT("Glow"), 10.f);
		DMI_Mesh->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.f, 0.f, 1.f, 1.f));
	}
	else
	{
		ClickGlow(IsSelected);
	}
}

void AMCChessObjectBase::ApplyAwayMaterial()
{
	UMaterialInterface* Parent = IsAway ? AwayMaterial.LoadSynchronous() : HomeMaterial.LoadSynchronous();
	DMI_Mesh = UMaterialInstanceDynamic::Create(Parent, this);
	if (SM_ChessObject)
	{
		SM_ChessObject->SetMaterial(0, DMI_Mesh);
	}
}

void AMCChessObjectBase::OnRep_IsAway()
{
	ApplyAwayMaterial();
}

void AMCChessObjectBase::OnRep_IsSelected()
{
	ClickGlow(IsSelected);
	if (IsSelected)
	{
		Server_CalcPossibility();
	}
	else
	{
		Server_DeCalcPossibility();
	}
}

void AMCChessObjectBase::OnRep_SlotName()
{
	if (TextRender)
	{
		TextRender->SetText(FText::FromName(SlotName));
	}
}

void AMCChessObjectBase::CalcHover(bool IsHovering)
{
	Client_Hover(IsHovering);
}

void AMCChessObjectBase::CalcPossibility()
{
	AMCChessTable* Table = GetChessTable();
	int32 Col = 0, Row = 0;
	if (!Table || !FMCChessRules::ParseSquare(SlotName, Col, Row))
	{
		return;
	}
	TArray<FMCMove> Moves;
	Table->BuildRules().GenerateLegalMoves(Col, Row, Moves);
	for (const FMCMove& Move : Moves)
	{
		Server_AddPossibleSlot(FMCChessRules::MakeSquareName(Move.ToCol, Move.ToRow));
	}
}

void AMCChessObjectBase::GetCurrentSlot(AMCTableSlot*& Slot, bool& bIsValid) const
{
	GetSlotByName(SlotName, Slot, bIsValid);
}

bool AMCChessObjectBase::GetIsObjectBelongToPlayer(int32 PlayerIndex)
{
	if (!(PlayerIndex >= 0 && PlayerIndex <= 1))
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Cannot find Game PC in PC list of Game GM"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 3.f);
		return false;
	}
	return (PlayerIndex == 1) == IsAway;
}

void AMCChessObjectBase::GetSlotByName(FName InSlotName, AMCTableSlot*& Slot, bool& bIsValid) const
{
	Slot = nullptr;
	bIsValid = false;
	if (AMCChessTable* Table = GetChessTable())
	{
		Table->GetSlotByName(InSlotName, Slot, bIsValid);
	}
	else
	{
		MC_ACCESSED_NONE("GetChessTable");
	}
}

void AMCChessObjectBase::GetOpponentKing(AMCKing*& OpponentKing, bool& bIsValid) const
{
	OpponentKing = nullptr;
	bIsValid = false;
	AMCChessTable* Table = GetChessTable();
	if (!Table)
	{
		MC_ACCESSED_NONE("GetChessTable");
		return;
	}
	for (AMCChessObjectBase* Element : Table->AllObjects)
	{
		if (IsValid(Element) && Element->ObjectType == E_ObjectType::King && Element->IsAway != IsAway)
		{
			OpponentKing = Cast<AMCKing>(Element);
			bIsValid = OpponentKing != nullptr;
			return;
		}
	}
}

// ---- EventGraph ----

void AMCChessObjectBase::OnMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	CalcHover(true);
}

void AMCChessObjectBase::OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	CalcHover(false);
}

void AMCChessObjectBase::OnMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0));
	if (!GamePC)
	{
		MC_ACCESSED_NONE("AsBP Game PC");
		return;
	}
	GamePC->Server_ClickedToObject(GamePC, this);
}

void AMCChessObjectBase::Server_IsSelected_Implementation(bool InIsSelected)
{
	Multicast_IsSelected(InIsSelected);
}

void AMCChessObjectBase::Multicast_IsSelected_Implementation(bool InIsSelected)
{
	SetIsSelectedNotify(InIsSelected);
}

void AMCChessObjectBase::Client_Hover_Implementation(bool IsHovering)
{
	HoverGlow(IsHovering);
}

void AMCChessObjectBase::Server_SetIsAway_Implementation(bool InIsAway)
{
	Multi_SetIsAway(InIsAway);
}

void AMCChessObjectBase::Multi_SetIsAway_Implementation(bool InIsAway)
{
	SetIsAwayNotify(InIsAway);
}

void AMCChessObjectBase::ShowPossibilities()
{
	AMCChessTable* Table = GetChessTable();
	for (const FName& Possible : PossibleSlots)
	{
		if (!Table)
		{
			MC_ACCESSED_NONE("GetChessTable");
			return;
		}
		for (AMCTableSlot* Slot : Table->AllSlots)
		{
			if (Slot && Slot->SlotName == Possible && Slot->GetClass()->ImplementsInterface(UMCSlotInterface::StaticClass()))
			{
				IMCSlotInterface::Execute_ShowPossibles(Slot, Slot->SlotName);
			}
		}
	}
}

void AMCChessObjectBase::Server_CalcPossibility_Implementation()
{
	CalcPossibility();
	Multicast_CalcPossibility();
}

void AMCChessObjectBase::Server_DeCalcPossibility_Implementation()
{
	Multicast_DeCalcPossibility();
}

void AMCChessObjectBase::Multicast_CalcPossibility_Implementation()
{
	ShowPossibilities();
}

void AMCChessObjectBase::Multicast_DeCalcPossibility_Implementation()
{
	Server_ClearPossibleSlots();
	if (AMCChessTable* Table = GetChessTable())
	{
		for (AMCTableSlot* Slot : Table->AllSlots)
		{
			if (Slot && Slot->GetClass()->ImplementsInterface(UMCSlotInterface::StaticClass()))
			{
				IMCSlotInterface::Execute_ShowPossibles(Slot, NAME_None);
			}
		}
	}
}

void AMCChessObjectBase::Server_AddPossibleSlot_Implementation(FName Socket)
{
	Multicast_AddPossibleSlot(Socket);
}

void AMCChessObjectBase::Multicast_AddPossibleSlot_Implementation(FName Slot)
{
	PossibleSlots.AddUnique(Slot);
}

void AMCChessObjectBase::Server_ClearPossibleSlots_Implementation()
{
	Multicast_ClearPossibleSlots();
}

void AMCChessObjectBase::Multicast_ClearPossibleSlots_Implementation()
{
	PossibleSlots.Empty();
	PossibleSlots.AddUnique(NAME_None);
}

void AMCChessObjectBase::SetSocketName_Implementation(FName SocketName)
{
	SetSlotNameNotify(SocketName);
}

void AMCChessObjectBase::Server_RotText_Implementation()
{
	if (AMCGame_PC* GamePC = Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Client_RotText(GamePC->GlobalPlayerIndex);
	}
}

void AMCChessObjectBase::Client_RotText_Implementation(int32 A)
{
	if ((A == 1) == IsAway)
	{
		const bool bPick = (A == 1) && IsAway;
		const FRotator WhenAway = bPick ? FRotator(90.f, 180.f, 0.f) : FRotator(90.f, 0.f, 0.f);
		const FRotator WhenHome = bPick ? FRotator(90.f, 0.f, 0.f) : FRotator(90.f, 180.f, 0.f);
		if (TextRender)
		{
			TextRender->SetRelativeRotation(IsAway ? WhenAway : WhenHome);
		}
	}
}

void AMCChessObjectBase::S_SetIsMoved_Implementation(bool InIsMoved)
{
	M_SetIsMoved(InIsMoved);
}

void AMCChessObjectBase::M_SetIsMoved_Implementation(bool InIsMoved)
{
	IsMoved = InIsMoved;
}
