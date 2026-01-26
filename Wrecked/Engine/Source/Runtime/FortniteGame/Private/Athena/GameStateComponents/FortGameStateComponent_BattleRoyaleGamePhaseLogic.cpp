#include "pch.h"

#include "Engine/Plugins/TaskLibrary/Public/TaskLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortGameModeAthena.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

class UFunction* FindFunctionByFName(UClass* Class, FName FuncName)
{
    for (const UStruct* Clss = Class; Clss; Clss = Clss->Super)
    {
        for (UField* Field = Clss->Children; Field; Field = Field->Next)
        {
            if (Field->HasTypeFlag(EClassCastFlags::Function) && Field->Name == FuncName)
                return static_cast<class UFunction*>(Field);
        }
    }

    return nullptr;
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartAircraftPhase() {
    AFortGameStateAthena* GS = GetGameState();
    const UFortPlaylistAthena* PlaylistData = GS ? GS->CurrentPlaylistInfo.BasePlaylist : nullptr;

    if (!GS || PlaylistData->bSkipAircraft)
    {
        return false;
    }
    //StartSafeZones
    if (GamePhase > EAthenaGamePhase::Warmup)
    {
        return false;
    }

    if (!PlaylistData)
    {
        return false;
    }

    if (PlaylistData->AirCraftBehavior != EAirCraftBehavior::NoAircraft)
    {
        int32 NumAircraftsNeeded = (PlaylistData->AirCraftBehavior == EAirCraftBehavior::OpposingAirCraftForEachTeam) ? GS->TeamCount : 1;
        float ServerTime = GS->GetServerWorldTimeSeconds();

        for (int32 i = 0; i < NumAircraftsNeeded; ++i)
        {
            AFortAthenaAircraft* Aircraft = nullptr;
            if (Aircrafts_GameState.IsValidIndex(i) && Aircrafts_GameState[i].Get())
            {
                Aircraft = Aircrafts_GameState[i].Get();
            }
            else
            {
                Aircraft = AFortAthenaAircraft::SpawnAircraft(GetWorld(), GS->MapInfo->AircraftClass, GS->MapInfo->FlightInfos[i]);
            }

            if (Aircraft)
            {
                TWeakObjectPtr<AFortAthenaAircraft>AicraftWeak;
                AicraftWeak = Aircraft;
                Aircrafts_GameMode.Add(AicraftWeak);
                Aircrafts_GameState.Add(AicraftWeak);

                Aircraft->StartFlightPath(ServerTime);

                FScriptDelegate OnDropZoneExitedDel;
                OnDropZoneExitedDel.Object = this;
                OnDropZoneExitedDel.FunctionName = UKismetStringLibrary::Conv_StringToName(L"OnAircraftExitedDropZone");

                FScriptDelegate OnDropZoneEnteredDel;
                OnDropZoneEnteredDel.Object = this;
                OnDropZoneEnteredDel.FunctionName = UKismetStringLibrary::Conv_StringToName(L"OnAircraftEnteredDropZone");

                FScriptDelegate OnFlightEndedDel;
                OnFlightEndedDel.Object = this;
                OnFlightEndedDel.FunctionName = UKismetStringLibrary::Conv_StringToName(L"OnAircraftFlightEnded");
                Aircraft->OnDropZoneEntered.InvocationList.Add(OnDropZoneEnteredDel);
                Aircraft->OnDropZoneExited.InvocationList.Add(OnDropZoneExitedDel);
                Aircraft->OnFlightEnded.InvocationList.Add(OnFlightEndedDel);
            }
        }
    }

    int32 AircraftReadyPlayerCount = 0;

    for (auto Player: GetGameMode()->AlivePlayers)
    {
        if (!Player)
            continue;

        int32 TargetIndex = (PlaylistData->AirCraftBehavior == EAirCraftBehavior::OpposingAirCraftForEachTeam) ? ((AFortPlayerStateAthena*)Player->PlayerState)->TeamIndex : 0;

        if (Aircrafts_GameMode.IsValidIndex(TargetIndex))
        {
            AFortAthenaAircraft* TargetAircraft = Aircrafts_GameMode[TargetIndex].Get();

            if (Player->Pawn)
            {
                auto Pawn = Player->MyFortPawn;
                Player->LastDamager = 0;
                Player->LastFallInstigator = 0;
                Pawn->ServerInternalEquipWeapon(0);
                Pawn->bIsInsideSafeZone = true;
                Player->GetAircraftComponent()->CurrentAircraft = TargetAircraft;
                Player->GetAircraftComponent()->ClientEnterAircraft(TargetAircraft);
                Player->GetAircraftComponent()->PlacePlayerInAircraft(TargetAircraft, false);
                Pawn->OnRep_IsInsideSafeZone();
                Player->ClientActivateSlot(EFortQuickBars::Primary, 0, 0.0, 1, 0);
                Player->RemoveAllInventoryItems();
                if (Player->Pawn)
                {
                    if (Pawn->Role == ENetRole::ROLE_Authority)
                    {
                        if (Pawn->bIsInAnyStorm)
                        {
                            Pawn->bIsInAnyStorm = false;
                            Pawn->OnRep_IsInAnyStorm();
                        }
                    }
                    Pawn->bIsInsideSafeZone = true;
                    Pawn->OnRep_IsInsideSafeZone();
                    for (auto& ScriptDelegate : Pawn->OnEnteredAircraft.InvocationList)
                    {
                        auto Object = ScriptDelegate.Object;
                        Object->ProcessEvent(FindFunctionByFName(Object->Class, ScriptDelegate.FunctionName), nullptr);
                    }
                }
                static void* (*Reset)(AFortPlayerControllerAthena*) = decltype(Reset)(ImageBase + 0x6DD1E84);
                Reset(Player);
            }
        }
    }
    return true;
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartSafeZones(bool bAllowRestart, bool bStartPhase) {
    AFortGameStateAthena* GS = GetGameState();
    if (!GS)
    {
        return false;
    }

    AFortAthenaMapInfo* MapInfo = GS->MapInfo;
    if (!MapInfo || MapInfo->GetSafeZoneCount() < 2)
    {
        return false;
    }

    UFortPlaylistAthena* PlaylistData = GS->CurrentPlaylistInfo.BasePlaylist;


    if (!bAllowRestart)
    {
        if (this->GamePhase == EAthenaGamePhase::SafeZones)
        {
            return true;
        }

        if (PlaylistData)
        {
            ESafeZoneStartUp StartUpType = PlaylistData->SafeZoneStartUp;

            switch (this->GamePhase)
            {
            case EAthenaGamePhase::Warmup:
                if (StartUpType == ESafeZoneStartUp::StartsWithWarmUp) return true;
                break;

            case EAthenaGamePhase::Aircraft:
                if (StartUpType == ESafeZoneStartUp::StartsWithAirCraft) return true;
                break;

            case EAthenaGamePhase::SafeZones:
                return true;
            }
        }
    }

    float CurrentServerTime = GS->GetServerWorldTimeSeconds();
    float StartDelay = MapInfo->SafeZoneStartDelay.Evaluate();

    this->SafeZonesStartTime = CurrentServerTime + StartDelay;

    if (bStartPhase)
    {
        return StartSafeZonesPhase(bAllowRestart);
    }

    return true;
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartSafeZonesPhase(bool bAllowRestart) {
    AFortGameStateAthena* GS = GetGameState();
    if (!GS)
    {
        return false;
    }

    AFortAthenaMapInfo* MapInfo = GS->MapInfo;
    if (!MapInfo || MapInfo->GetSafeZoneCount() < 2)
    {
        return false;
    }

    if (this->GamePhase != EAthenaGamePhase::SafeZones || bAllowRestart)
    {
        this->SetGamePhase(EAthenaGamePhase::SafeZones);

        if (!GS->IsCreativeOrPlaygroundModePlaylist())
        {
            FPlayersLeft PlayersLeft = GS->GetPlayersLeft();
        }
    }
    return true;
}

void AFortSafeZoneIndicator::StartPhase(int32 InPhaseIndex) {

}

void AFortSafeZoneIndicator::InitializeWithMapDefaultPhases() {
    AFortGameStateAthena* GameState = GetGameState();

    auto Logic = GetLogic();

    FFortSafeZoneDefinition Definition = GameState->MapInfo->SafeZoneDefinition;
    float ZoneCount = Definition.Count.Evaluate();
    auto SafeZoneLocations = *(TArray<FVector>*)(__int64(GetLogic()) + 0x468);

    if (this->SafeZonePhases.IsValid())
        this->SafeZonePhases.Free();

    const float Time = (float)UGameplayStatics::GetTimeSeconds(GameState);
    const float ServerTime = GameState->ServerWorldTimeSecondsDelta + Time;

    for (float i = 0; i < ZoneCount; i++)
    {
        FFortSafeZonePhaseInfo Phase{};

        Phase.Radius = Definition.Radius.EvaluateIndex(i);
        Phase.WaitTime = Definition.WaitTime.EvaluateIndex(i);
        Phase.ShrinkTime = Definition.ShrinkTime.EvaluateIndex(i);
        Phase.PlayerCap = (int)Definition.PlayerCapSolo.EvaluateIndex(i);
        Phase.DamageInfo = FFortSafeZoneDamageInfo();
        UDataTableFunctionLibrary::EvaluateCurveTableRow(GameState->AthenaGameDataTable, UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.Damage"), i, nullptr, &Phase.DamageInfo.Damage, FString());
        if (i == 0.f)
            Phase.DamageInfo.Damage = 0.01f;
        Phase.DamageInfo.bPercentageBasedDamage = true;
        Phase.TimeBetweenStormCapDamage = Logic->TimeBetweenStormCapDamage.EvaluateIndex(i);
        Phase.StormCapDamagePerTick = Logic->StormCapDamagePerTick.EvaluateIndex(i);
        Phase.StormCampingIncrementTimeAfterDelay = Logic->StormCampingIncrementTimeAfterDelay.EvaluateIndex(i);
        Phase.StormCampingInitialDelayTime = Logic->StormCampingInitialDelayTime.EvaluateIndex(i);
        Phase.MegaStormGridCellThickness = (int)Definition.MegaStormGridCellThickness.EvaluateIndex(i);
        Phase.Center = SafeZoneLocations[(int)i];

        this->SafeZonePhases.Add(Phase);
        this->PhaseCount++;
    }
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SpawnInitialSafeZone() {
    AFortGameStateAthena* AthenaGameState = GetGameState();

    if (!AthenaGameState || !AthenaGameState->MapInfo)
    {
        return;
    }

    auto SafeZoneLocations = *(TArray<FVector>*)(__int64(this) + 0x468);
    check(SafeZoneLocations.Num() > 0);
    check(SafeZoneLocations.Num() == AthenaGameState->MapInfo->SafeZoneDefinition.Count.Evaluate());

    if (!SafeZoneIndicator)
    {
        if (SafeZoneIndicatorClass)
        {
            SafeZoneIndicator = (AFortSafeZoneIndicator*)GetWorld()->SpawnActor(SafeZoneIndicatorClass, {});
        }
        if (SafeZoneIndicator)
        {
            FScriptDelegate Delegate;
            Delegate.Object = this;
            Delegate.FunctionName = UKismetStringLibrary::Conv_StringToName(L"OnSafeZonePhaseChanged");
            SafeZoneIndicator->OnSafeZonePhaseChanged.InvocationList.Add(Delegate);
            SafeZoneIndicator->InitializeWithMapDefaultPhases();
            SafeZoneIndicator->StartPhase(0);
        }
    }
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartWarmupPhase() {
    AFortGameStateAthena* GameState = GetGameState();
    if (!GameState || !GameState->IsA(AFortGameStateAthena::StaticClass()))
    {
        return false;
    }
    UFortPlaylistAthena* CurrentPlaylist = GameState->CurrentPlaylistInfo.BasePlaylist;

    if (CurrentPlaylist->SafeZoneStartUp == ESafeZoneStartUp::StartsWithWarmUp)
    {
        StartSafeZones(0, 0);
    }

    if (this->GamePhase == EAthenaGamePhase::Warmup)
    {
        return true;
    }
    if (!CurrentPlaylist->bSkipWarmup)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            this->WarmupCountdownStartTime = World->GetTimeSeconds();
        }

        if (CurrentPlaylist->bSkipAircraft)
        {
            this->bGameModeWillSkipAircraft = true;
        }
        else
        {
            for (auto AircraftPath : GameState->MapInfo->FlightInfos) {
                auto Aircraft = AFortAthenaAircraft::SpawnAircraft(GetWorld(), GameState->MapInfo->AircraftClass, AircraftPath);
                TWeakObjectPtr<AFortAthenaAircraft>AicraftWeak;
                AicraftWeak = Aircraft;
                Aircrafts_GameMode.Add(AicraftWeak);
                Aircrafts_GameState.Add(AicraftWeak);
            }
        }

        this->SetGamePhase(EAthenaGamePhase::Warmup);
        if (CurrentPlaylist)
        {
            this->WarmupCountdownDuration = WarmupDuration;
            this->WarmupCountdownEndTime = this->WarmupCountdownStartTime + WarmupCountdownDuration;
        }

        return true;
    }
    return false;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::UpdateSafeZonePhase() {
    AFortGameStateAthena* GameState = GetGameState();

    if (!GameState)
    {
        return;
    }

    AFortSafeZoneIndicator* SafeZone = this->SafeZoneIndicator;
    if (!SafeZone) {
        SpawnInitialSafeZone();
        SafeZone = this->SafeZoneIndicator;
    }
    float CurrentServerTime = GameState->GetServerWorldTimeSeconds();
    int32 CurrentPhase = SafeZone ? SafeZone->CurrentPhase : 0;


    if (!SafeZone->bPaused)
    {
        if (SafeZone)
        {
            bool bIsEventDriven = SafeZone->bSafezoneEventDriven;
            bool bIsShrinkingFinished = CurrentServerTime >= SafeZone->GetSafeZoneFinishShrinkTime();

            bool bShouldUpdatePhase = bIsShrinkingFinished || bIsEventDriven;

            if (bShouldUpdatePhase)
            {
                FString ResolutionMessage = TEXT("unknown");

                if (SafeZone->bPausedForPreview)
                {
                    SafeZone->bPausedForPreview = false;
                }

                if (CurrentPhase >= SafeZone->PhaseCount - 1)
                {
                    SetGamePhase(EAthenaGamePhase::EndGame);
                }
                else if (CurrentPhase == SafeZone->PlaylistMaxSafeZoneIndex)
                {
                    SetGamePhase(EAthenaGamePhase::EndGame);
                }
                else
                {
                    SafeZone->StartPhase(CurrentPhase++);
                }
            }
        }
    }
}


void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Tick()
{
    UWorld* World = GetWorld();
    if (!World) return;

    auto CurrentTime = World->GetTimeSeconds();

}