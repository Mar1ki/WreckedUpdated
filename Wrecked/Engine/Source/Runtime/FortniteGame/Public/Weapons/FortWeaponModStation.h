#pragma once
#include "pch.h"

namespace FortWeaponModStation {


    DefineOriginal(__int64, ServerStopInteractWithWorkbenchActor, UFortWeaponModStationComponent* Component, AFortWeaponModStationBase* ModStation);
    DefineOriginal(__int64, ServerPurchaseRemoveModImplementation, UFortWeaponModStationComponent* Component, UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon);

    
    void Init();
}