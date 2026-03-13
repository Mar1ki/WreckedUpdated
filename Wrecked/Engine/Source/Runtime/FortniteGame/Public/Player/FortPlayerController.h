#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortPlayerController
{
    DefineOriginal(void, ServerAcknowledgePossession, AFortPlayerControllerAthena* Controller, AFortPawn* Pawn);
    DefineOriginal(void, ServerExecuteInventoryItem, AFortPlayerController* Controller, FGuid ItemGuid);
    void ServerCreateBuildingActor(AFortPlayerControllerAthena *PlayerController, FCreateBuildingActorData &CreateBuildingData);
    void ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);
    void ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building, TSubclassOf<ABuildingSMActor> BuildingClass, uint8 Rot, bool Mirrored);
    void ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);
    void ServerAttemptInventoryDrop(AFortPlayerController* Controller, const struct FGuid& ItemGuid, int32 Count, bool bTrash);
    DefineOriginal(void, ServerLoadingScreenDropped, AFortPlayerControllerAthena* PC);
    void ServerRepairBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair);
    void ServerPlayEmoteItem(AFortPlayerController* PlayerController, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber);
    DefineOriginal(void, ServerReadyToStartMatch, AFortPlayerControllerAthena* PlayerController);
    void MovingEmoteStopped(UObject* Context, FFrame& Stack);
    void ServerCheat(AFortPlayerControllerAthena* Controller, FString Msg);
    DefineOriginal(void, ClientOnPawnDied, AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport& DeathReport);
    void Init();
}

inline void AFortPlayerController::PayBuildableClassPlacementCost(FBuildingClassData* Data)
{
    static void* (*PayBuildableClassPlacementCostInternal)(AFortPlayerController*, FBuildingClassData*) = decltype(PayBuildableClassPlacementCostInternal)(ImageBase + 0xA202570);
    PayBuildableClassPlacementCostInternal(this, Data);
}

inline bool AFortPlayerController::CanAffordToPlaceBuildableClass(FBuildingClassData* Data)
{
    static bool* (*CanAffordToPlaceBuildableClassInternal)(AFortPlayerController*, FBuildingClassData*) = decltype(CanAffordToPlaceBuildableClassInternal)(ImageBase + 0xA1CABF8);
    return CanAffordToPlaceBuildableClassInternal(this, Data);
}
inline void AFortPlayerController::PayBuildingRepairCost(ABuildingSMActor* BuildingActor)
{
    static void* (*PayBuildingRepairCostInternal)(AFortPlayerController*, ABuildingSMActor*) = decltype(PayBuildingRepairCostInternal)(ImageBase + 0xA202738);
    PayBuildingRepairCostInternal(this, BuildingActor);
}