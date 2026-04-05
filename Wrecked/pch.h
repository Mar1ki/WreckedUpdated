// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once

#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <cstring>
#include <string>
#include <thread>
#include <random>
#include <numeric>
#include <algorithm>
#include <sstream>
#include "SDK/SDK/Basic.hpp"
#include "SDK/SDK/CoreUObject_structs.hpp"
#include "SDK/SDK/CoreUObject_classes.hpp"
#include "SDK/SDK/Engine_structs.hpp"
#include "SDK/SDK/Engine_classes.hpp"
#include "SDK/SDK/FortniteGame_structs.hpp"
#include "SDK/SDK/FortniteGame_classes.hpp"
#include "SDK/SDK/GameplayAbilities_structs.hpp"
#include "SDK/SDK/GameplayAbilities_classes.hpp"
#include "SDK/SDK/PlayerPawn_Athena_classes.hpp"
#include "SDK/SDK/AscenderCodeRuntime_classes.hpp"
#include "SDK/SDK/LagerRuntime_classes.hpp"
#include "SDK/SDK/TacticalSprintRuntime_classes.hpp"
#include "SDK/SDK/ClamberingCodeRuntime_classes.hpp"
#include "SDK/SDK/ValetRuntime_classes.hpp"
#include "SDK/SDK/JunoGameNative_classes.hpp"
#include "SDK/SDK/WeaponModsCodeRuntime_classes.hpp"
#include "SDK/SDK/AscenderCodeRuntime_classes.hpp"
#include "SDK/SDK/FortniteAI_classes.hpp"
#include "SDK/SDK/FortniteAIServer_classes.hpp"
#include <set>
#include <atomic>
#include "Engine/Source/Runtime/Core/Public/Math/UnrealMathUtility.h"
using namespace SDK;
using namespace std;
inline uint64_t ImageBase = *(uint64_t*)(__readgsqword(0x60) + 0x10);
template<class T>
inline T* Cast(UObject* Object)
{
        return Object && (Object->IsA(T::StaticClass())) ? (T*)Object : nullptr;
}
#include "Include/MinHook.h"
#include "Include/memcury.h"

#define NAME_None FName(0)

enum ESpawnActorNameMode : uint8
{
        Required_Fatal,
        Required_ErrorAndReturnNull,
        Required_ReturnNull,
        Requested
};

enum EGameModeType
{
        Athena, // br
        Papaya, // party royale
        Juno, // lego
        Creative, // creative
        STW, // Save the world
        Delmar,
        BlastBerry,
};

struct FActorSpawnParameters
{
public:
        FName Name;

        AActor* Template;
        AActor* Owner;
        APawn* Instigator;
        ULevel* OverrideLevel;
        UChildActorComponent* OverrideParentComponent;
        ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;
        ESpawnActorScaleMethod TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
public:
        uint8    bRemoteOwned : 1;
public:
        uint8    bNoFail : 1;
        uint8    bDeferConstruction : 1;
        uint8    bAllowDuringConstructionScript : 1;
        uint8    bForceGloballyUniqueName : 1;
        ESpawnActorNameMode NameMode;
        EObjectFlags ObjectFlags;
        void* CallbackSum_Callable;
        void* CallbackSum_HeapAllocation;
};

#define check(expr)                                                         \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::cerr << "Check failed: " << #expr << "\n"                  \
                      << "  File: " << __FILE__ << "\n"                     \
                      << "  Line: " << __LINE__ << "\n"                     \
                      << "  Function: " << __func__ << std::endl;           \
            std::abort();                                                   \
        }                                                                   \
    } while (0)


template <typename T>
constexpr const T& Min(const T& A, const T& B)
{
	return (A <= B) ? A : B;
}

template <typename T>
constexpr const T& Max(const T & A, const T & B)
{
	return (A >= B) ? A : B;
}

template <typename T>
constexpr const T& Clamp(const T& Value, const T& Min, const T& Max)
{
	return (Value < Min) ? Min : (Value > Max) ? Max : Value;
}

template <typename T, typename U>
constexpr auto Lerp(const T& A, const T& B, const U& Alpha) -> decltype(A + (B - A) * Alpha)
{
	return A + (B - A) * Alpha;
}

static int RetTrue()
{
	return 1;
}

