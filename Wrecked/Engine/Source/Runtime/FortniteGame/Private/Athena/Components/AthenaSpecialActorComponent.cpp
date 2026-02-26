#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/AthenaSpecialActorComponent.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

struct FSpecialActorInitData
{
public:
	AActor* SpecialActorRef;
	FName UniqueID;
	FGameplayTag CategoryTag;
	APlayerState* PlayerState;
	float RepInterval;
	float RepIntervalDev;
	FSlateBrush MinimapIconBrush;
	FVector2D MinimapScale;
	FSlateBrush FullScreenMapIconBrush;
	FVector2D FullScreenMapScale;
	FSlateBrush CompassIconBrush;
	FVector2D CompassScale;
	bool bDrawCompassIcon;
	bool bDrawCompassDistanceText;
	unsigned __int8 RenderOnMapDuringPhases;
	bool bReplicateHealth;
	bool bReplicateShield;
	FText DisplayName;
	float RenderDistance;
	uint8   Pad_1[0x4];
	FGameplayTagContainer RenderTags;
	FGameplayTag SeasonItemTag;
	uint8   Pad_2[0x4];
	FSlateBrush SeasonItemInteractedBrush;
	FElevationArrowData ElevationArrowData;
	uint8   Pad_3[0x4];
	TArray<FSpecialActorSingleStatData> StatsToTrack;
};

void AthenaSpecialActorComponent::Activate(UAthenaSpecialActorComponent* Component, bool bReset)
{
    if (!Component || !Component->GetOwner())
    {
        return;
    }

    FSpecialActorInitData InitData;
    InitData.SpecialActorRef = Component->GetOwner();
    InitData.UniqueID = Component->SpecialActorID;
    InitData.CategoryTag = Component->SpecialActorTag;
    auto* OwnerPawn = Cast<AFortPawn>(Component->GetOwner());
    InitData.PlayerState = OwnerPawn ? OwnerPawn->PlayerState : nullptr;
    InitData.MinimapIconBrush = Component->SpecialActorMinimapIconBrush;
    InitData.MinimapScale = Component->SpecialActorMinimapIconScale;
    InitData.FullScreenMapIconBrush = Component->SpecialActorFullScreenMapIconBrush;
    InitData.FullScreenMapScale = Component->SpecialActorFullScreenMapIconScale;
    InitData.CompassIconBrush = Component->SpecialActorCompassIconBrush;
    InitData.CompassScale = Component->SpecialActorCompassIconScale;
    InitData.bDrawCompassIcon = Component->bUseCompassIcon;
    InitData.bDrawCompassDistanceText = Component->bDrawCompassDistanceText;
    InitData.RenderOnMapDuringPhases = Component->RenderOnMapDuringPhases;
    InitData.RenderDistance = Component->RenderDistance.Evaluate();
    InitData.RenderTags = Component->RenderTags;
    InitData.SeasonItemTag = Component->SeasonItemTag;
    InitData.SeasonItemInteractedBrush = Component->SeasonItemInteractedBrush;
    InitData.ElevationArrowData = Component->ElevationArrowData;
    static __int64 (*AddActorToReplicationList)(AFortSpecialActorReplicationInfo*, FSpecialActorInitData*) = decltype(AddActorToReplicationList)(ImageBase + 0x9A7BF4C);
    AddActorToReplicationList(GetGameState()->SpecialActorData, &InitData);
    ActivateOG(Component, bReset);
}

bool AFortPickup::RegisterSpecialActor() {
    static bool (*RegisterSpecialActorOG)(AFortPickup*) = decltype(RegisterSpecialActorOG)(ImageBase + 0x9BB5B78);
    return RegisterSpecialActorOG(this);
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