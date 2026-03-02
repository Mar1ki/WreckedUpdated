#pragma once
#include "pch.h"

namespace FortGameModeAthena
{
    DefineOriginal(bool, ReadyToStartMatch, AFortGameModeAthena* GameMode);
    APawn* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot);
    DefineOriginal(void, HandleStartingNewPlayer, AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer);
    DefineOriginal(void, HandleMatchHasStarted, AFortGameModeAthena* GameMode);

    EFortTeam PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller);

    void StartNewSafeZonePhase(AFortGameModeAthena* GameMode);
    void InitialSafeZone(AFortGameModeAthena* GameMode);

    static uint8_t PlayersOnCurrentTeam = 0;
    static uint8_t CurrentTeam = 3;

    static void (*NotifyGameMemberAdded)(AFortGameStateAthena*, uint8_t, uint8_t, FUniqueNetIdRepl*) = decltype(NotifyGameMemberAdded)(ImageBase + 0x26D37F4);

    
    
    void Init();
}
