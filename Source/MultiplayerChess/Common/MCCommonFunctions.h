#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/UnrealType.h"
#include "MCCommonFunctions.generated.h"

class AMCChessTable;
class AMCGame_PC;

// Calls a UFUNCTION through reflection, with parameters set by name. Used for engine functions that are
// Blueprint-callable but not exported to other modules (the online session proxies), and for widget functions.
class MULTIPLAYERCHESS_API FMCReflectionCall
{
public:
	FMCReflectionCall(UObject* InTarget, FName FunctionName);
	~FMCReflectionCall();

	bool IsValid() const { return Target && Function; }

	template <typename T>
	void Set(FName ParamName, const T& Value)
	{
		if (!IsValid())
		{
			return;
		}
		FProperty* Param = Function->FindPropertyByName(ParamName);
		if (!Param)
		{
			return;
		}
		if constexpr (std::is_same_v<T, bool>)
		{
			if (FBoolProperty* BoolParam = CastField<FBoolProperty>(Param))
			{
				BoolParam->SetPropertyValue_InContainer(Buffer, Value);
			}
		}
		else if (Param->GetSize() == sizeof(T))
		{
			Param->CopyCompleteValue(Param->ContainerPtrToValuePtr<void>(Buffer), &Value);
		}
	}

	template <typename T>
	T Get(FName ParamName) const
	{
		T Result{};
		if (IsValid())
		{
			FProperty* Param = Function->FindPropertyByName(ParamName);
			if (Param && Param->GetSize() == sizeof(T))
			{
				Param->CopyCompleteValue(&Result, Param->ContainerPtrToValuePtr<void>(Buffer));
			}
		}
		return Result;
	}

	void Call();

private:
	UObject* Target = nullptr;
	UFunction* Function = nullptr;
	uint8* Buffer = nullptr;
};

// Port of BPFL_CommonFunctions.
UCLASS()
class MULTIPLAYERCHESS_API UMCCommonFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common", meta = (WorldContext = "WorldContextObject"))
	static AMCChessTable* GetChessTableCommon(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common")
	static FText MakeSlotNameCommon(int32 Row, int32 Col);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common")
	static void ResolveSlotByNameCommon(FName SlotName, int32& Row, int32& Col);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common", meta = (WorldContext = "WorldContextObject"))
	static AMCGame_PC* GetGamePC(const UObject* WorldContextObject);

	// Column letters as used by the Blueprints: index 0 and 9 are empty strings, A..H are 1..8.
	static const TArray<FString>& GetColumnLetters();

	// Helpers for talking to Blueprint-only assets (UMG widgets) by name, since the widgets stay Blueprints.
	static void CallFunctionByName(UObject* Target, FName FunctionName);

	template <typename T>
	static void CallFunctionByName(UObject* Target, FName FunctionName, FName ParamName, const T& Value)
	{
		FMCReflectionCall Call(Target, FunctionName);
		Call.Set(ParamName, Value);
		Call.Call();
	}

	template <typename T>
	static void SetPropertyByName(UObject* Target, FName PropertyName, const T& Value)
	{
		if (!Target)
		{
			return;
		}
		FProperty* Property = Target->GetClass()->FindPropertyByName(PropertyName);
		if (Property && Property->GetSize() == sizeof(T))
		{
			Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Target), &Value);
		}
	}
};
