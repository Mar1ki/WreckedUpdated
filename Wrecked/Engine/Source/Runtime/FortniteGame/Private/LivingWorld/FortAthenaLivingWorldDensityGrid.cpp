#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldDensityGrid.h"

char FortAthenaLivingWorldDensityGrid::AddActor(FFortAthenaLivingWorldDensityGrid* DensityGrid, AActor* InActor, double DensityValue, double DensityRange)
{
    if (DensityGrid->GridCellSize.X == 0 || !DensityGrid->GridBounds.IsValid)
    {
        // grid bounds and grid cell size isnt set so um im setting it to random ass values rn but it should work if u find a better way to make it then do it
        DensityGrid->GridBounds.Min= { -150000.0, -150000.0, -50000.0 };
        DensityGrid->GridBounds.Max= {  150000.0,  150000.0,  50000.0 };
        DensityGrid->GridBounds.IsValid = 1;
        DensityGrid->GridCellSize= { 200.0, 200.0, 200.0 };
        
        InitializeGrid(DensityGrid, &DensityGrid->GridBounds, (int)DensityGrid->GridCellSize.X, (int)DensityGrid->GridCellSize.Y, (int)DensityGrid->GridCellSize.Z);
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