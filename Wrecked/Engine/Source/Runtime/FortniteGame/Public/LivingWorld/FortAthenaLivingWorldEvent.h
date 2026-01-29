#pragma once
#include "pch.h"
#include "SDK/SDK/GameplayTags_classes.hpp"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

namespace FortAthenaLivingWorldEvent
{

    TArray<FFortAthenaLivingWorldEvent*> GetAllWorldEvents(UFortAthenaLivingWorldManager* LivingWorldManager);
    TArray<AFortAthenaPatrolPath*> GetPatrolPathsFromTagQuery(const FGameplayTag& SearchTag);
    void TrySpawnEvents();
    void Hook();
}
