#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldDensityGrid.h"

char FortAthenaLivingWorldDensityGrid::AddActor(FFortAthenaLivingWorldDensityGrid* DensityGrid, AActor* InActor, double DensityValue, double DensityRange)
{
    if (!DensityGrid->bIsInitialized)
    {
        InitializeGrid(DensityGrid, &DensityGrid->GridBounds, DensityGrid->GridCellSize.X, DensityGrid->GridCellSize.Y, DensityGrid->GridCellSize.Z);
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