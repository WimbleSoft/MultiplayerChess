#include "Common/MCGameInstance.h"
#include "Common/MCBasePC.h"
#include "Common/MCCommonFunctions.h"
#include "Common/MCSave.h"
#include "Main/MCMainMenuPC.h"
#include "MultiplayerChess.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "CreateSessionCallbackProxy.h"
#include "DestroySessionCallbackProxy.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/GameUserSettings.h"
#include "JoinSessionCallbackProxy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	// The session proxy classes are MinimalAPI, so their static factory functions can't be linked from this module.
	// Call them through reflection, the same way the Blueprint nodes do.
	template <class TProxy>
	TProxy* MakeSessionProxy(FName FunctionName, TFunctionRef<void(FMCReflectionCall&)> SetParams)
	{
		FMCReflectionCall Call(TProxy::StaticClass()->GetDefaultObject(), FunctionName);
		SetParams(Call);
		Call.Call();
		return Call.Get<TProxy*>(TEXT("ReturnValue"));
	}

	// Same as UFindSessionsCallbackProxy::GetCurrentPlayers / GetMaxPlayers.
	int32 GetCurrentPlayers(const FBlueprintSessionResult& Result)
	{
		return Result.OnlineResult.Session.SessionSettings.NumPublicConnections - Result.OnlineResult.Session.NumOpenPublicConnections;
	}

	int32 GetMaxPlayers(const FBlueprintSessionResult& Result)
	{
		return Result.OnlineResult.Session.SessionSettings.NumPublicConnections;
	}
}

UMCGameInstance::UMCGameInstance()
{
	KickedWidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UI/Main/UI_Kicked.UI_Kicked_C")));
}

void UMCGameInstance::Init()
{
	Super::Init();

	// Event ReceiveInit
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetScreenResolution(Settings->GetDesktopResolution());
	}

	// Event HandleNetworkError / HandleTravelError
	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(this, &UMCGameInstance::HandleNetworkError);
		TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &UMCGameInstance::HandleTravelError);
	}
}

void UMCGameInstance::Shutdown()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	}
	Super::Shutdown();
}

void UMCGameInstance::CreateSession(AMCBasePC* PlayerController)
{
	if (!PlayerController || !PlayerController->R_SaveGame)
	{
		MC_ACCESSED_NONE("PlayerController.R_SaveGame");
		return;
	}
	UMCSave* Save = PlayerController->R_SaveGame;
	UCreateSessionCallbackProxy* Proxy = MakeSessionProxy<UCreateSessionCallbackProxy>(TEXT("CreateSession"), [&](FMCReflectionCall& Call)
	{
		Call.Set<UObject*>(TEXT("WorldContextObject"), this);
		Call.Set<APlayerController*>(TEXT("PlayerController"), PlayerController);
		Call.Set<int32>(TEXT("PublicConnections"), Save->PublicConnections);
		Call.Set<bool>(TEXT("bUseLAN"), Save->UseLan);
		Call.Set<bool>(TEXT("bUseLobbiesIfAvailable"), true);
	});
	if (!Proxy)
	{
		return;
	}
	Proxy->OnSuccess.AddDynamic(this, &UMCGameInstance::OnCreateSessionSuccess);
	Proxy->OnFailure.AddDynamic(this, &UMCGameInstance::OnCreateSessionFailure);
	Proxy->Activate();
}

void UMCGameInstance::OnCreateSessionSuccess()
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);
	UGameplayStatics::OpenLevel(this, TEXT("Lobby"), true, TEXT("listen"));
}

void UMCGameInstance::OnCreateSessionFailure()
{
}

void UMCGameInstance::FindSessions(AMCBasePC* PlayerController, UUserWidget* UI_SessionList)
{
	if (!PlayerController || !PlayerController->R_SaveGame)
	{
		MC_ACCESSED_NONE("PlayerController.R_SaveGame");
		return;
	}
	PendingSessionList = UI_SessionList;
	UMCSave* Save = PlayerController->R_SaveGame;
	UFindSessionsCallbackProxy* Proxy = MakeSessionProxy<UFindSessionsCallbackProxy>(TEXT("FindSessions"), [&](FMCReflectionCall& Call)
	{
		Call.Set<UObject*>(TEXT("WorldContextObject"), this);
		Call.Set<APlayerController*>(TEXT("PlayerController"), PlayerController);
		Call.Set<int32>(TEXT("MaxResults"), Save->MaxServerResults);
		Call.Set<bool>(TEXT("bUseLAN"), Save->UseLan);
		Call.Set<bool>(TEXT("bUseLobbies"), false);
	});
	if (!Proxy)
	{
		return;
	}
	Proxy->OnSuccess.AddDynamic(this, &UMCGameInstance::OnFindSessionsSuccess);
	Proxy->OnFailure.AddDynamic(this, &UMCGameInstance::OnFindSessionsFailure);
	Proxy->Activate();
}

