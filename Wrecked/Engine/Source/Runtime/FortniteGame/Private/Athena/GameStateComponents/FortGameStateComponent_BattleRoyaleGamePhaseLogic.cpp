#include "pch.h"

#include "Engine/Plugins/TaskLibrary/Public/TaskLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortGameModeAthena.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
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
    this->SetGamePhase(EAthenaGamePhase::Aircraft);
    GamePhaseStep = EAthenaGamePhaseStep::BusLocked;
    HandleGamePhaseStepChanged(GamePhaseStep);
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
    auto Time = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
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
        this->SafeZonesStartTime = Time + GS->MapInfo->SafeZoneStartDelay.Evaluate();
        this->SetGamePhase(EAthenaGamePhase::SafeZones);
        this->GamePhaseStep = EAthenaGamePhaseStep::StormForming;
        this->HandleGamePhaseStepChanged(GamePhaseStep);
        UE_LOG(LogGamePhases, Log, "Starting Safezones!");
    }
    return true;
}

void AFortSafeZoneIndicator::StartPhase(int32 InPhaseIndex) {
    auto CurrentTime = GetWorld()->GetTimeSeconds();
    auto GamePhaseLogic = GetLogic();

    if (!GamePhaseLogic) return;

    FFortSafeZonePhaseInfo Info = SafeZonePhases[InPhaseIndex];
    if (InPhaseIndex != 0)
    {
        PreviousCenter = NextCenter;
        PreviousRadius = NextRadius;
    }
    NextCenter = (FVector_NetQuantize100)Info.Center;
    NextRadius = Info.Radius;
    CurrentDamageInfo = Info.DamageInfo;
    CurrentPhase = InPhaseIndex;
    SafeZoneStartShrinkTime = CurrentTime + Info.WaitTime;
    SafeZoneFinishShrinkTime = SafeZoneStartShrinkTime + Info.ShrinkTime;

    *(EFortSafeZoneState*)(__int64(this) + 0x60C) = EFortSafeZoneState::Holding;
    OnSafeZoneStateChange(EFortSafeZoneState::Holding, InPhaseIndex == 0);

    GamePhaseLogic->GamePhaseStep = EAthenaGamePhaseStep::StormHolding;
    GamePhaseLogic->HandleGamePhaseStepChanged(GamePhaseLogic->GamePhaseStep);
    OnRep_CurrentPhase();
    UE_LOG(LogGamePhases, Log, "Starting new SafeZone (Index: %i): ShrinkTime: %f WaitTime %f Radius %f", InPhaseIndex, Info.ShrinkTime, Info.WaitTime, Info.Radius);
}

