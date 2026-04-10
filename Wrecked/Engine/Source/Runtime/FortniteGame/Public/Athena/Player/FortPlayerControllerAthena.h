#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortPlayerControllerAthena {
	DefineOriginal(void, ServerAttemptAircraftJump, UFortControllerComponent_Aircraft* Comp, FRotator ClientRot);
	DefineOriginal(void, ClientIsReadyToRespawn, AFortPlayerControllerAthena* PlayerController);
	//void ServerReturnToMainMenu(AFortPlayerControllerAthena* PlayerController);
	static void (*BeginSpectating)(AFortPlayerStateZone*, AFortPlayerStateZone*) = decltype(BeginSpectating)(ImageBase + 0xA291530);
	static void (*ClientSetViewTarget_Implementation)(AFortPlayerControllerAthena *a1, AFortPlayerControllerAthena* TargetController, FViewTargetTransitionParams *TransitionParams) = decltype(ClientSetViewTarget_Implementation)(ImageBase + 0x90531CC);
	static void (*SetViewTargetAndNotifyClient)(AFortPlayerControllerAthena*, AActor*, FViewTargetTransitionParams&) = decltype(SetViewTargetAndNotifyClient)(ImageBase + 0x9090C70);
	void ServerReturnToMainMenu(UObject* Context, FFrame& Stack);
	void ServerGiveCreativeItem(UObject* Context, FFrame& Stack);

	
	void ServerSpectateActor(AFortPlayerControllerAthena* PlayerController, AActor* NewViewTarget, bool bAllowStateChange);
	
	void Init();
}
