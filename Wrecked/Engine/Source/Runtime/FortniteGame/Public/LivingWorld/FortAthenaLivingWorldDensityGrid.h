#pragma once
#include "pch.h"
#include "SDK/SDK/GameplayTags_classes.hpp"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

namespace FortAthenaLivingWorldDensityGrid
{
    void Hook();
    DefineOriginal(char, AddActor, FFortAthenaLivingWorldDensityGrid* DensityGrid, AActor* InActor, double DensityValue, double DensityRange);
    static void (*InitializeGrid)(FFortAthenaLivingWorldDensityGrid* Grid, const TBox<double>* InGridBounds, int InGridCellSizeX, int InGridCellSizeY, int InGridCellSizeZ) = decltype(InitializeGrid)(ImageBase + 0xB68BA48);
    static void (*GetActorBounds)(FFortAthenaLivingWorldDensityGrid* Grid, const TBox<double>* result, AActor *Actor) = decltype(GetActorBounds)(ImageBase + 0xB6878E8); // if that doesnt work then we will do a manual way 

}
