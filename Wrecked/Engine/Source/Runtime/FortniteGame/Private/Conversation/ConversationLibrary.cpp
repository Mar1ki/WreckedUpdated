#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Conversation/ConversationLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"


void StartConversation(UObject* Context, FFrame& Params)
{
    std::cout << "StartConversation" << std::endl;
    
}



void ConversationLibrary::Init() {

    UHook* Hook = new UHook();
    
    Hook->Path = "/Script/FortniteGame.FortNonPlayerConversationParticipantComponent.StartConversation";
    Hook->Detour = StartConversation;
    UKismetHookingLibrary::Hook(Hook, Exec);


    Hook->Path = "/Script/FortniteGame.FortNonPlayerConversationParticipantComponent.StartConversation";
    Hook->Detour = StartConversation;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    delete Hook;
}