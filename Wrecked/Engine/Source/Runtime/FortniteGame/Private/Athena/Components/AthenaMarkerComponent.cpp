#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/AthenaMarkerComponent.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

void AthenaMarkerComponent::ServerAddMapMarker(UObject* Context, FFrame& Stack)
{
    auto& MarkerRequest = Stack.StepCompiledInRef<FFortClientMarkerRequest>();
    Stack.IncrementCode();

    auto MarkerComponent = (UAthenaMarkerComponent*)Context;
    auto PlayerController = (AFortPlayerControllerAthena*)MarkerComponent->GetOwner();
 
    if (PlayerController == nullptr)
        return;
 
    auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
 
    if (PlayerState == nullptr)
        return;
 
    FFortWorldMarkerData MarkerData = FFortWorldMarkerData();
 
    FMarkerID MarkerID{};
    MarkerID.PlayerId = PlayerState->PlayerId;
    MarkerID.InstanceID = MarkerRequest.InstanceID;
 
    MarkerData.MarkerID = MarkerID;
    MarkerData.Owner = PlayerState;
    MarkerData.MarkerType = MarkerRequest.MarkerType;
 
    MarkerData.BasePosition = MarkerRequest.BasePosition;
    MarkerData.BasePositionOffset = MarkerRequest.BasePositionOffset;
    MarkerData.WorldNormal = MarkerRequest.WorldNormal;
 
    if (MarkerRequest.MarkedActor)
    {
        BuildMarkerDataForActor(MarkerComponent, MarkerRequest.MarkedActor, &MarkerData);
 
        if (auto Pickup = Cast<AFortPickup>(MarkerRequest.MarkedActor))
        {
            MarkerData.ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
            MarkerData.ItemCount = Pickup->PrimaryPickupItemEntry.Count;
        }
    }
 
    MarkerData.bIncludeSquad = MarkerRequest.bIncludeSquad;
    MarkerData.bUseHoveredMarkerDetail = MarkerRequest.bUseHoveredMarkerDetail;
 
    for (auto TeamMember : PlayerState->PlayerTeam->TeamMembers)
    {
        auto TeamMemberAthena = (AFortPlayerControllerAthena*)TeamMember;
 
        if (TeamMember == nullptr || TeamMemberAthena == PlayerController)
            continue;
 
        auto TeamMemberMarkerComponent = TeamMemberAthena->MarkerComponent;
 
        if (TeamMemberMarkerComponent == nullptr)
            return;

        LocalPlaceMapMarker(TeamMemberMarkerComponent, &MarkerData);
 
        TeamMemberMarkerComponent->MarkerStream.MarkArrayDirty();
    }
}

void AthenaMarkerComponent::ServerRemoveMapMarker(UObject* Context, FFrame& Stack)
{
    ECancelMarkerReason CancelReason;
    auto& MarkerID = Stack.StepCompiledInRef<FMarkerID>();
    Stack.StepCompiledIn(&CancelReason);
    Stack.IncrementCode();

    auto MarkerComponent = (UAthenaMarkerComponent*)Context;
    
    auto PlayerController = (AFortPlayerControllerAthena*)MarkerComponent->GetOwner();
 
    if (PlayerController == nullptr)
        return;
 
    auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
 
    if (PlayerState == nullptr)
        return;
 
    for (auto TeamMember : PlayerState->PlayerTeam->TeamMembers)
    {
        auto TeamMemberAthena = (AFortPlayerControllerAthena*)TeamMember;
 
        if (TeamMember == nullptr || TeamMemberAthena == PlayerController)
            continue;
        auto TeamMemberMarkerComponent = TeamMemberAthena->MarkerComponent;
 
        if (TeamMemberMarkerComponent == nullptr)
            continue;
 
        UFortWorldMarker* WorldMarker = nullptr;
 
        for (FFortWorldMarkerData& Marker : TeamMemberMarkerComponent->MarkerStream.Markers)
        {
            if (Marker.MarkerID.PlayerId == MarkerID.PlayerId && Marker.MarkerID.InstanceID == MarkerID.InstanceID)
            {
                WorldMarker = Marker.MarkerInstance;
                break;
            }
        }
 
        if (WorldMarker == nullptr)
            continue;
 
       WorldMarker->CancelMarker();
 
        for (int32 i = 0; i < TeamMemberMarkerComponent->MarkerStream.Markers.Num(); ++i)
        {
            auto MarkerInstance = TeamMemberMarkerComponent->MarkerStream.Markers[i].MarkerInstance;
 
            if (MarkerInstance == WorldMarker)
            {
                TeamMemberMarkerComponent->MarkerStream.Markers.Remove(i);
                TeamMemberMarkerComponent->MarkerStream.MarkArrayDirty();
                break;
            }
        }
    }
}
void AthenaMarkerComponent::Init() {
    UHook* Hook = new UHook();
    
    Hook->Path = "/Script/FortniteGame.AthenaMarkerComponent.ServerAddMapMarker";
    Hook->Detour = ServerAddMapMarker;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    Hook->Path = "/Script/FortniteGame.AthenaMarkerComponent.ServerRemoveMapMarker";
    Hook->Detour = ServerRemoveMapMarker;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    delete Hook;
}