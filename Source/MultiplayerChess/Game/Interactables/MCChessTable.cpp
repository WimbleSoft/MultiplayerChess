#include "Game/Interactables/MCChessTable.h"
#include "Common/MCCommonFunctions.h"
#include "Game/Interactables/MCChessObjectBase.h"
#include "Game/Interactables/MCTableSlot.h"
#include "MultiplayerChess.h"
#include "Components/ChildActorComponent.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"

AMCChessTable::AMCChessTable()
{
	bReplicates = true;
	TableSlotClass = TSoftClassPtr<AMCTableSlot>(FSoftObjectPath(TEXT("/Game/Blueprints/Game/Interactables/BP_TableSlot.BP_TableSlot_C")));
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

	bRunningConstructionScript = true;
	SpawnSlots();
	SpawnObjects();
	bRunningConstructionScript = false;
}

USceneComponent* AMCChessTable::FindSceneComponent(FName ComponentName) const
{
	TArray<USceneComponent*> Components;
	GetComponents(Components);
	for (USceneComponent* Component : Components)
	{
		if (Component->GetFName() == ComponentName)
		{
			return Component;
		}
	}
	return nullptr;
}

UChildActorComponent* AMCChessTable::AddChildActorComponent(TSubclassOf<AActor> ChildClass, FName ComponentName)
{
	const FName UniqueName = MakeUniqueObjectName(this, UChildActorComponent::StaticClass(), ComponentName);
	UChildActorComponent* Component = NewObject<UChildActorComponent>(this, UniqueName);
	Component->CreationMethod = bRunningConstructionScript ? EComponentCreationMethod::UserConstructionScript : EComponentCreationMethod::Instance;
	Component->SetNetAddressable();
	if (USceneComponent* Root = GetRootComponent())
	{
		Component->SetupAttachment(Root);
	}
	if (!bRunningConstructionScript)
	{
		AddInstanceComponent(Component);
	}
	Component->RegisterComponent();
	Component->SetChildActorClass(ChildClass);
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
	UClass* SlotClass = TableSlotClass.LoadSynchronous();
	USceneComponent* SM_Table = FindSceneComponent(TEXT("SM_Table"));

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
			UChildActorComponent* Component = AddChildActorComponent(SlotClass, FName(*FString::Printf(TEXT("SlotComponent_%s"), *SlotName.ToString())));
			AMCTableSlot* Slot = Cast<AMCTableSlot>(Component->GetChildActor());
			if (!Slot)
			{
				// Clients don't spawn replicated child actors; they receive them from the server.
				continue;
			}
			Slot->HomeColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
			// The Blueprint left this pin at its default, which is a fully transparent black.
			Slot->AwayColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
			Slot->Server_SetIsAway(!(bInnerIsA ^ bOuterIsA));
			if (SM_Table)
			{
				Slot->AttachToComponent(SM_Table, FAttachmentTransformRules::KeepRelativeTransform, SlotName);
			}
			else
			{
				MC_ACCESSED_NONE("SM_Table");
			}
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
	UChildActorComponent* Component = AddChildActorComponent(S_ObjectMap.Class, FName(*FString::Printf(TEXT("ObjectComponent_%s"), *S_ObjectMap.SlotName.ToString())));
	AMCChessObjectBase* Object = Cast<AMCChessObjectBase>(Component->GetChildActor());
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
