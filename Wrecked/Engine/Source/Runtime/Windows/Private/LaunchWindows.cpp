#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/AthenaMarkerComponent.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/AthenaSpecialActorComponent.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/FortControllerComponent_RechargeWeapons.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Building/BuildingProp_ConversationCompatible.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Building/BuildingSMActor.h"
#include "Engine/Source/Runtime/FortniteGame/Public/FortniteAI/FortAthenaPatrolPath.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPlayerAugmentItemDefinition.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldDensityGrid.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldStaticPointProvider.h"
#ifndef Client
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine//NetDriver.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaCreativePortal.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldManager.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaVehicle.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortGameModeAthena.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Player/FortPlayerControllerAthena.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Player/FortPlayerController.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Building/BuildingContainer.h"
#include "Engine/Source/Runtime/GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Player/FortPlayerPawn.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/Projectiles/FortLightWeightProjectileManager.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/Projectiles/FortProjectileAthena.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Movement/FortMovementComp_Character.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortPoiVolume.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortVehicleMod.h"
#include "Engine/Source/Runtime/FortniteGame/Public/FortniteAI/AthenaAIServicePlayerBots.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/AI/FortAthenaAISpawnerData.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponModStation.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortWorldItem.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/InventoryManagementLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortInventoryOwnerInterface.h"
#include "Engine/Source/Runtime/FortniteGame/Public/FortniteAI/FortAthenaBTService_PickUpLoot.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "SDK/SDK/InfernoRuntime_classes.hpp"
#include "Engine/Source/Runtime/FortniteGame/Public/Conversation/ConversationLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Online/FortGameSessionDedicated.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPickup.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponRanged.h"
#endif
//#define Client
inline __int64 (*SendRequestNowOG)(UMcpProfileGroup* Group, void* HttpRequest, int a3);
__int64 SendRequestNow(UMcpProfileGroup* Group, void* HttpRequest, int a3)
{
    return SendRequestNowOG(Group, HttpRequest, 3);
}

template <typename _Is>
__forceinline void Patch(uintptr_t ptr, _Is byte)
{
    DWORD og;
    VirtualProtect(LPVOID(ptr), sizeof(_Is), PAGE_EXECUTE_READWRITE, &og);
    *(_Is*)ptr = byte;
    VirtualProtect(LPVOID(ptr), sizeof(_Is), og, &og);
}

void PatchAllNetModes()
{
    Memcury::PE::Address add{ nullptr };

    const auto sizeOfImage = Memcury::PE::GetNTHeaders()->OptionalHeader.SizeOfImage;
    const auto scanBytes = reinterpret_cast<std::uint8_t*>(Memcury::PE::GetModuleBase());
    auto AttemptDeriveFromURL = Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B D1").GetAs<void*>();

    for (auto i = 0ul; i < sizeOfImage - 5; ++i)
    {
        if (scanBytes[i] == 0xE8)
        {
            if (Memcury::PE::Address(&scanBytes[i]).RelativeOffset(1).GetAs<void*>() == AttemptDeriveFromURL)
            {

                add = Memcury::PE::Address(&scanBytes[i]);

                // scan for the read of World->NetDriver

                for (auto j = 0; j > -0x100000; j--) // so we find everything. no func is actually 1mb
                {
                    if ((scanBytes[i + j] & 0xF8) == 0x48 && ((scanBytes[i + j + 1] & 0xFC) == 0x80 || (scanBytes[i + j + 1] & 0xF8) == 0x38) && scanBytes[i + j + 3] == 0x38)
                    {
                        // now, scan for if (NetDriver) return NM_Client;

                        bool found = false;
                        for (auto k = 0; k < 0x100; k++)
                        {
                            if (scanBytes[i + j + k] == 0x75)
                            {
                                auto Scuffness = __int64(&scanBytes[i + j + k + 5]);

                                if (*(uint32_t*)Scuffness != 0xF0 && (scanBytes[i + j + k + 4] != 0xC || scanBytes[i + j + k + 5] != 0xB) && scanBytes[i + j + k + 4] != 0x09)
                                    continue;

                                Patch<uint16_t>(__int64(&scanBytes[i + j + k]), 0x9090);
                                found = true;
                                break;
                            }
                            else if (scanBytes[i + j + k] == 0x74)
                            {
                                auto Scuffness = __int64(&scanBytes[i + j + k]);
                                Scuffness = (Scuffness + 2) + *(int8_t*)(Scuffness + 1);

                                if (*(uint32_t*)(Scuffness + 3) != 0xF0 && (*(uint8_t*)(Scuffness + 2) != 0xC || *(uint8_t*)(Scuffness + 3) != 0xB) && *(uint8_t*)(Scuffness + 2) != 0x09)
                                    continue;

                                Patch<uint8_t>(__int64(&scanBytes[i + j + k]), 0xeb);
                                found = true;
                                break;
                            }
                            else if (scanBytes[i + j + k] == 0x0F && scanBytes[i + j + k + 1] == 0x85)
                            {
                                auto Scuffness = __int64(&scanBytes[i + j + k + 9]);

                                if (*(uint32_t*)Scuffness != 0xF0 && (scanBytes[i + j + k + 8] != 0xC || scanBytes[i + j + k + 9] != 0xB) && scanBytes[i + j + k + 8] != 0x09)
                                    continue;

                                Patch<uint32_t>(__int64(&scanBytes[i + j + k]), 0x90909090);
                                Patch<uint16_t>(__int64(&scanBytes[i + j + k + 4]), 0x9090);
                                found = true;
                                break;
                            }
                            else if (scanBytes[i + j + k] == 0x0F && scanBytes[i + j + k + 1] == 0x84)
                            {
                                auto Scuffness = __int64(&scanBytes[i + j + k]);
                                Scuffness = (Scuffness + 6) + *(int32_t*)(Scuffness + 2);

                                if (*(uint32_t*)(Scuffness + 3) != 0xF0 && (*(uint8_t*)(Scuffness + 2) != 0xC || *(uint8_t*)(Scuffness + 3) != 0xB) && *(uint8_t*)(Scuffness + 2) != 0x09)
                                    continue;

                                Patch<uint16_t>(__int64(&scanBytes[i + j + k]), 0xe990);
                                found = true;
                                break;
                            }
                        }
                        if (found)
                            break;
                    }
                }
            }
        }
    }
}
int CheckCheckpoint() {
    return -1;
}

