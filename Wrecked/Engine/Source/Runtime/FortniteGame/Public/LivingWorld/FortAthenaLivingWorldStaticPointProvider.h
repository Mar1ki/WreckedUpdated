#pragma once
#include "pch.h"
#include "SDK/SDK/GameplayTags_classes.hpp"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

namespace FortAthenaLivingWorldStaticPointProvider
{

    void RegisterToLivingWorldManager(IFortAthenaLivingWorldPointProviderInterface* PointProviderInterface, AFortAthenaLivingWorldStaticPointProvider* PointProvider);

    void Init();
}