void UMCGameInstance::OnFindSessionsSuccess(const TArray<FBlueprintSessionResult>& Results)
{
	for (const FBlueprintSessionResult& Result : Results)
	{
		if (GetCurrentPlayers(Result) <= GetMaxPlayers(Result))
		{
			FoundSessions.Add(Result);
			UMCCommonFunctions::CallFunctionByName(PendingSessionList.Get(), TEXT("AddNewSessionCard"), TEXT("SessionInfo"), Result);
		}
	}
}

void UMCGameInstance::OnFindSessionsFailure(const TArray<FBlueprintSessionResult>& Results)
{
}

void UMCGameInstance::DestroySession(APlayerController* PlayerController, const FString& Reason)
{
	PendingDestroyController = PlayerController;
	PendingDestroyReason = Reason;
	UDestroySessionCallbackProxy* Proxy = MakeSessionProxy<UDestroySessionCallbackProxy>(TEXT("DestroySession"), [&](FMCReflectionCall& Call)
	{
		Call.Set<UObject*>(TEXT("WorldContextObject"), this);
		Call.Set<APlayerController*>(TEXT("PlayerController"), PlayerController);
	});
	if (!Proxy)
	{
		return;
	}
	Proxy->OnSuccess.AddDynamic(this, &UMCGameInstance::OnDestroySessionSuccess);
	Proxy->OnFailure.AddDynamic(this, &UMCGameInstance::OnDestroySessionFailure);
	Proxy->Activate();
}

void UMCGameInstance::OnDestroySessionSuccess()
{
	UGameplayStatics::OpenLevel(this, TEXT("MainMenu"), true, TEXT(""));
	if (PendingDestroyReason.Len() > 0)
	{
		if (UClass* WidgetClass = KickedWidgetClass.LoadSynchronous())
		{
			if (UUserWidget* Widget = CreateWidget<UUserWidget>(PendingDestroyController.Get(), WidgetClass))
			{
				Widget->AddToViewport(1);
			}
		}
	}
}

void UMCGameInstance::OnDestroySessionFailure()
{
}

void UMCGameInstance::JoinSession(APlayerController* PlayerController, const FBlueprintSessionResult& SearchResult)
{
	if (GetCurrentPlayers(SearchResult) <= GetMaxPlayers(SearchResult))
	{
		JoinedSession = SearchResult;
		UJoinSessionCallbackProxy* Proxy = MakeSessionProxy<UJoinSessionCallbackProxy>(TEXT("JoinSession"), [&](FMCReflectionCall& Call)
		{
			Call.Set<UObject*>(TEXT("WorldContextObject"), this);
			Call.Set<APlayerController*>(TEXT("PlayerController"), PlayerController);
			Call.Set<FBlueprintSessionResult>(TEXT("SearchResult"), SearchResult);
		});
		if (!Proxy)
		{
			return;
		}
		Proxy->OnSuccess.AddDynamic(this, &UMCGameInstance::OnJoinSessionSuccess);
		Proxy->OnFailure.AddDynamic(this, &UMCGameInstance::OnJoinSessionFailure);
		Proxy->Activate();
	}
}

void UMCGameInstance::OnJoinSessionSuccess()
{
}

void UMCGameInstance::OnJoinSessionFailure()
{
	UKismetSystemLibrary::PrintString(this, TEXT("SESSION JOIN FAILED"), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
}

void UMCGameInstance::ServerTravel(const FString& LevelName, APlayerController* SpecificPlayer)
{
	// The Blueprint also had an "if (false) -> OpenLevel(Lobby, listen)" branch, which can never run.
	UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("servertravel ") + LevelName, SpecificPlayer);
}

void UMCGameInstance::ChangeSessionDetails(APlayerController* PlayerController)
{
	// The Blueprint only cast to BP_MainMenuPC and never used the result.
	Cast<AMCMainMenuPC>(PlayerController);
}

void UMCGameInstance::HandleNetworkError(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	const bool bIsServer = NetDriver && NetDriver->IsServer();
	UKismetSystemLibrary::PrintString(this, FString(bIsServer ? TEXT("Server") : TEXT("Client")) + ENetworkFailure::ToString(FailureType), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
}

void UMCGameInstance::HandleTravelError(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UKismetSystemLibrary::PrintString(this, ETravelFailure::ToString(FailureType), true, true, FLinearColor(1.f, 0.f, 0.f, 1.f), 10.f);
}
