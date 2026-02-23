#pragma once
#include "pch.h"

namespace FortGameSessionDedicated {
	DefineOriginal(bool, LoadMap, UEngine* Engine, FWorldContext* WorldContext, FURL URL, class UPendingNetGame* Pending, FString* Error);
	TSubclassOf<AGameSession>* GetGameSessionClass(AFortGameMode* GameMode, TSubclassOf<AGameSession>* result);
	void Init();
}