#pragma once
#include "pch.h"

namespace FortAthenaMapInfo
{
    extern std::vector<FVector> StormCircles;

    void PrepareStormCircles(AFortAthenaMapInfo* MapInfo, int32 Seed, const TArray<float>& Radii);
}