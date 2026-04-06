#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Online/FortGameSessionDedicated.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

bool FortGameSessionDedicated::LoadMap(UEngine* Engine, FWorldContext* WorldContext, FURL* URL, UPendingNetGame* Pending, FString* Error) {
	auto res = LoadMapOG(Engine, WorldContext, URL, Pending, Error);
	if (res)
	{
		//We would do listening checks here
	}
	return res;
}

TSubclassOf<AGameSession>* FortGameSessionDedicated::GetGameSessionClass(AFortGameMode* GameMode, TSubclassOf<AGameSession>* result) {
	result->ClassPtr = AFortGameSessionDedicatedAthena::StaticClass();
	return result;
}

void* FortGameSessionDedicated::GetServicePermissionsByName(void* Config, FString* Name) {
	logtofile(Name->ToStr().c_str());
	static FServicePermissionsMcp* ServicePerms = nullptr;
	if (!ServicePerms)
	{
		ServicePerms = new FServicePermissionsMcp{ .Id = L"ec684b8c687f479fadea3cb2ad83f5c6" };
	}

	return ServicePerms;
}

UWorld* FortGameSessionDedicated::GetWorldFromSmth(__int64 Smth) {
	return GetWorld();
}

const wchar_t* FortGameSessionDedicated::CommandLineGet()
{
	auto Command = std::wstring(CommandLineGetOG()) + L" -AllowAllPlaylistsInShipping";
	return Command.c_str();
}

char FortGameSessionDedicated::GetSquadIdForCurrentPlayer(AFortGameSessionDedicatedAthena* GameSession, FUniqueNetIdRepl UniqueId)
{
	for (auto Player : GetGameMode()->AlivePlayers)
	{
		if (Player && Player->PlayerState)
		{
			auto PlayerState = (AFortPlayerStateAthena*)Player->PlayerState;
			if (PlayerState->AreUniqueIDsIdentical(PlayerState->UniqueID, UniqueId))
			{
				return PlayerState->TeamIndex - 3;
			}
		}
	}
	return 0;
}

void FortGameSessionDedicated::Init() {

	UHook* Hook = new UHook();

	Hook->Address = ImageBase + 0x24B61E4;
	Hook->Detour = LoadMap;
	Hook->Original = (void**)&LoadMapOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = ImageBase + 0x2161D28;
	Hook->Detour = GetServicePermissionsByName;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	Hook->Address = 0x788 / 8;
	Hook->Detour = GetGameSessionClass;
	Hook->Class = AFortGameMode::StaticClass();
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Address = ImageBase + 0x145D240;
	Hook->Detour = CommandLineGet;
	Hook->Original = (void**)&CommandLineGetOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = ImageBase + 0x9A10B1C;
	Hook->Detour = GetWorldFromSmth;
	Hook->Original = (void**)&GetWorldFromSmthOG;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	Hook->Address = ImageBase + 0x9E6CDF0;
	Hook->Detour = GetSquadIdForCurrentPlayer;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	delete Hook;
}