void ProcessEventHook(UObject* Object, UFunction* Function, void* Parameters) {
    if (!Function)
        return;
    if (Function->Name.ToString().contains("ServerNotifyPawnHit"))
    {
        logtofile(Function->GetFullName().c_str());
    }
    //logtofile(Function->GetFullName().c_str());
    return UObject::ProcessEventOG(Object, Function, Parameters);
}

static inline char (*GetStringOG)(__int64 a1, const wchar_t* Section, const wchar_t* Key, __int64* Value, __int64* Filename) = nullptr;
char GetStringINI(__int64 a1, const wchar_t* Section, const wchar_t* Key, __int64* Value, __int64* Filename) {
    if (Section && Key) {
        if (wcscmp(Section, L"OnlineSubsystemMcp.McpProfile") == 0 && wcslen(Key) == 0) {
            Key = L"McpDedicatedServerCommandUrl";
        }
    }

    return GetStringOG(a1, Section, Key, Value, Filename);
}
void DebugFunc() {
    throw out_of_range("AHHHAHAHHA");
}
void Main()
{
    MH_Initialize();
#ifndef Client
    AllocConsole();
    FILE* File = nullptr;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    freopen_s(&File, "CONOUT$", "w+", stderr);
    SetConsoleTitleA("Wrecked | Setting up");

    Sleep(15000);
    
    while (!IsInFrontend())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    SetConsoleTitleA(("Wrecked 30.00 - Preparing - Joinable = false | Compiled at " +
        std::string(__TIME__)).c_str());
    *(bool*)(ImageBase + 0x12A37C1A) = false; 
    *(bool*)(ImageBase + 0x12A37BAC) = true; 
    
    PatchAllNetModes();
    FortGameModeAthena::Init();
    FortPlayerControllerAthena::Init();
    FortPlayerController::Init();
    NetDriver::Init();
    BuildingContainer::Setup();
    AbilitySystemComponent::Setup();
    FortPlayerPawn::Init();
    FortKismetLibrary::Init();
    FortLightWeightProjectileManager::Init();
    FortProjectileAthena::Init();
    FortMovementComp_Character::Init();
    FortAthenaVehicle::Hook();
    FortAthenaLivingWorldManager::Hook();
    FortPoiVolume::Setup();
    AthenaAIServicePlayerBots::Init();
    FortAthenaAISpawnerData::Init();
    FortAthenaCreativePortal::Init();
    FortVehicleMod::Init();
    FortWeaponModStation::Init();
    FortWorldItem::Init();
    InventoryManagementLibrary::Init();
    FortInventoryOwnerInterface::Init();
    FortGameStateComponent_BattleRoyaleGamePhaseLogic::Init();
    ConversationLibrary::Init();
    FortAthenaLivingWorldStaticPointProvider::Init();
    FortPlayerAugmentItemDefinition::Init();
    //AthenaSpecialActorComponent::Init();
    AthenaMarkerComponent::Init();
    FortGameSessionDedicated::Init();
    FortAthenaPatrolPath::Init();
    FortAthenaBTService_PickUpLoot::Init();
    FortAthenaLivingWorldDensityGrid::Hook();
    FortControllerComponent_RechargeWeapons::Init();
    BuildingProp_ConversationCompatible::Init();
    BuildingSMActor::Init();
    FortWeaponRanged::Init();
    FortPickup::Init();
    std::random_device rd;
    FMath::RandInit(rd());


    UHook* Hook = new UHook();

    std::vector<uintptr_t> NullFuncs = {
        0x1C6A23C,
        0x1C6A588,
        0xA90678C, //inventoryPostOnspawned (crashing for no reason)
        0xC8AF1DC //PropagateGrassInBounds for curie
    };

    std::vector<uintptr_t> RetTrueFuncs = {
        0x29702D8,  // KickPlayer
        0x15AA5CC,  // WorldGetNetMode
        0x15AA5AC,  // DriverGetNetMode
        0x24B7E94,  // InternaalGetNetMode
        0x2958210,  // SupportsMakingVisibleTransactionRequests
        0xB68E5C4 //UFortDeterminismConfig::IsUsingNoLager()
    };

    std::vector<uintptr_t> RetFalseFuncs = {
        0x1458F48,
    };

    for (const auto& Addr : NullFuncs) {
        Hook->Address = ImageBase + Addr;
        Hook->Byte = 0xC3;
        UKismetHookingLibrary::Hook(Hook, Byte);
    }

    for (const auto& Addr : RetTrueFuncs) {
        Hook->Address = ImageBase + Addr;
        UKismetHookingLibrary::Hook(Hook, RTrue);
    }

    for (const auto& Addr : RetFalseFuncs) {
        Hook->Address = ImageBase + Addr;
        UKismetHookingLibrary::Hook(Hook, RFalse);
    }

    Hook->Address = ImageBase + 0x4A90E20;
    Hook->Detour = CheckCheckpoint;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Address = ImageBase + 0x1E5FE78;
    Hook->Detour = DebugFunc;
    UKismetHookingLibrary::Hook(Hook, Address);
    
    Hook->Address = ImageBase + 0x7C082F4;
    Hook->Original = (void**)&SendRequestNowOG;
    Hook->Detour = SendRequestNow;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Address = ImageBase + Offsets::ProcessEvent;
    Hook->Original = (void**)&UObject::ProcessEventOG;
    Hook->Detour = ProcessEventHook;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Address = ImageBase + 0x18F6540;
    Hook->Original = (void**)&GetStringOG;
    Hook->Detour = GetStringINI;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Address = ImageBase + 0x2958194;
    Hook->Byte = 0x01;
    UKismetHookingLibrary::Hook(Hook, Byte);

    GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    *(bool*)(__int64(ImageBase) + 0x12BF67B0) = true;
    //*(int*)(__int64(ImageBase) + 0x12BCB068) = 7;
    //*(int*)(ImageBase + 0x12BCB280) = 7; //mme
    *(int*)(ImageBase + 0x12BCB180) = 7;
    *(int*)(ImageBase + 0x12BD3B70) = 7; // lwm log of some sort
    *(int*)(ImageBase + 0x12BCB0A8) = 7;
    *(int*)(ImageBase + 0x12CEABC0) = 7; // recharge weapon log
    *(bool*)(ImageBase + 0x12D84D92) = false;
    *(int*)(ImageBase + 0x12BCFF68) = 7; // something with phoebe ais

    if (CurrentGamemode == EGameModeType::Papaya)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"open Apollo_Papaya", nullptr);
    }
    else if (CurrentGamemode == EGameModeType::Juno)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(),  L"open AW_Village_POI", nullptr);
    }
    else if (CurrentGamemode == EGameModeType::Creative)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(),  L"open Creative_NoApollo_Terrain", nullptr);
    }
    else if (CurrentGamemode == EGameModeType::Delmar)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(),  L"open DelMar_Tutorials_SeamlessTutorialRun", nullptr);
    } 
    else if (CurrentGamemode == EGameModeType::BlastBerry) {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"open /Game/BlastBerryMap/NewWorld.NewWorld", nullptr);
    }
    else
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(),  L"open Helios_Terrain", nullptr);
    }
    
    //UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), CurrentGamemode == EGameModeType::Juno ? L"open 1_WWWG_0": L"open Helios_Terrain", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortUIDirector", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"net.allowencryption 0", nullptr);
    //UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogIris VeryVerbose", nullptr);
    //UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogReplicationSystem VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"net.Iris.UseIrisReplication 1", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogEOSSDK off", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnline off", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAscenderZipline none", nullptr); // random logs that pretty much we dont care about 
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortChoosePlayerStart VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnlineGame VeryVerbose", nullptr);
    
    for (auto& Curie : GetObjectsOfClass<UCurieGlobals>())
       Curie->bEnableCurie = false;
    
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAthenaAIServiceBots VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAthenaBots VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAISpawnerData VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogNiagara none", nullptr); // we hate niagara
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogVehicleWeapon VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogWeaponModStation VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAnalytics none", nullptr); // we hate the api server
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogCreativePlots VeryVerbose", nullptr); // somehow portals are not stripped fully
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogFortControllerComponentHabanero VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogFortQuest VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogOnlineSession VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogFortPickup VeryVerbose", nullptr);

#endif
#ifdef Client
    UEngine::GetEngine()->GameViewport->ViewportConsole = reinterpret_cast<UConsole*>(UGameplayStatics::SpawnObject(UConsole::StaticClass(), UEngine::GetEngine()->GameViewport));
    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"net.allowencryption 0", nullptr);
    *(bool*)(__int64(IMAGEBASE) + 0x12BF67B0) = true;
    
    UHook* Hook = new UHook();
    Hook->Address = IMAGEBASE + 0x90957DC;
    UKismetHookingLibrary::Hook(Hook, RFalse);
#endif
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == 1)
    {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Main, nullptr, 0, nullptr);
    }
    
    return TRUE;
}

