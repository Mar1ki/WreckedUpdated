#pragma once
#include "pch.h"
#include "SDK/SDK/GameplayTags_classes.hpp"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortAthenaLivingWorldManager
{

    void Hook();
    void PerformPointProviderSpawn();
    DefineOriginal(char, RegisterActorAndTagDensityReservoir, UFortAthenaLivingWorldManager *a1, UObject a2, float a3, float a4, __int64 a5);
    DefineOriginal(void, HandleGamePhaseStepChanged, UObject* Object, FFrame* Frame);
    DefineOriginal(void, RequestEventGeneration, UObject* Object, FFrame* Frame);
    DefineOriginal(void, TrySpawnEvent, UObject* Object, FFrame* Frame);
}
