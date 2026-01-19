#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaMapInfo.h"
#include <random>

int AFortAthenaMapInfo::GetSafeZoneCount() {
    static int(__fastcall * GetSafeZoneCountOG)(AFortAthenaMapInfo* mapinfo) = decltype(GetSafeZoneCountOG)(ImageBase + 0x4036410);
    return GetSafeZoneCountOG(this);
}

void AFortAthenaMapInfo::GenerateSafeZoneLocations(TArray<FVector>& OutLocations, TArray<FVector4>& BlacklistLocations, FVector* CenterLocation, int CurrentSafeZoneIndex, bool bForceLastSafeZoneToCenterLocation)
{
    int SafeZoneCount = SafeZoneDefinition.Count.Evaluate();
    if (SafeZoneCount <= 0) return;
    OutLocations.Free();
    OutLocations.ResizeTo(SafeZoneCount);
    OutLocations.SetNumElements(SafeZoneCount);
    if (CenterLocation)
    {
       OutLocations[0] = *CenterLocation;
    }

    int LastIndex = SafeZoneCount - 1;
    bool bWasLastForced = false;

    for (int i = LastIndex; i > CurrentSafeZoneIndex; --i)
    {
        bool bIsFinalPhase = (i == LastIndex);
        int NextIndex = bIsFinalPhase ? 0 : i + 1;
        FVector NextLocation = OutLocations[NextIndex];
        FSafeZoneLocationParams Params = ConstructSafeZoneLocationParams(i, *CenterLocation, NextLocation, bIsFinalPhase, CurrentSafeZoneIndex);
        
        FVector GeneratedResult;

        if (bForceLastSafeZoneToCenterLocation && !bWasLastForced)
        {
            Params.ForceDistanceMin = 0.0f;
            Params.ForceDistanceMax = 0.0f;

            GeneratedResult = *CenterLocation;
            bWasLastForced = true;
        }
        else
        {
            GeneratedResult = PickSafeZoneLocation(Params, BlacklistLocations);
        }
        OutLocations[i] = GeneratedResult;
    }
}