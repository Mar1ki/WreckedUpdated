#include "pch.h"

#include "Engine/Plugins/TaskLibrary/Public/TaskLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortGameModeAthena.h"

// TODO: Make the gamephase logic actually use the playlist info bc i see that it doesnt 
void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Tick()
{
    UWorld* World = GetWorld();
    if (!World) return;

    auto CurrentTime = UGameplayStatics::GetTimeSeconds(GetWorld());

    if (!UTaskLibrary::IsInitialized("UFortGameStateComponent_BattleRoyaleGamePhaseLogic"))
    {
        UTaskLibrary::Initialize("UFortGameStateComponent_BattleRoyaleGamePhaseLogic", CurrentTime);
        UTaskLibrary::QueueTask("UFortGameStateComponent_BattleRoyaleGamePhaseLogic", CurrentTime, WarmupDuration, [this, World]() {
            if (GamePhase == EAthenaGamePhase::Warmup)
                if (auto GameMode = Cast<AFortGameModeAthena>(World->AuthorityGameMode))
                    GameMode->StartAircraftPhase();
        });

        return;
    }

    UTaskLibrary::ProcessTasks("UFortGameStateComponent_BattleRoyaleGamePhaseLogic", CurrentTime);

    static bool bAircraftUnlocked = false;
    if (!bAircraftUnlocked)
    {
        AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(World->AuthorityGameMode);
        if (!GameMode || GameMode->AlivePlayers.Num() == 0 || Aircrafts_GameState.Num() == 0)
            return;

        AFortAthenaAircraft* Aircraft = Aircrafts_GameState[0].Get();
        if (Aircraft && Aircraft->DropStartTime != -1 && Aircraft->DropStartTime <= CurrentTime)
        {
            bAircraftUnlocked = true;
            bAircraftIsLocked = false;
            
            UTaskLibrary::QueueTask("UFortGameStateComponent_BattleRoyaleGamePhaseLogic", CurrentTime, 33.f, [this, GameMode, Aircraft]() {
                if (GamePhase == EAthenaGamePhase::Aircraft)
                {
                    for (auto Player : GameMode->AlivePlayers)
                    {
                        if (Player->IsInAircraft())
                            Player->GetAircraftComponent()->ServerAttemptAircraftJump(FRotator());
                    }
                    
                    Aircraft->K2_DestroyActor();
                    FortGameModeAthena::InitialSafeZone(GameMode);
                    SetGamePhase(EAthenaGamePhase::SafeZones);
                }
            });
            
            // Handle thank bus driver for bots
            for (auto Controller : GameMode->AliveBots)
            {
                if (Controller->CachedAIServicePlayerBots)
                {
                    if (GetGameState()->IsA(AFortGameStateAthena::StaticClass()))
                    {
                        float ThankProbability = Controller->CachedAIServicePlayerBots->ThankBusDriverProbability.Evaluate();
                        
                        if (ThankProbability > 0 && (rand() & 0x7FFF) < (ThankProbability * 32767.0f))
                        {
                            UAthenaAIServicePlayerBots* AIService = Controller->CachedAIServicePlayerBots;
                            float MaxTime = AIService->ThankBusDriverMaxTime.Evaluate();
                            float MinTime = AIService->ThankBusDriverMinTime.Evaluate();
                            // return min and max at 1 idk why
                          auto ignew =  UFortScalableFloatUtils::GetValueAtLevel(AIService->ThankBusDriverMinTime,((rand() & 0x7FFF) / 32767.0f) );
                            float RandomTime = MinTime + ((rand() & 0x7FFF) / 32767.0f) * (MaxTime - MinTime);
                            
                            if (RandomTime > 0.0f)
                            {
                                UTaskLibrary::QueueTask("UFortGameStateComponent_BattleRoyaleGamePhaseLogic", CurrentTime, RandomTime, [Controller]() {
                                    auto BotState = (AFortPlayerStateAthena*)Controller->PlayerState;
                                    Controller->ThankBusDriver();
                                    BotState->bThankedBusDriver = true;
                                    BotState->OnRep_ThankedBusDriver();
                                });
                            }
                        }
                    }
                }
            }
        }
    }
    
    AFortGameModeAthena* GameMode = Cast<AFortGameModeAthena>(World->AuthorityGameMode);
    if (!SafeZoneIndicator) return;

    if (SafeZoneIndicator->SafeZoneFinishShrinkTime < CurrentTime)
    {
        if (SafeZoneIndicator->CurrentPhase >= 12)
            SetGamePhase(EAthenaGamePhase::EndGame);
        else
            FortGameModeAthena::StartNewSafeZonePhase(GameMode);
        return;
    }

    if (SafeZoneIndicator->SafeZoneStartShrinkTime <= CurrentTime && GamePhaseStep != EAthenaGamePhaseStep::StormShrinking)
    {
        GamePhaseStep = EAthenaGamePhaseStep::StormShrinking;
        HandleGamePhaseStepChanged(EAthenaGamePhaseStep::StormShrinking);
    }

    for (auto Player : GameMode->AlivePlayers)
    {
        if (Player->MyFortPawn)
        {
            bool bIsInSafeZone = IsInCurrentSafeZone(Player->MyFortPawn->K2_GetActorLocation(), true);
            auto Pawn = Player->MyFortPawn;
            if (Pawn->bIsInsideSafeZone != bIsInSafeZone && Pawn->bIsInAnyStorm!=!bIsInSafeZone)
            {
                Pawn->bIsInAnyStorm = !bIsInSafeZone;
                Pawn->OnRep_IsInAnyStorm();
                Pawn->bIsInsideSafeZone = bIsInSafeZone; 
                Pawn->OnRep_IsInsideSafeZone();
            }
        }
    }
}