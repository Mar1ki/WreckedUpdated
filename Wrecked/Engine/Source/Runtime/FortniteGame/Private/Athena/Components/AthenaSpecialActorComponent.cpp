#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/AthenaSpecialActorComponent.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

int8 RegisterSlateBrush(const FSlateBrush& Brush)
{
    for (int32 i = 0; i < GetGameState()->SpecialActorData->SlateBrushes.Num(); i++)
    {
        if (GetGameState()->SpecialActorData->SlateBrushes[i] == Brush)
        {
            return i;
        }
    }

    GetGameState()->SpecialActorData->SlateBrushes.Add(Brush);
    return GetGameState()->SpecialActorData->SlateBrushes.Num() - 1;
}

int8 RegisterBrushScale(const FVector2D& Scale)
{
    for (int32 i = 0; i < GetGameState()->SpecialActorData->BrushScales.Num(); i++)
    {
        if (GetGameState()->SpecialActorData->BrushScales[i] == Scale)
        {
            return i;
        }
    }

    GetGameState()->SpecialActorData->BrushScales.Add(Scale);
    return GetGameState()->SpecialActorData->BrushScales.Num() - 1;
}

void AthenaSpecialActorComponent::Activate(UAthenaSpecialActorComponent* Component, bool bReset)
{
    UE_LOG(LogTemp, Log, "UAthenaSpecialActorComponent::Active Called");

    bool bShouldSkip = false;
    
    if (!Component->SpecialActorTag.TagName)
    {
        UE_LOG(LogTemp, Warning, "SpecialActorTag.TagName is null for component %c",*Component->GetName().c_str());
        bShouldSkip = true;
    }
    
    if (Component->SpecialActorMinimapIconScale.IsZero())
    {
        UE_LOG(LogTemp, Warning, "SpecialActorMinimapIconScale is null for component %s", Component->GetName().c_str());
        bShouldSkip = true;
    }
    
    if (Component->SpecialActorFullScreenMapIconScale.IsZero())
    {
        UE_LOG(LogTemp, Warning, "SpecialActorFullScreenMapIconScale is null for component %s", Component->GetName().c_str());
        bShouldSkip = true;
    }
    
    if (!Component->SpecialActorCompassIconBrush.ResourceObject)
    {
        UE_LOG(LogTemp, Warning, "SpecialActorCompassIconBrush.ResourceObject is null for component %s", Component->GetName().c_str());
        bShouldSkip = true;
    }
    
    if (Component->SpecialActorCompassIconScale.IsZero())
    {
        UE_LOG(LogTemp, Warning, "SpecialActorCompassIconScale is null for component %s", Component->GetName().c_str());
        bShouldSkip = true;
    }

    if (bShouldSkip)
    {
        UE_LOG(LogTemp, Log, "SpecialActorComponent is ass so we dont process it %s", Component->GetName().c_str());
        return ActivateOG(Component, bReset);
    }
    FSpecialActorRepData NewRepData = {};

    NewRepData.SpecialActorID = Component->SpecialActorID;
    NewRepData.SpecialActor = Component->GetOwner();
    NewRepData.CategoryTag = Component->SpecialActorTag;
    NewRepData.PlayerState = nullptr;
    NewRepData.ReplicationInterval = 0.1f;
    NewRepData.ReplicationIntervalDeviation = 0.05f;
    NewRepData.NextReplicationTime = GetWorld()->GetTimeSeconds();

    NewRepData.bDrawCompassIcon = Component->bUseCompassIcon;
    NewRepData.bDrawCompassDistanceText = Component->bDrawCompassDistanceText;
    NewRepData.bDrawRotated = Component->bDrawRotated;
    NewRepData.YawOffset = Component->YawOffset;
    NewRepData.ElevationArrowData = Component->ElevationArrowData;
    NewRepData.RenderOnMapDuringPhases = Component->RenderOnMapDuringPhases;
    NewRepData.DrawLayer = Component->DrawLayer;
    NewRepData.SeasonItemTag = Component->SeasonItemTag;
    NewRepData.Priority = Component->Priority;
    NewRepData.RenderDistance = Component->RenderDistance.Evaluate();
    NewRepData.RenderTags = Component->RenderTags;

    auto BrushIdx = RegisterSlateBrush(Component->SpecialActorMinimapIconBrush);
    auto Sclidx = RegisterBrushScale(Component->SpecialActorMinimapIconScale);
    NewRepData.MinimapIconBrushIndex = BrushIdx;
    NewRepData.MinimapScaleIndex = Sclidx;
    NewRepData.FullScreenMapIconBrushIndex = BrushIdx;
    NewRepData.FullScreenMapScaleIndex = Sclidx;
    NewRepData.CompassIconBrushIndex = BrushIdx;
    NewRepData.CompassScaleIndex = Sclidx;

    FSpecialActorTransform InitialTransform;
    InitialTransform.Location = (FVector_NetQuantize)Component->GetOwner()->K2_GetActorLocation();
    InitialTransform.Yaw = Component->GetOwner()->K2_GetActorRotation().Yaw;

    AddActorToReplicationList(GetGameState()->SpecialActorData, &NewRepData);

   // ActivateOG(Component, bReset);
}


void AthenaSpecialActorComponent::Init() {
    UHook* Hook = new UHook();

    Hook->Address = 0x2c0/8;
    Hook->Class = UAthenaSpecialActorComponent::StaticClass();
    Hook->Detour = Activate;
    Hook->Original = (void**)&ActivateOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT); 

    delete Hook;
}