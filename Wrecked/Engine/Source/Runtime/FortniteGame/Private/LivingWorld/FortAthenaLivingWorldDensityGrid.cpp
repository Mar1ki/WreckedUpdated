#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldDensityGrid.h"

char FortAthenaLivingWorldDensityGrid::AddActor(FFortAthenaLivingWorldDensityGrid* DensityGrid, AActor* InActor, double DensityValue, double DensityRange)
{
    auto WorldConfig = GetLivingWorldManager()->GetConfig();
    
    if (DensityGrid->GridBounds.Max.X == 0)
    {
        TBox<double> ActorBounds;
        GetActorBounds(DensityGrid, &ActorBounds, InActor);
        
        // TODO: Get proper grid bounds size
     //   DensityGrid->GridBounds.Min= { -150000.0, -150000.0, -50000.0 };
      //  DensityGrid->GridBounds.Max= {  150000.0,  150000.0,  50000.0 };

        DensityGrid->GridBounds = ActorBounds;
        
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