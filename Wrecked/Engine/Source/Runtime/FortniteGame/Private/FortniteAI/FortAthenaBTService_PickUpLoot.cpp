#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/FortniteAI/FortAthenaBTService_PickUpLoot.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void FortAthenaBTService_PickUpLoot::AssessLootInteractionInfo(UFortAthenaBTService_PickUpLoot* Service, bool* bOutSkipInteractionExecution, UBlackboardComponent* BBComp, AFortAthenaAIBotController* BotController, AActor* Actor) {

    if (AFortPickup* Pickup = Cast<AFortPickup>(Actor))
    {
        //proper pickup logic needed
        //BotController->PlayerBotPawn->ServerHandlePickup(Pickup, Pickup->DefaultFlyTime, Pickup->PickupLocationData.StartDirection, true);
    }
    else if (ABuildingContainer* Container = Cast<ABuildingContainer>(Actor))
    {
        Container->BP_SpawnLoot(BotController->PlayerBotPawn);
    }
    *bOutSkipInteractionExecution = false;
}

void FortAthenaBTService_PickUpLoot::Init() {
	UHook* Hook = new UHook();

	Hook->Class = UFortAthenaBTService_PickUpLoot::StaticClass();
	Hook->Address = 0x67;
	Hook->Detour = AssessLootInteractionInfo;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	delete Hook;
}