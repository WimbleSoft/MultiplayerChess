#include "Game/Interactables/MCChessTable.h"
#include "Common/MCCommonFunctions.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "Game/Interactables/MCTableSlot.h"
#include "MultiplayerChess.h"
#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"

namespace
{
	struct FMCBoardLabel
	{
		const TCHAR* Name;
		const TCHAR* Text;
		bool bAway;
		FVector Location;
		FRotator Rotation;
	};

	// Coordinate labels of BP_ChessTable, with the transforms they had in the Blueprint.
	const FMCBoardLabel BoardLabels[] = {
		{ TEXT("A_Away"), TEXT("A"), true, FVector(23.405241f, -18.497940f, 3.200000f), FRotator(90.000000f, 539.994141f, 899.991699f) },
		{ TEXT("B_Away"), TEXT("B"), true, FVector(23.405241f, -13.195345f, 3.200000f), FRotator(90.000000f, 379.470856f, 739.470337f) },
		{ TEXT("C_Away"), TEXT("C"), true, FVector(23.417459f, -7.941622f, 3.200000f), FRotator(90.000000f, 179.982513f, 899.981384f) },
		{ TEXT("D_Away"), TEXT("D"), true, FVector(23.417459f, -2.614585f, 3.200000f), FRotator(90.000000f, 379.470093f, 1099.468628f) },
		{ TEXT("E_Away"), TEXT("E"), true, FVector(23.417459f, 2.614702f, 3.200000f), FRotator(90.000000f, 177.750504f, 177.749023f) },
		{ TEXT("F_Away"), TEXT("F"), true, FVector(23.417459f, 7.929510f, 3.200000f), FRotator(90.000000f, -373.263245f, -13.264889f) },
		{ TEXT("G_Away"), TEXT("G"), true, FVector(23.417459f, 13.189342f, 3.200000f), FRotator(90.000000f, 179.987762f, 179.985779f) },
		{ TEXT("H_Away"), TEXT("H"), true, FVector(23.417459f, 18.498047f, 3.200000f), FRotator(90.000000f, 160.529144f, 160.527008f) },
		{ TEXT("1_Away"), TEXT("1"), true, FVector(-18.680574f, 23.119345f, 3.199999f), FRotator(90.000000f, 1429.973511f, 1069.972046f) },
		{ TEXT("2_Away"), TEXT("2"), true, FVector(-13.377982f, 23.119263f, 3.200000f), FRotator(90.000000f, 1066.737061f, 346.736023f) },
		{ TEXT("3_Away"), TEXT("3"), true, FVector(-8.124262f, 23.106964f, 3.200000f), FRotator(90.000000f, 714.948486f, 354.947845f) },
		{ TEXT("4_Away"), TEXT("4"), true, FVector(-2.797232f, 23.106880f, 3.200001f), FRotator(90.000000f, 900.000000f, 899.999756f) },
		{ TEXT("5_Away"), TEXT("5"), true, FVector(2.432050f, 23.106802f, 3.200000f), FRotator(90.000000f, 179.859344f, 179.858917f) },
		{ TEXT("6_Away"), TEXT("6"), true, FVector(7.746866f, 23.106718f, 3.200002f), FRotator(90.000000f, 899.999634f, 539.999512f) },
		{ TEXT("7_Away"), TEXT("7"), true, FVector(13.006695f, 23.106638f, 3.200002f), FRotator(90.000000f, 900.000000f, 539.999512f) },
		{ TEXT("8_Away"), TEXT("8"), true, FVector(18.315397f, 23.106554f, 3.200003f), FRotator(90.000000f, 540.000000f, 539.999084f) },
		{ TEXT("A_Home"), TEXT("A"), false, FVector(-23.522430f, -18.497995f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("B_Home"), TEXT("B"), false, FVector(-23.522430f, -13.195399f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("C_Home"), TEXT("C"), false, FVector(-23.510212f, -7.941677f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("D_Home"), TEXT("D"), false, FVector(-23.510212f, -2.614646f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("E_Home"), TEXT("E"), false, FVector(-23.510212f, 2.614642f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("F_Home"), TEXT("F"), false, FVector(-23.510212f, 7.929455f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("G_Home"), TEXT("G"), false, FVector(-23.510212f, 13.189287f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("H_Home"), TEXT("H"), false, FVector(-23.510212f, 18.497992f, 3.200000f), FRotator(90.000000f, 0.000000f, 179.999939f) },
		{ TEXT("1_Home"), TEXT("1"), false, FVector(-18.680573f, -23.116800f, 3.200000f), FRotator(90.000000f, 1279.471191f, 379.471191f) },
		{ TEXT("2_Home"), TEXT("2"), false, FVector(-13.377978f, -23.116882f, 3.200001f), FRotator(90.000000f, 360.140686f, -179.859497f) },
		{ TEXT("3_Home"), TEXT("3"), false, FVector(-8.124258f, -23.129181f, 3.200001f), FRotator(90.000000f, 1080.000000f, 900.000122f) },
		{ TEXT("4_Home"), TEXT("4"), false, FVector(-2.797226f, -23.129265f, 3.200001f), FRotator(90.000000f, 720.000671f, 540.001038f) },
		{ TEXT("5_Home"), TEXT("5"), false, FVector(2.432056f, -23.129343f, 3.200001f), FRotator(90.000000f, -359.996063f, -539.995850f) },
		{ TEXT("6_Home"), TEXT("6"), false, FVector(7.746872f, -23.129427f, 3.200003f), FRotator(90.000000f, 540.000000f, 360.000427f) },
		{ TEXT("7_Home"), TEXT("7"), false, FVector(13.006699f, -23.129507f, 3.200003f), FRotator(90.000000f, 360.019897f, 180.020004f) },
		{ TEXT("8_Home"), TEXT("8"), false, FVector(18.315399f, -23.129591f, 3.200004f), FRotator(90.000000f, 1080.003662f, 540.003723f) },
	};
}

AMCChessTable::AMCChessTable()
{
	bReplicates = true;

	SM_Table = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SM_Table"));
	RootComponent = SM_Table;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> TableMesh(TEXT("/Game/Geometry/Meshes/Table/ChessTable.ChessTable"));
	SM_Table->SetSkeletalMeshAsset(TableMesh.Object);

	Texts = CreateDefaultSubobject<USceneComponent>(TEXT("Texts"));
	Texts->SetupAttachment(SM_Table);

	for (const FMCBoardLabel& Label : BoardLabels)
	{
		UTextRenderComponent* TextComponent = CreateDefaultSubobject<UTextRenderComponent>(Label.Name);
		TextComponent->SetupAttachment(Texts);
		TextComponent->SetRelativeLocation(Label.Location);
		TextComponent->SetRelativeRotation(Label.Rotation);
		TextComponent->SetRelativeScale3D(FVector(0.2f));
		TextComponent->SetText(FText::FromString(Label.Text));
		TextComponent->SetWorldSize(26.f);
		TextComponent->SetHorizontalAlignment(EHTA_Center);
		TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
		TextComponent->SetTextRenderColor(Label.bAway ? FColor(0, 0, 0, 255) : FColor(255, 255, 255, 255));
		TextComponent->SetCastShadow(false);
		Labels.Add(TextComponent);
	}

	TableSlotClass = AMCTableSlot::StaticClass();
}

void AMCChessTable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCChessTable, AllSlots);
	DOREPLIFETIME(AMCChessTable, AllObjects);
}

void AMCChessTable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// The Blueprint's variables start from their defaults on every construction script run.
	AllSlots.Reset();
	AllObjects.Reset();

	SpawnSlots();
	SpawnObjects();
}

void AMCChessTable::BeginPlay()
{
	Super::BeginPlay();

	// A Blueprint's construction script runs again for the play world, a native OnConstruction does not. In PIE the
	// board's child actors are spawned again from scratch at the board's origin, so the server repeats the setup
	// (slot names, colours, socket attachment, piece placement) on the existing components.
	if (!HasAuthority())
	{
		return;
	}
	ReusableChildActorComponents.Reset();
	for (UActorComponent* Component : BlueprintCreatedComponents)
	{
		if (UChildActorComponent* ChildActorComponent = Cast<UChildActorComponent>(Component))
		{
			ReusableChildActorComponents.Add(ChildActorComponent);
		}
	}
	AllSlots.Reset();
	AllObjects.Reset();
	SpawnSlots();
	SpawnObjects();
	ReusableChildActorComponents.Reset();
}

UChildActorComponent* AMCChessTable::AddChildActorComponent(TSubclassOf<AActor> ChildClass)
{
	if (ReusableChildActorComponents.Num() > 0)
	{
		UChildActorComponent* Existing = ReusableChildActorComponents[0];
		ReusableChildActorComponents.RemoveAt(0);
		if (Existing)
		{
			if (Existing->GetChildActorClass() != ChildClass)
			{
				Existing->SetChildActorClass(ChildClass);
			}
			return Existing;
		}
	}

	// Same path as the Blueprint "Add Child Actor Component" node: the component is tracked as created by the
	// construction script, so it is saved with the level and rebuilt (not duplicated) when construction reruns.
	UChildActorComponent* Component = Cast<UChildActorComponent>(AddComponentByClass(UChildActorComponent::StaticClass(), true, FTransform::Identity, true));
	if (!Component)
	{
		return nullptr;
	}
	Component->SetChildActorClass(ChildClass);
	Component->SetupAttachment(GetRootComponent());
	FinishAddComponent(Component, true, FTransform::Identity);
	return Component;
}

void AMCChessTable::ClearSlotsObjects()
{
	for (AMCTableSlot* Slot : AllSlots)
	{
		if (Slot)
		{
			Slot->Destroy();
		}
	}
	for (AMCChessObjectBase* Object : AllObjects)
	{
		if (Object)
		{
			Object->Destroy();
		}
	}
	AllSlots.Empty();
	AllObjects.Empty();
}

AMCTableSlot* AMCChessTable::GetTableSlotBySocketName(FName SocketName) const
{
	AMCTableSlot* Slot = nullptr;
	for (AMCTableSlot* Element : AllSlots)
	{
		if (!Element)
		{
			MC_ACCESSED_NONE("Array Element");
			continue;
		}
		if (SocketName == Element->SlotName)
		{
			Slot = Element;
			break;
		}
	}
	return Slot;
}

void AMCChessTable::SpawnObjects()
{
	if (!DT_ObjectMapper)
	{
		// BP_ChessTable sets DT_ObjectMap as the class default; fall back to it if the property was not carried over.
		DT_ObjectMapper = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprints/Game/DT_ObjectMap.DT_ObjectMap"));
	}
	if (!DT_ObjectMapper)
	{
		return;
	}
	for (const FName& RowName : DT_ObjectMapper->GetRowNames())
	{
		if (const FS_ObjectMap* Row = DT_ObjectMapper->FindRow<FS_ObjectMap>(RowName, TEXT("SpawnObjects")))
		{
			SpawnObject(*Row);
		}
	}
}

void AMCChessTable::SpawnSlots()
{
	UClass* SlotClass = TableSlotClass;

	// FlipFlop macros: their state is a function local, it starts at "A" on every call and toggles on every pass.
	bool bOuterIsA = false;
	bool bInnerIsA = false;
	for (int32 ColIndex = 1; ColIndex <= 8; ++ColIndex)
	{
		bOuterIsA = !bOuterIsA;
		for (int32 RowIndex = 1; RowIndex <= 8; ++RowIndex)
		{
			bInnerIsA = !bInnerIsA;

			const FName SlotName = FName(*UMCCommonFunctions::MakeSlotNameCommon(RowIndex, ColIndex).ToString());
			UChildActorComponent* Component = AddChildActorComponent(SlotClass);
			AMCTableSlot* Slot = Component ? Cast<AMCTableSlot>(Component->GetChildActor()) : nullptr;
			if (!Slot)
			{
				// Clients don't spawn replicated child actors; they receive them from the server.
				continue;
			}
			Slot->HomeColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
			// The Blueprint left this pin at its default, which is a fully transparent black.
			Slot->AwayColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
			Slot->Server_SetIsAway(!(bInnerIsA ^ bOuterIsA));
			Slot->AttachToComponent(SM_Table, FAttachmentTransformRules::KeepRelativeTransform, SlotName);
			Slot->SetSlotName(SlotName);
			AllSlots.Add(Slot);
		}
	}
}

void AMCChessTable::GetObjectBySlotName(FName SlotName, AMCChessObjectBase*& ChessObject, bool& bIsValid) const
{
	for (AMCChessObjectBase* Element : AllObjects)
	{
		if (!Element)
		{
			MC_ACCESSED_NONE("Array Element");
			continue;
		}
		if (SlotName == Element->SlotName)
		{
			ChessObject = Element;
			bIsValid = true;
			return;
		}
	}
	ChessObject = nullptr;
	bIsValid = false;
}

void AMCChessTable::GetSlotByName(FName QuerySlotName, AMCTableSlot*& Slot, bool& bIsValid) const
{
	for (AMCTableSlot* Element : AllSlots)
	{
		if (!Element)
		{
			MC_ACCESSED_NONE("Array Element");
			continue;
		}
		if (QuerySlotName == Element->SlotName)
		{
			Slot = Element;
			bIsValid = true;
			return;
		}
	}
	Slot = nullptr;
	bIsValid = false;
}

void AMCChessTable::SpawnObject(const FS_ObjectMap& S_ObjectMap)
{
	UChildActorComponent* Component = AddChildActorComponent(S_ObjectMap.Class);
	AMCChessObjectBase* Object = Component ? Cast<AMCChessObjectBase>(Component->GetChildActor()) : nullptr;
	if (!Object)
	{
		return;
	}
	Object->SetSlotNameNotify(S_ObjectMap.SlotName);
	Object->Server_SetIsAway(S_ObjectMap.IsAway);
	AllObjects.Add(Object);
	for (AMCTableSlot* Slot : AllSlots)
	{
		if (!Slot)
		{
			MC_ACCESSED_NONE("Array Element");
			continue;
		}
		if (S_ObjectMap.SlotName == Slot->SlotName)
		{
			Object->AttachToActor(Slot, FAttachmentTransformRules::KeepRelativeTransform);
			Object->SetActorRelativeLocation(FVector::ZeroVector);
			Object->SetActorRelativeRotation(FRotator::ZeroRotator);
			Slot->Server_SetAttachedObject(Object);
		}
	}
}

TMap<FName, bool> AMCChessTable::GetAllSlotsOccupationInfo() const
{
	TMap<FName, bool> TempTableSlots;
	for (AMCTableSlot* Element : AllSlots)
	{
		if (!Element)
		{
			MC_ACCESSED_NONE("Array Element");
			continue;
		}
		TempTableSlots.Add(Element->SlotName, IsValid(Element->AttachedObject));
	}
	return TempTableSlots;
}
