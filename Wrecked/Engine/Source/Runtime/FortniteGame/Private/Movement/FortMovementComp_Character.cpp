#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Movement/FortMovementComp_Character.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Movement/FortMovementMode_BaseExtLogic.h"
#include "SDK/SDK/NitroGameplayRuntime_classes.hpp"

FMMERuntimeContext CreateRuntimeContext(UFortMovementMode_BaseExtRuntimeData* RuntimeData) {
    static void (*ConstructRuntimeContext)(FMMERuntimeContext*, UFortMovementMode_BaseExtRuntimeData*) = decltype(ConstructRuntimeContext)(ImageBase + 0x1C105BC);
    FMMERuntimeContext Temp;
    ConstructRuntimeContext(&Temp, RuntimeData);
    return Temp;
}

void RemoveRuntimeData(TArray<UFortMovementMode_BaseExtRuntimeData*>& Array, UFortMovementMode_BaseExtRuntimeData* RuntimeData) {
    if (!RuntimeData)
        return;

    for (auto i = 0; i < Array.Num(); i++) 
    {
        if (Array.IsValidIndex(i) && Array[i]->LogicID.MovementMode_ExtCRC_ID == RuntimeData->LogicID.MovementMode_ExtCRC_ID)
        {
            Array.Remove(i);
            break;
        }
    }
}

int FindRuntimeDataIndex(TArray<UFortMovementMode_BaseExtRuntimeData*>& Array, UFortMovementMode_BaseExtRuntimeData* RuntimeData) {
    if (!RuntimeData)
        return -1;

    for (auto i = 0; i < Array.Num(); i++) {
        if (Array.IsValidIndex(i) && Array[i]->LogicID.MovementMode_ExtCRC_ID == RuntimeData->LogicID.MovementMode_ExtCRC_ID) {
            return i;
        }
    }
    return -1;
}