static int RetFalse()
{
	return 0;
}

static void Return()
{
	return;
}

enum class ELogLevel { Fatal, Error, Warning, Log, Verbose, VeryVerbose };

inline constexpr ELogLevel Fatal = ELogLevel::Fatal;
inline constexpr ELogLevel Error = ELogLevel::Error;
inline constexpr ELogLevel Warning = ELogLevel::Warning;
inline constexpr ELogLevel Log = ELogLevel::Log;
inline constexpr ELogLevel Verbose = ELogLevel::Verbose;
inline constexpr ELogLevel VeryVerbose = ELogLevel::VeryVerbose;


inline const char *ToString( ELogLevel level ) {
        switch ( level ) {
        case ELogLevel::Verbose:
                return "Verbose";
        case ELogLevel::VeryVerbose:
                return "VeryVerbose";
        case ELogLevel::Log:
                return "Log";
        case ELogLevel::Warning:
                return "Warning";
        case ELogLevel::Error:
                return "Error";
        case ELogLevel::Fatal:
                return "Fatal";
        default:
                return "Unknown";
        }
}

inline const wchar_t *ToWString( ELogLevel level ) {
        switch ( level ) {
        case ELogLevel::Verbose:
                return L"Verbose";
        case ELogLevel::VeryVerbose:
                return L"VeryVerbose";
        case ELogLevel::Log:
                return L"Log";
        case ELogLevel::Warning:
                return L"Warning";
        case ELogLevel::Error:
                return L"Error";
        case ELogLevel::Fatal:
                return L"Fatal";
        default:
                return L"Unknown";
        }
}

inline void SetConsoleColorByLogLevel( ELogLevel Level ) {
        HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

        WORD color = FOREGROUND_RED | FOREGROUND_GREEN |
                     FOREGROUND_BLUE; // default white

        switch ( Level ) {
        case ELogLevel::Warning:
                color = FOREGROUND_RED | FOREGROUND_GREEN; // yellow
                break;
        case ELogLevel::Error:
                color = FOREGROUND_RED; // red
                break;
        case ELogLevel::Fatal:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY; // bright red
                break;
        case ELogLevel::Verbose:
                color = FOREGROUND_GREEN | FOREGROUND_BLUE; // cyan
                break;
        case ELogLevel::VeryVerbose:
                color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; // bright blue
                break;
        case ELogLevel::Log:
        default:
                color = FOREGROUND_RED | FOREGROUND_GREEN |
                        FOREGROUND_BLUE; // white
                break;
        }

        SetConsoleTextAttribute( hConsole, color );
}

inline void ResetConsoleColor() {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ),
                                 FOREGROUND_RED | FOREGROUND_GREEN |
                                     FOREGROUND_BLUE );
}


struct FLogCategory {
        const char *Name;
        ELogLevel RuntimeVerbosity;
};


#if _DEBUG
#define DEFINE_LOG_CATEGORY( CategoryName )                                    \
        inline FLogCategory CategoryName = { #CategoryName, ELogLevel::VeryVerbose }
#else
#define DEFINE_LOG_CATEGORY( CategoryName )                                    \
        inline FLogCategory CategoryName = { #CategoryName, ELogLevel::Log }
#endif

inline void UELogImpl(FLogCategory& Category, ELogLevel Level, const char* msg) {
        if ((int)Level > (int)Category.RuntimeVerbosity)
                return;

        SetConsoleColorByLogLevel(Level);
        std::cout << Category.Name << ": "
                  << ToString(Level) << ": " << msg << std::endl;
        ResetConsoleColor();
}

inline std::string WStringToUTF8(const std::wstring& wstr) {
        return std::string(wstr.begin(), wstr.end());
}

inline void UELogWImpl(FLogCategory& Category, ELogLevel Level, const wchar_t* msg) {
        if ((int)Level > (int)Category.RuntimeVerbosity)
                return;

        std::wstring wfinal(msg);
        std::string finalbuf = WStringToUTF8(wfinal);

        SetConsoleColorByLogLevel(Level);
        std::cout << Category.Name << ": "
                  << ToString(Level) << ": " << finalbuf << std::endl;
        ResetConsoleColor();
}

inline void UE_LOG_DISPATCH(FLogCategory& Category, ELogLevel Level, const char* Format, ...) {
        va_list args;
        va_start(args, Format);

        char buffer[2048];
        vsnprintf(buffer, sizeof(buffer), Format, args);
        va_end(args);

        UELogImpl(Category, Level, buffer);
}

inline void UE_LOG_DISPATCH(FLogCategory& Category, ELogLevel Level, const wchar_t* Format, ...) {
        va_list args;
        va_start(args, Format);

        wchar_t buffer[2048];
        vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), Format, args);
        va_end(args);

        UELogWImpl(Category, Level, buffer);
}

