#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Online/FortGameSessionDedicated.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

bool FortGameSessionDedicated::LoadMap(UEngine* Engine, FWorldContext* WorldContext, FURL URL, class UPendingNetGame* Pending, FString* Error) {
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

void FortGameSessionDedicated::Init() {

	UHook* Hook = new UHook();

	Hook->Address = ImageBase + 0x24B61E4;
	Hook->Detour = LoadMap;
	Hook->Original = (void**)&LoadMapOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = 0x788 / 8;
	Hook->Detour = GetGameSessionClass;
	Hook->Class = AFortGameMode::StaticClass();
	//UKismetHookingLibrary::Hook(Hook, VFT);

	delete Hook;
}