void CoolFuncion(UFortMovementComp_Character* Comp) {//139B788
    static EFortMovementModeExt_UpdateResult(*UpdateBeforeCharacterMovement)(UFortMovementMode_BaseExtLogic*, FMMERuntimeContext) = decltype(UpdateBeforeCharacterMovement)(ImageBase + 0x9DADFB4);
    static void (*OnExstensionDeactivated)(UFortMovementMode_BaseExtLogic*, FMMERuntimeContext) = decltype(OnExstensionDeactivated)(ImageBase + 0x27EEAB8);
    static void (*OnExstensionActivated)(UFortMovementMode_BaseExtLogic*, FMMERuntimeContext) = decltype(OnExstensionActivated)(ImageBase + 0x27EE7C8);
    static void (*StrangeShitNoImpl)(UFortMovementMode_BaseExtLogic*, FMMERuntimeContext) = decltype(StrangeShitNoImpl)(ImageBase + 0x32E92C4);
    static void (*AnotherUselesHandler)(UFortMovementComp_Character*, UFortMovementMode_BaseExtRuntimeData**) = decltype(AnotherUselesHandler)(ImageBase + 0x9D86318);
    static void (*MagicPawnFunction1)(AFortPawn*, UFortMovementMode_BaseExtRuntimeData*) = decltype(MagicPawnFunction1)(ImageBase + 0xA0F70CC);
    static __int64 (*UpdateActiveLayer)(UFortMovementMode_BaseExtLogic*, FMMERuntimeContext) = decltype(UpdateActiveLayer)(ImageBase + 0x9DADE80);

    auto Pawn = Cast<AFortPawn>(Comp->CharacterOwner);
    if (!Pawn) return;

    TArray<UFortMovementMode_BaseExtRuntimeData*> ToRemove{};
    UFortMovementMode_BaseExtRuntimeData* ActiveRuntimeData = nullptr;
    UFortMovementMode_BaseExtLogic* ActiveExtension = nullptr;

    for (auto RuntimeData : Comp->PushedMovementModeExtensionRuntimeData) {
        auto MME = FortMovementMode_BaseExtLogic::GetMovementModeExtById(Pawn, RuntimeData->LogicID);
        if (!MME) continue;

        auto RuntimeContext = CreateRuntimeContext(RuntimeData);
        
        auto UpdateResult = UpdateBeforeCharacterMovement(MME, RuntimeContext);

        UE_LOG(LogMME, Log, "Handling RuntimeData(%s): UpdateResult: %i", RuntimeData->GetName().c_str(), int(UpdateResult));

        if (false) {
            //needs impl
        }

        if (UpdateResult == EFortMovementModeExt_UpdateResult::Ended) {
            ToRemove.Add(RuntimeData);

            if (FindRuntimeDataIndex(Comp->ActiveLayeredMovementModeRuntimeData, RuntimeData) >= 0) {
                OnExstensionDeactivated(MME, RuntimeContext);
                void (*SomeStrangeFunc)(UFortMovementMode_BaseExtLogic*) = decltype(SomeStrangeFunc)(MME->VTable[0x2b8/8]);
                SomeStrangeFunc(MME);
                RemoveRuntimeData(Comp->ActiveLayeredMovementModeRuntimeData, RuntimeData);
            }
        }

        else if (UpdateResult == EFortMovementModeExt_UpdateResult::Active) {
            ActiveRuntimeData = RuntimeData;
            ActiveExtension = MME;
        }

        else if (UpdateResult == EFortMovementModeExt_UpdateResult::Active_Layered && FindRuntimeDataIndex(Comp->ActiveLayeredMovementModeRuntimeData, RuntimeData) == -1) {
            Comp->ActiveLayeredMovementModeRuntimeData.Add(RuntimeData);
            OnExstensionActivated(MME, RuntimeContext);
        }
    }

    for (auto RuntimeData : ToRemove) {
        auto MME = FortMovementMode_BaseExtLogic::GetMovementModeExtById(Pawn, RuntimeData->LogicID);
        
        if (MME) {
            auto RuntimeContext = CreateRuntimeContext(RuntimeData);
            StrangeShitNoImpl(MME, RuntimeContext);

        }
        AnotherUselesHandler(Comp, &RuntimeData);
    }
    bool flag = false;
    if (Comp->ActiveMovementModeExtensionRuntimeData)
        flag = ActiveRuntimeData == Comp->ActiveMovementModeExtensionRuntimeData;
    else
        flag = Comp->ActiveMovementModeExtensionRuntimeData == nullptr;

    if (!flag) {
        if (Comp->ActiveMovementModeExtension) {
            auto RuntimeContext = CreateRuntimeContext(Comp->ActiveMovementModeExtensionRuntimeData);
            OnExstensionDeactivated(Comp->ActiveMovementModeExtension, RuntimeContext);
            void (*SomeStrangeFunc)(UFortMovementMode_BaseExtLogic*) = decltype(SomeStrangeFunc)(Comp->ActiveMovementModeExtension->VTable[0x2b8 / 8]);
            SomeStrangeFunc(Comp->ActiveMovementModeExtension);
        }
        Comp->ActiveMovementModeExtension = ActiveExtension;
        Comp->ActiveMovementModeExtensionRuntimeData = ActiveRuntimeData;
        MagicPawnFunction1(Pawn, Comp->ActiveMovementModeExtensionRuntimeData);

        if (Comp->ActiveMovementModeExtension) {
            auto RuntimeContext = CreateRuntimeContext(Comp->ActiveMovementModeExtensionRuntimeData);
            OnExstensionActivated(Comp->ActiveMovementModeExtension, RuntimeContext);

        }
    }
    /*
    here goes shit from first if (false)/i would implement that shit later
    */
    static void (*TPairToArray)(TMap<FMovementMode_ExtID, UFortMovementMode_BaseExtLogic*>* Pair, TArray< UFortMovementMode_BaseExtLogic*>* Array) = decltype(TPairToArray)(ImageBase + 0x1FACEA0);
    TArray<UFortMovementMode_BaseExtLogic*> RegisteredLogics;
    TPairToArray(&Pawn->RegisteredMovementModeExtentionLogic, &RegisteredLogics);
    for (auto MME : RegisteredLogics) {
        if (MME) {
            *(BYTE*)(__int64(MME) + 0x10D) &= ~1u;
            void (*StrangeVtableFunc)(UFortMovementMode_BaseExtLogic*) = decltype(StrangeVtableFunc)(MME->VTable[0x4D8/8]);
            StrangeVtableFunc(MME);
        }
    }
    
    for (auto RuntimeData : Comp->ActiveLayeredMovementModeRuntimeData) {
        auto MME = FortMovementMode_BaseExtLogic::GetMovementModeExtById(Pawn, RuntimeData->LogicID);
        if (MME) {
            auto RuntimeContext = CreateRuntimeContext(RuntimeData);
            UpdateActiveLayer(MME, RuntimeContext);
        }
    }

    ToRemove.Free();
}
struct FServerMoveTick 
{
    UFortMovementComp_Character* Comp;
    int32 CurrentServerMoveTick = 0;
};
struct FServerMMEEntry
{
    UFortMovementComp_Character* Comp;
    UFortMovementMode_BaseExtRuntimeData* Data;
    uint32 LogicCRC;
    uint8 RuntimeID;
    int32 LastSeenServerMoveTick;
};

