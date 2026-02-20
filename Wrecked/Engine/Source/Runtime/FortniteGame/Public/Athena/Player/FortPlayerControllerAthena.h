#pragma once
#include "pch.h"

namespace FortPlayerControllerAthena {
	DefineOriginal(void, ServerAttemptAircraftJump, UFortControllerComponent_Aircraft* Comp, FRotator ClientRot);
	DefineOriginal(void, ClientIsReadyToRespawn, AFortPlayerControllerAthena* PlayerController);
	//void ServerReturnToMainMenu(AFortPlayerControllerAthena* PlayerController);
	DefineOriginal(void, GetPlayerViewPoint, APlayerController* PlayerController, FVector& Loc, FRotator& Rot);
	void Init();
}