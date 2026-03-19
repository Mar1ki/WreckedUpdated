#pragma once
#include "pch.h"

namespace BuildingProp_ConversationCompatible
{
    DefineOriginal(char, ServerOnAttemptInteract, ABuildingGameplayActorConversationCompatible* BuildingActor, const FInteractionType *InteractType);
    void Init();
}