#define UE_LOG(Category, Verbosity, Format, ...) \
UE_LOG_DISPATCH(Category, Verbosity, Format, ##__VA_ARGS__)

#define UE_LOG_W(Category, Verbosity, Format, ...) \
UE_LOG_DISPATCH(Category, Verbosity, Format, ##__VA_ARGS__)

#define DefineOriginal(_Rt, _Name, ...) static inline _Rt (*_Name##OG)(##__VA_ARGS__); static _Rt _Name(##__VA_ARGS__);

DEFINE_LOG_CATEGORY(LogIris);
DEFINE_LOG_CATEGORY(LogReplication);
DEFINE_LOG_CATEGORY(LogWrecked);
DEFINE_LOG_CATEGORY(LogServer);
DEFINE_LOG_CATEGORY(LogAthena);
DEFINE_LOG_CATEGORY(LogMatch);
DEFINE_LOG_CATEGORY(LogTeams);
DEFINE_LOG_CATEGORY(LogStorm);
DEFINE_LOG_CATEGORY(LogInventory);
DEFINE_LOG_CATEGORY(LogWeapons);
DEFINE_LOG_CATEGORY(LogAbilities);
DEFINE_LOG_CATEGORY(LogItems);
DEFINE_LOG_CATEGORY(LogVehicles);
DEFINE_LOG_CATEGORY(LogBuilding);
DEFINE_LOG_CATEGORY(LogAI);
DEFINE_LOG_CATEGORY(LogPlayer);
DEFINE_LOG_CATEGORY(LogPawn);
DEFINE_LOG_CATEGORY(LogCreative);
DEFINE_LOG_CATEGORY(LogQuests);
DEFINE_LOG_CATEGORY(LogTemp);
DEFINE_LOG_CATEGORY(LogCheats);
DEFINE_LOG_CATEGORY(LogKismet);
DEFINE_LOG_CATEGORY(LogGameMode);
DEFINE_LOG_CATEGORY(LogGamePhases);
DEFINE_LOG_CATEGORY(LogGameState);
DEFINE_LOG_CATEGORY(LogNet);
DEFINE_LOG_CATEGORY(LogProjectileManager);
DEFINE_LOG_CATEGORY(LogMME);
DEFINE_LOG_CATEGORY(LogLivingWorldManager);
DEFINE_LOG_CATEGORY(LogWeaponMods);
DEFINE_LOG_CATEGORY(LogWeaponModStation);
DEFINE_LOG_CATEGORY(LogInventoryManagmentLibrary);
DEFINE_LOG_CATEGORY(LogFortInventoryOwnerInterface);
DEFINE_LOG_CATEGORY(LogLivingWorldStaticPointProvider);

inline bool IsInFrontend()
{
	if (auto World = SDK::UWorld::GetWorld())
	{
		if (World->GetName().contains("Frontend"))
		{
			return true;
		}
	}
}

template<typename T>
        __forceinline std::vector<T*> GetObjectsOfClass()
{
        std::vector<T*> Objects;
        for (int i = 0; i < UObject::GObjects->Num(); i++)
        {
                T* Object = reinterpret_cast<T*>(UObject::GObjects->GetByIndex(i));
                if (!Object) continue;
                if (Object->IsA(T::StaticClass()))
                {
                        Objects.push_back(Object);
                }
        }
        return Objects;
}
template<typename T>
__forceinline T* GetFirstObjectOfClass()
{
        for (int i = 0; i < UObject::GObjects->Num(); i++)
        {
                T* Object = reinterpret_cast<T*>(UObject::GObjects->GetByIndex(i));
                if (!Object) continue;
                if (Object->IsA(T::StaticClass()))
                {
                        return Object;
                }
        }
        return nullptr;
}

inline bool IsValidPointer(void* ptr) {
        return ptr != nullptr && ptr != (void*)0xFFFFFFFFULL && ptr != (void*)0xCCCCCCCCULL && ptr != (void*)0xDDDDDDDDULL && (uintptr_t)ptr > 0x10000;
}

inline UWorld* GetWorld()
{
        return UWorld::GetWorld();
}

inline AFortGameModeAthena* GetGameMode()
{
        if (auto World = GetWorld())
        {
                return (AFortGameModeAthena*)World->AuthorityGameMode;
        }
        return nullptr;
}

inline AFortGameStateAthena* GetGameState()
{
        if (auto World = GetWorld())
        {
                return (AFortGameStateAthena*)World->GameState;
        }
        return nullptr;
}

inline UFortAssetManager* GetAssetManager()
{
        static UFortAssetManager* FortAssetManager = nullptr;
        if (!FortAssetManager)
        {
                UFortAssetManager* AssetManager = (UFortAssetManager*)UFortEngine::GetEngine()->AssetManager;
                FortAssetManager = AssetManager;
        }
        
        return FortAssetManager;
}

inline UFortAthenaLivingWorldManager* GetLivingWorldManager()
{
        static UFortAthenaLivingWorldManager* LivingWorldManager = nullptr;
        if (!LivingWorldManager)
        {
                LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass());
        }
        return LivingWorldManager;
}

inline AFortAthenaLivingWorldPointProvidersEnabler* GetLivingWorldPointProviderEnabler()
{
        static AFortAthenaLivingWorldPointProvidersEnabler* LivingWorldPointProviderEnabler = nullptr;
        if (!LivingWorldPointProviderEnabler)
        {
              LivingWorldPointProviderEnabler = GetFirstObjectOfClass<AFortAthenaLivingWorldPointProvidersEnabler>();
        }
        return LivingWorldPointProviderEnabler;
}


#define getCounter() __counter__
#define __runOnce(_V) static uint32_t _ROnce_##_V = 0; if (++_ROnce_##_V == 1)
#define _runOnce(_V) __runOnce(_V)
#define runOnce _runOnce(__COUNTER__)

template<typename T, typename Func>
inline void ForEachMutator(UWorld* World, UClass* MutatorClass, Func&& Callback)
{
        if (!World || !MutatorClass) return;
    
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, MutatorClass, &FoundActors);

        for (AActor* Actor : FoundActors)
                if (Actor->IsA(T::StaticClass())) Callback((T*)Actor);
}