void AFortSafeZoneIndicator::InitializeWithMapDefaultPhases() {
    AFortGameStateAthena* GameState = GetGameState();

    auto Logic = GetLogic();

    FFortSafeZoneDefinition Definition = GameState->MapInfo->SafeZoneDefinition;
    float ZoneCount = Definition.Count.Evaluate();
    auto SafeZoneLocations = *(TArray<FVector>*)(__int64(GetLogic()) + 0x468);

    if (this->SafeZonePhases.IsValid())
        this->SafeZonePhases.Free();

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
    OnRep_PhaseCount();
    this->SafeZoneHeight = 100000;
    this->PreviousCenter = (FVector_NetQuantize100)GameState->MapInfo->GetMapCenter();
    this->NextCenter = (FVector_NetQuantize100)GameState->MapInfo->GetMapCenter();
    this->PreviousRadius = GameState->MapInfo->SafeZoneStartingRadius.Evaluate();
    this->NextRadius = PreviousRadius;
    this->CurrentPhase = 0;
    OnRep_CurrentPhase();
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SpawnInitialSafeZone() {
    AFortGameStateAthena* AthenaGameState = GetGameState();
    AFortGameModeAthena* GameMode = GetGameMode();

    if (!AthenaGameState || !AthenaGameState->MapInfo || !GameMode)
    {
        return;
    }

    bool ShouldSpawnSafeZoneIndicator = true;
    for (auto Mutator : GameMode->MutatorListComponent->Mutators) {
        EFortMutatorReturnValue(*ShouldSpawnSafeZoneIndicatorOG)(AFortGameplayMutator*, bool*) = decltype(ShouldSpawnSafeZoneIndicatorOG)(Mutator->VTable[0xA60 / 8]);
        if (ShouldSpawnSafeZoneIndicatorOG(Mutator, &ShouldSpawnSafeZoneIndicator) == EFortMutatorReturnValue::Ignore)
            break;
    }
    if (!ShouldSpawnSafeZoneIndicator) return;

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
            SafeZoneIndicator->StartPhase(1);
            OnRep_SafeZoneIndicator();
        }
    }
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::UpdateWarmupPhase(float Time) {
    if (this->WarmupCountdownEndTime < Time) {
        if (!this->StartAircraftPhase()) {
            if (GetGameState()->CurrentPlaylistInfo.BasePlaylist->SafeZoneStartUp == ESafeZoneStartUp::StartsWithNoAirCraft) {
                if (!this->StartSafeZones(true, true) && GetGameState()->MapInfo)
                    this->SetGamePhase(EAthenaGamePhase::None);
            }
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
        GamePhaseStep = EAthenaGamePhaseStep::Warmup;
        HandleGamePhaseStepChanged(GamePhaseStep);
        if (CurrentPlaylist)
        {
            this->WarmupCountdownDuration = WarmupDuration;
            this->WarmupCountdownEndTime = this->WarmupCountdownStartTime + WarmupCountdownDuration;
        }

        return true;
    }
    return false;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::UpdateSafeZonePhase(float Time) {
    AFortGameStateAthena* GameState = GetGameState();
    AFortGameModeAthena* GameMode = GetGameMode();
    if (!GameState || !GameState->MapInfo)
    {
        return;
    }
    
    if (SafeZonesStartTime > Time || SafeZonesStartTime<=0)
        return;
    
    if (!SafeZoneIndicator) {
        SpawnInitialSafeZone();
    }
    if (SafeZoneIndicator) {
        bool bFinishedHolding = SafeZoneIndicator->SafeZoneStartShrinkTime < Time;
        bool bFinishedShrinking = SafeZoneIndicator->SafeZoneFinishShrinkTime < Time;

        if (bFinishedHolding && bFinishedShrinking) {
            if (SafeZoneIndicator->CurrentPhase + 1 == 13) {
                SetGamePhase(EAthenaGamePhase::EndGame);
                GamePhaseStep = EAthenaGamePhaseStep::EndGame;
                HandleGamePhaseStepChanged(GamePhaseStep);
            }
            SafeZoneIndicator->StartPhase(SafeZoneIndicator->CurrentPhase+1);
        }
        else if (!bFinishedHolding && !bFinishedShrinking && GamePhaseStep == EAthenaGamePhaseStep::StormHolding) {
            GamePhaseStep = EAthenaGamePhaseStep::StormShrinking;
            HandleGamePhaseStepChanged(GamePhaseStep);

            *(EFortSafeZoneState*)(__int64(SafeZoneIndicator) + 0x60C) = EFortSafeZoneState::Shrinking;
            SafeZoneIndicator->OnSafeZoneStateChange(EFortSafeZoneState::Holding, SafeZoneIndicator->CurrentPhase == 0);
        }
    }
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::UpdateAircraftPhase(float Time) {
    AFortGameStateAthena* GameState = GetGameState();

    if (!GameState) {
        return;
    }

    for (auto AircraftWeak : this->Aircrafts_GameMode) {
        auto Aircraft = AircraftWeak.Get();
        if (!Aircraft) continue;
        this->bAircraftIsLocked = Aircraft->DropStartTime > Time;

        if (!bAircraftIsLocked && GamePhaseStep != EAthenaGamePhaseStep::BusFlying) {
            GamePhaseStep = EAthenaGamePhaseStep::BusFlying;
            HandleGamePhaseStepChanged(GamePhaseStep);
        }
        
        bool ShouldEjectPlayers = Aircraft->DropEndTime < Time;

        if (ShouldEjectPlayers) {
            for (auto Player : GetGameMode()->AlivePlayers)
            {
                if (Player->IsInAircraft())
                {
                    Player->GetAircraftComponent()->ServerAttemptAircraftJump(FRotator());
                }
            }
            if (GameState->CurrentPlaylistInfo.BasePlaylist && GameState->CurrentPlaylistInfo.BasePlaylist->SafeZoneStartUp == ESafeZoneStartUp::UseDefaultGameBehavior)
                StartSafeZones(0, 1);

            Aircraft->K2_DestroyActor();
        }
    }
}

void FortGameStateComponent_BattleRoyaleGamePhaseLogic::GamePhaseTickComponent(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Logic, float Delta)
{
    UWorld* World = GetWorld();
    if (!World) return GamePhaseTickComponentOG(Logic, Delta);

    auto CurrentTime = World->GetTimeSeconds();

    if (Logic->GamePhase == EAthenaGamePhase::Warmup) {
        Logic->UpdateWarmupPhase(CurrentTime);
    }
    if (Logic->GamePhase == EAthenaGamePhase::Aircraft) {
        Logic->UpdateAircraftPhase(CurrentTime);
    }
    if (Logic->GamePhase == EAthenaGamePhase::SafeZones) {
        Logic->UpdateSafeZonePhase(CurrentTime);
    }

    if (Logic->SafeZoneIndicator) {
        auto GameMode = GetGameMode();
        for (auto Player : GameMode->AlivePlayers)
        {
            if (Player->MyFortPawn)
            {
                bool bIsInSafeZone = Logic->IsInCurrentSafeZone(Player->MyFortPawn->K2_GetActorLocation(), true);
                auto Pawn = Player->MyFortPawn;
                if (Pawn->bIsInsideSafeZone != bIsInSafeZone && Pawn->bIsInAnyStorm != !bIsInSafeZone)
                {
                    Pawn->bIsInAnyStorm = !bIsInSafeZone;
                    Pawn->OnRep_IsInAnyStorm();
                    Pawn->bIsInsideSafeZone = bIsInSafeZone;
                    Pawn->OnRep_IsInsideSafeZone();
                }
            }
        }
    }
    return GamePhaseTickComponentOG(Logic, Delta);
}

void FortGameStateComponent_BattleRoyaleGamePhaseLogic::Init() {
    UHook* Hook = new UHook();

    Hook->Address = 0x79;
    Hook->Class = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass();
    Hook->Detour = GamePhaseTickComponent;
    Hook->Original = (void**)&GamePhaseTickComponentOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    delete Hook;
}