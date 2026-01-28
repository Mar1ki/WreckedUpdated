#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldStaticPointProvider.h"

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

    for (auto& Enabler : GetObjectsOfClass<AFortAthenaLivingWorldPointProvidersEnabler>())
    {
        Enabler->LinkedPointProviders.Add(NewPoint);
    }
    
    LivingWorldManager->PointProviders.Add(NewPoint);
    LivingWorldManager->RegisteredPointProvidersCount++;
    UE_LOG(LogLivingWorldManager, Verbose, "Registered a new Point Provider %s (%s)", PointProvider->GetName().c_str(), PointProvider->Class->GetName().c_str());
}




void FortAthenaLivingWorldStaticPointProvider::Init()
{
    UHook* Hook = new UHook();
    
    Hook->Address = ImageBase + 0xB6B7E64;
    Hook->Detour = RegisterToLivingWorldManager;
    UKismetHookingLibrary::Hook(Hook, Address);
    
    delete Hook;
}