inline UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GetLogic()
{
     return UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(GetWorld());
}

#define INV_PI			(0.31830988618f)
#define HALF_PI			(1.57079632679f)
#define PI 					(3.1415926535897932f)

inline float RadiansToDegrees(float Radians)
{
        return Radians * (180.0f / PI);
}
inline void SinCos(float* ScalarSin, float* ScalarCos, float  Value)
{
        float quotient = (INV_PI * 0.5f) * Value;
        if (Value >= 0.0f)
        {
                quotient = (float)((int)(quotient + 0.5f));
        }
        else
        {
                quotient = (float)((int)(quotient - 0.5f));
        }
        float y = Value - (2.0f * PI) * quotient;

        float sign;
        if (y > HALF_PI)
        {
                y = PI - y;
                sign = -1.0f;
        }
        else if (y < -HALF_PI)
        {
                y = -PI - y;
                sign = -1.0f;
        }
        else
        {
                sign = +1.0f;
        }

        float y2 = y * y;

        *ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

        float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
        *ScalarCos = sign * p;
}

inline FRotator FQuatToRot(FQuat Quat)
{
        const float SingularityTest = Quat.Z * Quat.X - Quat.W * Quat.Y;
        const float YawY = 2.f * (Quat.W * Quat.Z + Quat.X * Quat.Y);
        const float YawX = (1.f - 2.f * (FMath::Square(Quat.Y) + FMath::Square(Quat.Z)));

        const float SINGULARITY_THRESHOLD = 0.4999995f;
        const float RAD_TO_DEG = (180.f) / PI;
        FRotator RotatorFromQuat;

        if (SingularityTest < -SINGULARITY_THRESHOLD)
        {
                RotatorFromQuat.pitch = -90.f;
                RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
                RotatorFromQuat.Roll = UKismetMathLibrary::NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * UKismetMathLibrary::Atan2(Quat.X, Quat.W) * RAD_TO_DEG));
        }
        else if (SingularityTest > SINGULARITY_THRESHOLD)
        {
                RotatorFromQuat.pitch = 90.f;
                RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
                RotatorFromQuat.Roll = UKismetMathLibrary::NormalizeAxis(RotatorFromQuat.Yaw - (2.f * UKismetMathLibrary::Atan2(Quat.X, Quat.W) * RAD_TO_DEG));
        }
        else
        {
                RotatorFromQuat.pitch = UKismetMathLibrary::asin(2.f * (SingularityTest)) * RAD_TO_DEG;
                RotatorFromQuat.Yaw = UKismetMathLibrary::Atan2(YawY, YawX) * RAD_TO_DEG;
                RotatorFromQuat.Roll = UKismetMathLibrary::Atan2(-2.f * (Quat.W * Quat.X + Quat.Y * Quat.Z), (1.f - 2.f * (FMath::Square(Quat.X) + FMath::Square(Quat.Y)))) * RAD_TO_DEG;
        }
        return RotatorFromQuat;
}

