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

string UBlackboardComponent::GetKeyName(unsigned __int8 Key) {
	if (this && this->IsA<UBlackboardComponent>()) {
		auto BlackBoard = this->BlackboardAsset;
		if (BlackboardAsset) {
			if (Key != 0xFF) {
                auto FirstKey = *(unsigned __int8*)(__int64(BlackBoard) + 0x4C);
                if (BlackboardAsset->Keys.IsValidIndex(Key-FirstKey)) {
                    auto KeyData = BlackBoard->Keys[Key - FirstKey];
                    return KeyData.EntryName.ToString().c_str();
                }
			}
		}
		
	}
    return "";
}

void FortAthenaBTService_PickUpLoot::ManageWeapon(UFortAthenaBTService_ManageWeapon* ManageWeapon, UBehaviorTreeComponent* OwnerComp) {
    static EExecutionStatus(*GetExecutionStatus)(UBlackboardComponent * BBComp, unsigned __int8 BlackboardKey) = decltype(GetExecutionStatus)(ImageBase + 0xA8C32E0);
    static EExecutionStatus(*SetExecutionStatus)(UBlackboardComponent * BBComp, unsigned __int8 BlackboardKey, EExecutionStatus Status) = decltype(SetExecutionStatus)(ImageBase + 0xA8CDCE4);
    //SetExecutionStatus(OwnerComp->BlackboardComp, *(unsigned __int8*)(__int64(ManageWeapon) + 0x98), EExecutionStatus::ExecutionAllowed);
    auto ReloadingStatus = GetExecutionStatus(OwnerComp->BlackboardComp, *(unsigned __int8*)(__int64(ManageWeapon) + 0x9E));
    auto TriggerMeleeKeyStatus = GetExecutionStatus(OwnerComp->BlackboardComp, *(unsigned __int8*)(__int64(ManageWeapon) + 0x9A));
    auto WeaponFireStatus = GetExecutionStatus(OwnerComp->BlackboardComp, *(unsigned __int8*)(__int64(ManageWeapon) + 0x98));
    auto WeaponTriggerThrowableStatus = GetExecutionStatus(OwnerComp->BlackboardComp, *(unsigned __int8*)(__int64(ManageWeapon) + 0x9C));
    auto TacticalSprintExecutionStatus = GetExecutionStatus(OwnerComp->BlackboardComp, *(unsigned __int8*)(__int64(ManageWeapon) + 0xA6));
    //UBlackboardComponent
    //UE_LOG(LogAI, Log, "ManageWeapon: ReloadingStatus(%i) TriggerMeleeKeyStatus(%i) WeaponFireStatus(%i) WeaponTriggerThrowableStatus(%i) TacticalSprintExecutionStatus(%i)", ReloadingStatus, TriggerMeleeKeyStatus, WeaponFireStatus, WeaponTriggerThrowableStatus, TacticalSprintExecutionStatus);
    ManageWeaponOG(ManageWeapon, OwnerComp);
}

void FortAthenaBTService_PickUpLoot::Init() {
	UHook* Hook = new UHook();

	Hook->Class = UFortAthenaBTService_PickUpLoot::StaticClass();
	Hook->Address = 0x67;
	Hook->Detour = AssessLootInteractionInfo;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = ImageBase + 0xAA1D3B4;
    Hook->Detour = ManageWeapon;
    Hook->Original = (void**)&ManageWeaponOG;
    UKismetHookingLibrary::Hook(Hook, Address);

    //Hook->Address = ImageBase + 0xA8CDCE4;
    //Hook->Detour = SetExecutionStatus;
    //Hook->Original = (void**)&SetExecutionStatusOG;
    //UKismetHookingLibrary::Hook(Hook, Address);

	delete Hook;
}