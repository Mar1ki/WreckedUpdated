#pragma once
#include "pch.h"

namespace FortPoiVolume
{
    DefineOriginal(void, PostInitializeComponents, AFortPoiVolume* Volume);
    DefineOriginal(void, SendComplexCustomStatEvent,
        UFortQuestManager* QuestManager,
        UObject* TargetObj,
        FGameplayTagContainer& SourceTags,
        FGameplayTagContainer& TargetTags,
        int32 Count);
    DefineOriginal(void, ProcessLocationTagDeltaForController, AFortPoiManager* Manager, AFortPlayerControllerGameplay* Controller, FGameplayTagContainer* TagsToRemove, FGameplayTagContainer* TagsToAdd);
    void Setup();
}
