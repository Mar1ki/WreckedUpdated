#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldDensityGrid.h"

char FortAthenaLivingWorldDensityGrid::AddActor(FFortAthenaLivingWorldDensityGrid* DensityGrid, AActor* InActor, double DensityValue, double DensityRange)
{
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass());
    auto WorldConfig = LivingWorldManager->GetConfig();


    /*std::cout << DensityGrid->GridBounds.Max.X << std::endl;
    std::cout << DensityGrid->GridBounds.Max.Y << std::endl;
    std::cout << DensityGrid->GridBounds.Max.Z << std::endl;

    std::cout << "-----------------------------------" << std::endl;
    
    std::cout << DensityGrid->GridBounds.Min.X << std::endl;
    std::cout << DensityGrid->GridBounds.Min.Y << std::endl;
    std::cout << DensityGrid->GridBounds.Min.Z << std::endl */


    
    
    if (DensityGrid->GridCellSize.X == 0)
    {
        // thats the last thing i need to setup rn
        DensityGrid->GridBounds.Min= { -150000.0, -150000.0, -50000.0 };
        DensityGrid->GridBounds.Max= {  150000.0,  150000.0,  50000.0 };
        
        InitializeGrid(DensityGrid, &DensityGrid->GridBounds, (int) WorldConfig->ActorDensityGridCellSize.Value, (int) WorldConfig->ActorDensityGridCellSize.Value, (int) WorldConfig->ActorDensityGridCellSize.Value);
    }

    return AddActorOG(DensityGrid, InActor, DensityValue, DensityRange);
}
void FortAthenaLivingWorldDensityGrid::Hook()
{
    UHook* Hook = new UHook();

    Hook->Address = ImageBase + 0xB680704;
    Hook->Detour = AddActor;
    Hook->Original = (void**)&AddActorOG;
    UKismetHookingLibrary::Hook(Hook, Address);

    delete Hook;
}