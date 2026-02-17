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

void AthenaSpecialActorComponent::Activate(UAthenaSpecialActorComponent* SpecialActorComponent, bool bReset)
{
    
    UE_LOG(LogTemp, Log, "UAthenaSpecialActorComponent::Active Called");
    FName SpecialActorID = SpecialActorComponent->SpecialActorID;

    FSpecialActorRepData NewRepData = {};

    NewRepData.SpecialActorID = SpecialActorID;
    NewRepData.SpecialActor = SpecialActorComponent->GetOwner();
    NewRepData.CategoryTag = SpecialActorComponent->SpecialActorTag;
    NewRepData.PlayerState = nullptr;
    NewRepData.ReplicationInterval = 0.1f;
    NewRepData.ReplicationIntervalDeviation = 0.05f;
    NewRepData.NextReplicationTime = GetWorld()->GetTimeSeconds();

    NewRepData.bDrawCompassIcon = SpecialActorComponent->bUseCompassIcon;
    NewRepData.bDrawCompassDistanceText = SpecialActorComponent->bDrawCompassDistanceText;
    NewRepData.bDrawRotated = SpecialActorComponent->bDrawRotated;
    NewRepData.YawOffset = SpecialActorComponent->YawOffset;
    NewRepData.ElevationArrowData = SpecialActorComponent->ElevationArrowData;
    NewRepData.RenderOnMapDuringPhases = SpecialActorComponent->RenderOnMapDuringPhases;
    NewRepData.DrawLayer = SpecialActorComponent->DrawLayer;
    NewRepData.SeasonItemTag = SpecialActorComponent->SeasonItemTag;
    NewRepData.Priority = SpecialActorComponent->Priority;
    NewRepData.RenderDistance = SpecialActorComponent->RenderDistance.Evaluate();
    NewRepData.RenderTags = SpecialActorComponent->RenderTags;

    auto BrushIdx = RegisterSlateBrush(SpecialActorComponent->SpecialActorMinimapIconBrush);
    auto Sclidx = RegisterBrushScale(SpecialActorComponent->SpecialActorMinimapIconScale);
    NewRepData.MinimapIconBrushIndex = BrushIdx;
    NewRepData.MinimapScaleIndex = Sclidx;
    NewRepData.FullScreenMapIconBrushIndex = BrushIdx;
    NewRepData.FullScreenMapScaleIndex = Sclidx;
    NewRepData.CompassIconBrushIndex = BrushIdx;
    NewRepData.CompassScaleIndex = Sclidx;

    FSpecialActorTransform InitialTransform;
    InitialTransform.Location = (FVector_NetQuantize)SpecialActorComponent->GetOwner()->K2_GetActorLocation();
    InitialTransform.Yaw = SpecialActorComponent->GetOwner()->K2_GetActorRotation().Yaw;
    GetGameState()->SpecialActorData->SpecialActorTransforms.Add(InitialTransform);
    NewRepData.ReplicatedTransformIndex = GetGameState()->SpecialActorData->SpecialActorTransforms.Num() - 1;

    GetGameState()->SpecialActorData->SpecialActorRepList.SpecialActorList.Add(NewRepData);
    GetGameState()->SpecialActorData->SpecialActorRepList.MarkArrayDirty();
    GetGameState()->OnRep_SpecialActorData();
    ActivateOG(SpecialActorComponent, bReset);
}


void AthenaSpecialActorComponent::Init() {
    UHook* Hook = new UHook();

    Hook->Address = 0x2c0/8;
    Hook->Class = UAthenaSpecialActorComponent::StaticClass();
    Hook->Detour = Activate;
    Hook->Original = (void**)&ActivateOG;
    //UKismetHookingLibrary::Hook(Hook, EveryVFT); 

    delete Hook;
}