#pragma once
#include "pch.h"
namespace FortVehicleMod {
    UClass* GetRandomVehicleMod();
    void Init();

    static void (__fastcall* ApplyModConfigToVehicleInternal)(UFortVehicleModConfig* ModConfig, IFortVehicleInterface* VehicleInterface) = (decltype(ApplyModConfigToVehicleInternal))(ImageBase + 0xA644268);
    static UFortVehicleModConfig* (__fastcall* GetVehicleModConfigFromTagInternal)(IFortVehicleInterface* Vehicle, FGameplayTag* ModTag) = (decltype(GetVehicleModConfigFromTagInternal))(ImageBase + 0x9AAF080);
    DefineOriginal(void, ServerVehicleModSeatRPC,AFortPlayerControllerAthena* PlayerController,const FGameplayTag& RequestType, UFortVehicleModComponent* ModInstance, int32 SeatIndex);
    bool CanApplyMod(IFortVehicleInterface* VehicleInterface, FGameplayTag& NewModTag);
};

