#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortPlayerController
{
    DefineOriginal(void, ServerAcknowledgePossession, AFortPlayerControllerAthena* Controller, AFortPawn* Pawn);
    DefineOriginal(void, ServerExecuteInventoryItem, AFortPlayerController* Controller, FGuid ItemGuid);
    void ServerCreateBuildingActor(AFortPlayerControllerAthena *Controller, FCreateBuildingActorData &CreateBuildingData);
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
    bool RemoveInventoryItem(IFortInventoryOwnerInterface* Interface, FGuid* ItemGuid, int Count);
    void Init();
}