TArray<FServerMoveTick> ServerMoveTick;
TArray<FServerMMEEntry> ActiveMME;

void AddMME(UFortMovementComp_Character* Comp, UFortMovementMode_BaseExtRuntimeData* RuntimeData) {
    auto Pawn = Cast<AFortPawn>(Comp->PawnOwner);
    if (!Pawn)
        return;

    Comp->ActiveMovementModeExtensionRuntimeData = RuntimeData;
    Comp->ActiveMovementModeExtension = FortMovementMode_BaseExtLogic::GetMovementModeExtById((AFortPlayerPawn*)Comp->PawnOwner, RuntimeData->LogicID);
    Comp->ActiveLayeredMovementModeRuntimeData.Add(RuntimeData);
    Pawn->ActiveMovementModeExtensionRuntimeData = RuntimeData;
}

void CleanupMME(UFortMovementComp_Character* Comp)
{
    auto CurrentServerMoveTick = 0;
    for (auto& Tick : ServerMoveTick) {
        if (Tick.Comp == Comp)
            CurrentServerMoveTick = Tick.CurrentServerMoveTick;
    }
    
    const int32 MissingTickThreshold = 3; // 5 ticks could be better ig

    for (int32 i = ActiveMME.Num() - 1; i >= 0; i--)
    {
        FServerMMEEntry& Entry = ActiveMME[i];
        if (Comp != Entry.Comp)
            continue;

        UFortMovementMode_BaseExtRuntimeData* Data = Entry.Data;

        bool Remove = false;

        if (CurrentServerMoveTick - Entry.LastSeenServerMoveTick > MissingTickThreshold)
            Remove = true;

        if (Remove) 
        {
            auto Name = Data->GetName();
            auto Pawn = (AFortPlayerPawnAthena*)Comp->GetOwner();
            std::cout << "Removing MME: " << Name << " (ID: " << Data->LogicID.MovementMode_ExtCRC_ID  << ")\n";

            // no idea why they dont remove the cue automaticly 
            if (Data->IsA<UFortMovementMode_PlayerNitroStateRuntimeData>())
            {
                auto NitroStateLogic = (UFortMovementMode_ExtLogicNitroState*)FortMovementMode_BaseExtLogic::GetMovementModeExtById(Pawn, Data->LogicID);
                for (auto& Tag : NitroStateLogic->GameplayCuesTagsToPush)
                    NitroStateLogic->RemoveGameplayCue_Replicated(Tag);
            }
            // universal mme or smth like that/ fists mme
            if (Data->IsA<UFortMovementMode_MovementAttributeRuntimeData>())
            {
              auto AttributeLogic = (UFortMovementMode_ExtMovementAttribute*)FortMovementMode_BaseExtLogic::GetMovementModeExtById(Pawn, Data->LogicID);
                std::cout << "Universa" << AttributeLogic->Name.ToString().c_str() <<  std::endl;
            }

            // to rewrite
            if (Data->IsA<UFortMovementMode_TacSprintRuntimeData>())
            {
                if (Pawn && Pawn->HolsterWeaponRequests.Num() > 0)
                {
                    for (int32 j = 0; j < Pawn->HolsterWeaponRequests.Num(); j++)
                    {
                        auto& holsterRequest = Pawn->HolsterWeaponRequests[j];
                        Pawn->SetHolsterWeaponBP(holsterRequest.HolsterSource, false, false, holsterRequest.HolsterId);
                        Pawn->CurrentWeapon->bShowWhenHolstered = false; // i think its needed too
                        Pawn->BroadcastHolsterWeaponEvent(false);
                    }
                } 
            }
            // idk make it a seperate function so its cleaner ig? idk
            auto RemoveMME = [Data](TArray<UFortMovementMode_BaseExtRuntimeData*>& Array) {
                std::vector<int> IndexesToRemove;
                for (auto i = 0; i < Array.Num(); i++) {
                    if (Array[i] && Array[i]->LogicID.MovementMode_ExtCRC_ID == Data->LogicID.MovementMode_ExtCRC_ID)
                        IndexesToRemove.push_back(i);
                }
                for (auto i : IndexesToRemove) {
                    Array.Remove(i);
                }
            };
            RemoveMME(Comp->PushedMovementModeExtensionRuntimeData);
            RemoveMME(Comp->ActiveLayeredMovementModeRuntimeData);
            RemoveMME(Comp->NewlyAddedMovementModeExtensionRuntimeData);

            ActiveMME.Remove(i);
        }
    }
}
void HandleMME(UFortMovementComp_Character* Comp, TArray<UFortMovementMode_BaseExtRuntimeData*> MoveMMEs)
{
    auto CurrentServerMoveTick = 0;
    bool FoundTick = false;
    for (auto& Tick : ServerMoveTick) {
        if (Tick.Comp == Comp)
        {
            Tick.CurrentServerMoveTick++;
            CurrentServerMoveTick = Tick.CurrentServerMoveTick;
            FoundTick = true;
        }
            
    }
    if (!FoundTick) {
        FServerMoveTick Tick;
        Tick.CurrentServerMoveTick = 1;
        CurrentServerMoveTick = Tick.CurrentServerMoveTick;
        Tick.Comp = Comp;
        ServerMoveTick.Add(Tick);
    }

    for (int32 i = 0; i < MoveMMEs.Num(); i++)
    {
        UFortMovementMode_BaseExtRuntimeData* Data = MoveMMEs[i];
        if (!Data)
            continue;

        uint32 CRC = Data->LogicID.MovementMode_ExtCRC_ID;
        uint8 Runtime = Data->RuntimeID;

        bool Found = false;

        for (int32 j = 0; j < ActiveMME.Num(); j++)
        {
            FServerMMEEntry& Entry = ActiveMME[j];
            if (Entry.Comp != Comp)
                continue;

            if (Entry.LogicCRC == CRC && Entry.RuntimeID == Runtime)
            {
                Entry.LastSeenServerMoveTick = CurrentServerMoveTick;
                Found = true;
                break;
            }
        }

        if (Found)
            continue;

        FServerMMEEntry NewEntry;
        NewEntry.Comp = Comp;
        NewEntry.Data = Data;
        NewEntry.LogicCRC = CRC;
        NewEntry.RuntimeID = Runtime;
        NewEntry.LastSeenServerMoveTick = CurrentServerMoveTick;

        ActiveMME.Add(NewEntry);
        Comp->PushedMovementModeExtensionRuntimeData.Add(Data);
        Comp->NewlyAddedMovementModeExtensionRuntimeData.Add(Data);
        
    }
}
//Alr so to fix most issues(ig) i need to handle and update activedata (updating in activelayered)
void FortMovementComp_Character::ServerMove_PerformMovement(UFortMovementComp_Character* Comp, __int64 MoveData) {
    auto RuntimeDatas = *(TArray<UFortMovementMode_BaseExtRuntimeData*>*)(MoveData + 0xC8);
    HandleMME(Comp, RuntimeDatas);
    CleanupMME(Comp);
    ServerMove_PerformMovementOG(Comp, MoveData);
}

bool ReturnTrue() {
    return true;
}


void FortMovementComp_Character::Init() {
    UHook* Hook = new UHook();

    Hook->Address = ImageBase + 0x9D89CD8;
    Hook->Detour = ServerMove_PerformMovement;
    Hook->Original = (void**)&ServerMove_PerformMovementOG;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Address = 0x139B788 + ImageBase;
    Hook->Detour = CoolFuncion;
    //UKismetHookingLibrary::Hook(Hook, Address);

    delete Hook;
}