inline float WeightedRandom(float AllWeights)
{
    return (rand() * 0.000030518509f) * AllWeights;
}

inline void logtofile(const std::string& message) {
        std::ofstream file("log.txt", std::ios::app);
        if (file.is_open()) {
                file << message << std::endl;
        }
}
enum class EGetSparseClassDataMethod : uint8
{
    CreateIfNull,
    ArchetypeIfNull,
    ReturnIfNull,
};

template<class T>
T* GetSparseData(UObject* _this, EGetSparseClassDataMethod method = EGetSparseClassDataMethod::ArchetypeIfNull) {
    static void* (__fastcall * GetSparseClassDataOG)(UObject * _this, EGetSparseClassDataMethod) = decltype(GetSparseClassDataOG)(InSDKUtils::GetImageBase() + 0x1572574);
    return (T*)GetSparseClassDataOG(_this, method);
}
inline std::string to_hex(__int64 Address)
{
    std::stringstream stream;
    stream
        << std::hex << Address;
    return stream.str();
}
template<typename T>
__forceinline T* GetRandomObjectOfClass()
{
        std::vector<T*> Objects;
        for (int i = 0; i < UObject::GObjects->Num(); i++)
        {
                T* Object = reinterpret_cast<T*>(UObject::GObjects->GetByIndex(i));
                if (Object && Object->IsA(T::StaticClass()))
                        Objects.push_back(Object);
        }
        if (Objects.empty())
                return nullptr;
    
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, Objects.size() - 1);

        T* Result = nullptr;
        while (!Result)
                Result = Objects[dis(gen)];

        return Result;
}
// here we need to check max ammo so it spawns with the full mag not the default full mag
inline void ApplyWeaponModset(AFortPickupAthena* Pickup, UFortWeaponModSetData* Modset) {
    static unsigned __int64* (__fastcall * sub_13DC400)(__int64* a1, UFortWeaponModItemDefinition * a2) = decltype(sub_13DC400)(InSDKUtils::GetImageBase() + 0x13DC400);
    static __int64(__fastcall * sub_9BA1D68)(FFortItemEntry * Entry, __int64* a2) = decltype(sub_9BA1D68)(InSDKUtils::GetImageBase() + 0x9BA1D68);

    auto WeaponDef = Cast<UFortWeaponItemDefinition>(Pickup->PrimaryPickupItemEntry.ItemDefinition);
    TArray<FFortModSetModEntry> Mods;
    switch (WeaponDef->Rarity) {
    case EFortRarity::Common:
        Mods = Modset->CommonRarityMods;
        break;
    case EFortRarity::Uncommon:
        Mods = Modset->UncommonRarityMods;
        break;
    case EFortRarity::Rare:
        Mods = Modset->RareRarityMods;
        break;
    case EFortRarity::Epic:
        Mods = Modset->EpicRarityMods;
        break;
    case EFortRarity::Legendary:
        Mods = Modset->LengdaryRarityMods;
        break;
    default:
        break;
    }

    for (auto ModSlot : WeaponDef->WeaponModSlots) {
        Pickup->PrimaryPickupItemEntry.WeaponModSlots.Add(ModSlot);
    }
    for (auto Mod : Mods) {
        __int64 v9;
        sub_13DC400(&v9, Mod.ModDefinition);
        sub_9BA1D68(&Pickup->PrimaryPickupItemEntry, &v9);
    }
}

