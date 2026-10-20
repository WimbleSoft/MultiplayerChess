#include "Common/MCCommonFunctions.h"
#include "Game/Interactables/MCChessTable.h"
#include "Game/MCGame_PC.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerChess.h"

AMCChessTable* UMCCommonFunctions::FindChessTable(const UObject* WorldContextObject)
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AMCChessTable::StaticClass(), OutActors);
	return OutActors.IsValidIndex(0) ? Cast<AMCChessTable>(OutActors[0]) : nullptr;
}

void UMCCommonFunctions::GetChessTableCommon(const UObject* WorldContextObject, AMCChessTable*& ChessTable)
{
	ChessTable = FindChessTable(WorldContextObject);
}

FText UMCCommonFunctions::MakeSlotNameCommon(int32 Row, int32 Col)
{
	const TArray<FString>& Letters = GetColumnLetters();
	FFormatNamedArguments Args;
	Args.Add(TEXT("LETTER"), FText::FromString(Letters.IsValidIndex(Col) ? Letters[Col] : FString()));
	Args.Add(TEXT("NUMBER"), FText::AsNumber(Row));
	return FText::Format(NSLOCTEXT("BPFL_CommonFunctions", "SlotNameFormat", "{LETTER}{NUMBER}"), Args);
}

void UMCCommonFunctions::ResolveSlotByNameCommon(FName SlotName, int32& Row, int32& Col)
{
	const FString Name = SlotName.ToString();
	Row = FCString::Atoi(*Name.Mid(1, 1));
	Col = GetColumnLetters().Find(Name.Mid(0, 1));
}

AMCGame_PC* UMCCommonFunctions::FindGamePC(const UObject* WorldContextObject)
{
	return Cast<AMCGame_PC>(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}

void UMCCommonFunctions::GetGamePC(const UObject* WorldContextObject, AMCGame_PC*& GamePC)
{
	GamePC = FindGamePC(WorldContextObject);
}

const TArray<FString>& UMCCommonFunctions::GetColumnLetters()
{
	static const TArray<FString> Letters = { TEXT(""), TEXT("A"), TEXT("B"), TEXT("C"), TEXT("D"), TEXT("E"), TEXT("F"), TEXT("G"), TEXT("H"), TEXT("") };
	return Letters;
}

void UMCCommonFunctions::CallFunctionByName(UObject* Target, FName FunctionName)
{
	FMCReflectionCall Call(Target, FunctionName);
	Call.Call();
}

FMCReflectionCall::FMCReflectionCall(UObject* InTarget, FName FunctionName)
	: Target(InTarget)
{
	if (!Target)
	{
		return;
	}
	Function = Target->FindFunction(FunctionName);
	if (!Function)
	{
		UE_LOG(LogMultiplayerChess, Warning, TEXT("%s has no function %s"), *GetNameSafe(Target), *FunctionName.ToString());
		return;
	}
	Buffer = static_cast<uint8*>(FMemory::Malloc(FMath::Max<int32>(Function->ParmsSize, 1), 16));
	FMemory::Memzero(Buffer, FMath::Max<int32>(Function->ParmsSize, 1));
	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->InitializeValue_InContainer(Buffer);
	}
}

FMCReflectionCall::~FMCReflectionCall()
{
	if (Buffer)
	{
		for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->DestroyValue_InContainer(Buffer);
		}
		FMemory::Free(Buffer);
	}
}

void FMCReflectionCall::Call()
{
	if (IsValid())
	{
		Target->ProcessEvent(Function, Buffer);
	}
}
