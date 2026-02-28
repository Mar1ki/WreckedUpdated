#pragma once
#include "pch.h"
#include "SDK/SDK/GameplayTags_classes.hpp"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortAthenaLivingWorldStaticPointProvider
{

    void RegisterToLivingWorldManager(IFortAthenaLivingWorldPointProviderInterface* PointProviderInterface, AFortAthenaLivingWorldStaticPointProvider* PointProvider);
    void LivingWorldManagerEnablePointProvider(AActor* PointProvider);
    void LivingWorldManagerDisablePointProvider(AActor* PointProvider);
    TArray<AFortAthenaLivingWorldStaticPointProvider*> GetPointProvidersFromTagQuery(const FGameplayTag& SearchTag);

    DefineOriginal(void, EnablePointProvider, UObject* Object, FFrame& Params);

    void Init();
}
