#include "Game/MCGame_PS.h"
#include "Game/MCGame_PC.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AMCGame_PS::AMCGame_PS()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMCGame_PS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCGame_PS, PlayerInfo);
}

AMCGame_PC* AMCGame_PS::GetPC() const
{
	return Cast<AMCGame_PC>(GetPlayerController());
}

void AMCGame_PS::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		const AMCGame_PC* PC = GetPC();
		const UEnum* RoleEnum = StaticEnum<E_PlayerRole>();
		FFormatNamedArguments Args;
		Args.Add(TEXT("PlayerName"), FText::FromString(FString(TEXT("Player_")) + FString::FromInt(PC ? PC->GlobalPlayerIndex : 0)));
		Args.Add(TEXT("Points"), FText::AsNumber(PlayerInfo.PlayerPoints));
		Args.Add(TEXT("PlayerRole"), FText::FromString(RoleEnum->GetNameStringByValue(static_cast<int64>(PlayerInfo.PlayerRole))));
		UKismetSystemLibrary::PrintText(this, FText::Format(NSLOCTEXT("BP_Game_PS", "TickInfo", "PlayerName: {PlayerName} Points: {Points} Role: {PlayerRole}"), Args), true, false, FLinearColor(0.f, 0.66f, 1.f, 1.f), 0.f);
	}
}

void AMCGame_PS::Server_SetPlayerInfo_Implementation(const FS_PlayerInfo& InPlayerInfo)
{
	Multi_SetPlayerInfo(InPlayerInfo);
}

void AMCGame_PS::Multi_SetPlayerInfo_Implementation(const FS_PlayerInfo& InPlayerInfo)
{
	PlayerInfo = InPlayerInfo;
}
