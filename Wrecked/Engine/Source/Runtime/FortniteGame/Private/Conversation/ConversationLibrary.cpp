#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Conversation/ConversationLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void ConversationLibrary::Init() {

    UHook* Hook = new UHook();

    delete Hook;
}