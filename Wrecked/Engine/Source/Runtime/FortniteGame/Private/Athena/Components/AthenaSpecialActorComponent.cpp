#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/AthenaSpecialActorComponent.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"



void AthenaSpecialActorComponent::Activate(UAthenaSpecialActorComponent* Component, bool bReset)
{
    
    UE_LOG(LogTemp, Log, "UAthenaSpecialActorComponent::Active Called");

    ActivateOG(Component, bReset);
}


void AthenaSpecialActorComponent::Init() {
    UHook* Hook = new UHook();

    Hook->Address = 0x0; // ADJUST !!!!!!!!!!!!!!
    Hook->Class = UAthenaSpecialActorComponent::StaticClass();
    Hook->Detour = Activate;
    Hook->Original = (void**)&ActivateOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT); 

    delete Hook;
}