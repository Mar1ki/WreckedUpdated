#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldStaticPointProvider.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

void FortAthenaLivingWorldStaticPointProvider::RegisterToLivingWorldManager(IFortAthenaLivingWorldPointProviderInterface* PointProviderInterface, AFortAthenaLivingWorldStaticPointProvider* PointProvider)
{
    auto GameState = GetGameState();
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GameState->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass());

    if (!LivingWorldManager)
    {
        UE_LOG(LogLivingWorldManager, Warning, "%s - failed to register loaded PP because there is no LWM", PointProvider->Name.ToString().c_str());
    }

    // i think this has more sense  or idk
    PointProvider->bStartEnabled = true; // sets up some shit
    TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> NewPoint{};
    NewPoint.ObjectPointer = PointProvider;
    NewPoint.InterfacePointer = PointProviderInterface;

    auto Enabler = GetLivingWorldPointProviderEnabler();
    Enabler->AvailableLinkedPointProviders.Add(NewPoint);
    
    LivingWorldManager->PointProviders.Add(NewPoint);
    LivingWorldManager->RegisteredPointProvidersCount++;
    UE_LOG(LogLivingWorldManager, Verbose, "Registered a new Point Provider %s (%s)", PointProvider->GetName().c_str(), PointProvider->Class->GetName().c_str());
}



// not used right now move it later to the manager
void FortAthenaLivingWorldStaticPointProvider::LivingWorldManagerDisablePointProvider(AActor* PointProvider)
{
    auto CastedPointProvider = (AFortAthenaLivingWorldStaticPointProvider*)PointProvider;
    CastedPointProvider->bStartEnabled = false;
}

void FortAthenaLivingWorldStaticPointProvider::LivingWorldManagerEnablePointProvider(AActor* PointProvider)
{
    auto CastedPointProvider = (AFortAthenaLivingWorldStaticPointProvider*)PointProvider;
    CastedPointProvider->bStartEnabled = true;
}


TArray<AFortAthenaLivingWorldStaticPointProvider*> FortAthenaLivingWorldStaticPointProvider::GetPointProvidersFromTagQuery(const FGameplayTag& SearchTag)
{
    TArray<AFortAthenaLivingWorldStaticPointProvider*> FoundPoints;
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass());
    for (auto Point : LivingWorldManager->PointProviders)
    {
        auto PointProvider = (AFortAthenaLivingWorldStaticPointProvider*)Point.ObjectPointer;
        if (PointProvider->FiltersTags.HasTag(SearchTag))
        {
            FoundPoints.Add(PointProvider);
        }
    }
    return FoundPoints;
}


AFortAthenaLivingWorldStaticPointProvider* FortAthenaLivingWorldStaticPointProvider::GetRandomPointProviderForEventSpawn(TArray<AFortAthenaLivingWorldStaticPointProvider*> InArray)
{
    TArray<AFortAthenaLivingWorldStaticPointProvider*> RandArray;   

    for (auto PointProvider : InArray)
    {
        if (PointProvider && PointProvider->bStartEnabled)
        {
            RandArray.Add(PointProvider);
        }
    }
    if (RandArray.IsEmpty())
    {
        return nullptr;
    }

    return RandArray[FMath::RandHelper(RandArray.Num())];
}

void FortAthenaLivingWorldStaticPointProvider::EnablePointProvider(UObject* Object, FFrame& Params)
{
    auto Enabler = GetLivingWorldPointProviderEnabler();
    auto PointProvider = (AFortAthenaPatrolPathPointProvider*)Object;

    TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> PointProviderInterfaceScript{};
    PointProviderInterfaceScript.ObjectPointer = PointProvider;
    PointProviderInterfaceScript.InterfacePointer = PointProvider->GetInterfaceAddress<IFortAthenaLivingWorldPointProviderInterface>();
    
    Enabler->AvailableLinkedPointProviders.Add(PointProviderInterfaceScript);

    PointProvider->bStartEnabled = true;
    UE_LOG(LogLivingWorldStaticPointProvider, Log, "Enabling PointProvider");
    
    EnablePointProviderOG(Object, Params);
}

void FortAthenaLivingWorldStaticPointProvider::DisablePointProvider(UObject* Object, FFrame& Params)
{
    auto Enabler = GetLivingWorldPointProviderEnabler();
    auto PointProvider = (AFortAthenaPatrolPathPointProvider*)Object;
    
    TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> PointProviderInterfaceScript{};
    PointProviderInterfaceScript.ObjectPointer = PointProvider;
    PointProviderInterfaceScript.InterfacePointer = PointProvider->GetInterfaceAddress<IFortAthenaLivingWorldPointProviderInterface>();
    
    Enabler->LinkedPointProviders.Add(PointProviderInterfaceScript);
    PointProvider->bStartEnabled = false;
    UE_LOG(LogLivingWorldStaticPointProvider, Log, "Disabling PointProvider");
    
    DisablePointProviderOG(Object, Params);
}

void FortAthenaLivingWorldStaticPointProvider::Init()
{
    UHook* Hook = new UHook();
    
    Hook->Address = ImageBase + 0xB6B7E64;
    Hook->Detour = RegisterToLivingWorldManager;
    UKismetHookingLibrary::Hook(Hook, Address);
    
    Hook->Path = "/Script/LagerRuntime.FortAthenaPatrolPathPointProvider.EnablePointProvider";
    Hook->Detour = EnablePointProvider;
    Hook->Original = (void**)&EnablePointProviderOG;
    UKismetHookingLibrary::Hook(Hook, Exec);

    Hook->Path = "/Script/LagerRuntime.FortAthenaPatrolPathPointProvider.DisablePointProvider";
    Hook->Detour = DisablePointProvider;
    Hook->Original = (void**)&EnablePointProviderOG;
    UKismetHookingLibrary::Hook(Hook, Exec);

    
    Hook->Path = "/Script/LagerRuntime.FortAthenaLivingWorldStaticPointProvider.DisablePointProvider";
    Hook->Detour = DisablePointProvider;
    Hook->Original = (void**)&DisablePointProviderOG;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    Hook->Path = "/Script/LagerRuntime.FortAthenaLivingWorldStaticPointProvider.EnablePointProvider";
    Hook->Detour = EnablePointProvider;
    Hook->Original = (void**)&DisablePointProviderOG;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    delete Hook;
}
