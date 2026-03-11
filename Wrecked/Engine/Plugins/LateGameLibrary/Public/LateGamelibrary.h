#pragma once
#include "pch.h"

class ULateGameLibrary {
public:
	static void CustomSafeZoneHandler(float Time);
	static void RestartSafeZoneIndicator();
	static void LateGameStartAircraft();
	static void SpawnInitialLateGameSafeZone();
	static void GiveRandomLoadoutToPlayer(AFortPlayerControllerAthena* PlayerCotntroller);
};