inline void ApplyWeaponModset(FFortItemEntry* Entry, UFortWeaponModSetData* Modset) {
    static unsigned __int64* (__fastcall * sub_13DC400)(__int64* a1, UFortWeaponModItemDefinition * a2) = decltype(sub_13DC400)(InSDKUtils::GetImageBase() + 0x13DC400);
    static __int64(__fastcall * sub_9BA1D68)(FFortItemEntry * Entry, __int64* a2) = decltype(sub_9BA1D68)(InSDKUtils::GetImageBase() + 0x9BA1D68);

    auto WeaponDef = Cast<UFortWeaponItemDefinition>(Entry->ItemDefinition);
    TArray<FFortModSetModEntry> Mods;
    switch (WeaponDef->Rarity) {
    case EFortRarity::Common:
        Mods = Modset->CommonRarityMods;
        break;
    case EFortRarity::Uncommon:
        Mods = Modset->UncommonRarityMods;
        break;
    case EFortRarity::Rare:
        Mods = Modset->RareRarityMods;
        break;
    case EFortRarity::Epic:
        Mods = Modset->EpicRarityMods;
        break;
    case EFortRarity::Legendary:
        Mods = Modset->LengdaryRarityMods;
        break;
    default:
        break;
    }

    for (auto ModSlot : WeaponDef->WeaponModSlots) {
        Entry->WeaponModSlots.Add(ModSlot);
    }
    for (auto Mod : Mods) {
        __int64 v9;
        sub_13DC400(&v9, Mod.ModDefinition);
        sub_9BA1D68(Entry, &v9);
    }
}

inline void UpdateBaseWeaponAmmo(FFortItemEntry* Entry)
{
        for (auto Mod : Entry->WeaponModSlots)
        {
                if (Mod.WeaponMod->IsA(UFortWeaponModItemDefinitionMagazine::StaticClass()))
                {
                        auto tag = Mod.WeaponMod->WeaponModTag;

                        auto result = Mod.WeaponMod->WeaponModDataTable->Search([&tag](FName& key, uint8*& value)
                        {
                                return key.ToString().contains(tag.TagName.ToString().c_str());
                        });

                        if (result)
                        {
                                auto ModInfo = (FFortWeaponModDataRow*)(*result);
                                Entry->LoadedAmmo = Entry->LoadedAmmo * ModInfo->WeaponAmmoClipSize;
                        }
                }
        }
}

inline FVector GetSafeNormal(FVector v)
{
    float sizeSq = v.X * v.X + v.Y * v.Y + v.Z * v.Z;
    if (sizeSq > KINDA_SMALL_NUMBER)
    {
        float inv = 1.f / UKismetMathLibrary::sqrt(sizeSq);
        return FVector(v.X * inv, v.Y * inv, v.Z * inv);
    }
    return FVector(0.f, 0.f, 0.f);
}

template<class InterfaceType>
TScriptInterface<InterfaceType> CreateScriptInterface(UObject* Object)
{
    TScriptInterface<InterfaceType> ScriptInterface{};
    ScriptInterface.ObjectPointer = Object;
    ScriptInterface.InterfacePointer = Object->GetInterfaceAddress<InterfaceType>();
    return ScriptInterface;
}

inline FRotator VectorToRotatorManual(FVector DirectionVector)
{
    float Pitch = RadiansToDegrees(UKismetMathLibrary::asin(DirectionVector.Z));

    float Yaw = RadiansToDegrees(UKismetMathLibrary::Atan2(DirectionVector.Y, DirectionVector.X));

    return FRotator(Pitch, Yaw, 0.0f);
}
constexpr bool bLateGame = true;
constexpr bool bUseExperimentalDamaging = true;
constexpr bool bEnableLWM = false;
constexpr float WarmupDuration = 500.f;
constexpr float LateGameWarmupDuration = 50.f;
inline EGameModeType CurrentGamemode = EGameModeType::Athena;
#endif //PCH_H
