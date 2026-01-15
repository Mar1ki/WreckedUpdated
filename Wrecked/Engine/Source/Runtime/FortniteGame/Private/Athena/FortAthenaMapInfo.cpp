#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaMapInfo.h"
#include <random>

namespace FortAthenaMapInfo { std::vector<FVector> StormCircles; }

void FortAthenaMapInfo::PrepareStormCircles(AFortAthenaMapInfo* MapInfo, int32 Seed, const TArray<float>& Radii)
{
    StormCircles.clear();
    StormCircles.reserve(Radii.Num());

    std::mt19937 RNG(Seed);
    std::uniform_real_distribution<float> Rand(0.f, 1.f);

    const FBoxSphereBounds& Bounds = MapInfo->CachedPlayableBoundsForClients;

    StormCircles.push_back(MapInfo->GetMapCenter());

    float Angle = Rand(RNG) * 2.f * PI;
    FVector2D Dir(cos(Angle), sin(Angle));

    int32 ZoneCount = Radii.Num();
        
    for (int i = 1; i < 5 && i < ZoneCount; ++i)
    {
        float BaseAngle = atan2(Dir.Y, Dir.X);
        Angle = BaseAngle + (Rand(RNG) - 0.5f) * (PI / 36.f);
        Dir = FVector2D(cos(Angle), sin(Angle));
            
        float SizeSq = Dir.X * Dir.X + Dir.Y * Dir.Y;
        if (SizeSq > 1.e-4f)
        {
            float Inv = 1.f / FMath::Sqrt(SizeSq);
            Dir = FVector2D(Dir.X * Inv, Dir.Y * Inv);
        }

        float Offset = (0.4f + Rand(RNG) * 0.5f) * (Radii[i - 1] - Radii[i]);
        FVector NewCenter = StormCircles[i - 1] + FVector(Dir.X, Dir.Y, 0.f) * Offset;

        StormCircles.push_back(NewCenter);
    }

    for (int i = 5; i < 8 && i < ZoneCount; ++i)
    {
        Angle = Rand(RNG) * 2.f * PI;
        FVector2D RandDir(cos(Angle), sin(Angle));
        float Dist = sqrt(Radii[i - 1] * Radii[i - 1] - Radii[i] * Radii[i]);

        FVector NewCenter = StormCircles[i - 1] + FVector(RandDir.X, RandDir.Y, 0.f) * Dist;
        NewCenter.X = FMath::Clamp(NewCenter.X, Bounds.Origin.X - Bounds.BoxExtent.X + Radii[i], Bounds.Origin.X + Bounds.BoxExtent.X - Radii[i]);
        NewCenter.Y = FMath::Clamp(NewCenter.Y, Bounds.Origin.Y - Bounds.BoxExtent.Y + Radii[i], Bounds.Origin.Y + Bounds.BoxExtent.Y - Radii[i]);

        StormCircles.push_back(NewCenter);
    }

    if (ZoneCount > 7)
    {
        FVector RefCenter = StormCircles[6];
        float RefRadius = Radii[6];

        for (int i = 8; i < ZoneCount; ++i)
        {
            Angle = Rand(RNG) * 2.f * PI;
            float Dist = Rand(RNG) * RefRadius * 0.8f;
            FVector2D RandDir(cos(Angle), sin(Angle));

            FVector NewCenter = RefCenter + FVector(RandDir.X, RandDir.Y, 0.f) * Dist;
            NewCenter.X = FMath::Clamp(NewCenter.X, Bounds.Origin.X - Bounds.BoxExtent.X + Radii[i], Bounds.Origin.X + Bounds.BoxExtent.X - Radii[i]);
            NewCenter.Y = FMath::Clamp(NewCenter.Y, Bounds.Origin.Y - Bounds.BoxExtent.Y + Radii[i], Bounds.Origin.Y + Bounds.BoxExtent.Y - Radii[i]);

            StormCircles.push_back(NewCenter);
        }
    }
}
