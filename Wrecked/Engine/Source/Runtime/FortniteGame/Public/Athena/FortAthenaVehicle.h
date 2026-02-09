#pragma once
#include "pch.h"


namespace FortAthenaVehicle
{
    void SpawnVehicles();
    void Hook();
    UClass* GetRandomMod();
    void ApplyModToVehicle(AFortAthenaVehicle* Vehicle, bool bRandomMod = false, UClass* VehicleMod = nullptr);
    void BreakDamageablePart(AFortAthenaVehicle* Vehicle, const FName* ShapeName);
    DefineOriginal(void, PostInitializeComponents, AFortAthenaVehicle* Vehicle);
}
