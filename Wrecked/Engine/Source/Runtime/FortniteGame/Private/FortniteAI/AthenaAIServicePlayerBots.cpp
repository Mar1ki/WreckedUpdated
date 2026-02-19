#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/FortniteAI/AthenaAIServicePlayerBots.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "SDK/SDK/FortniteAI_structs.hpp"

void AthenaAIServicePlayerBots::InitializeMMRInfos() {
	UAthenaAIServicePlayerBots* AIServicePlayerBots = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());
	AIServicePlayerBots->DefaultBotAISpawnerData = StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");
	
	TArray<AActor*> VolumeS;
	UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPoiVolume::StaticClass(), &VolumeS);
	for (auto& Vol : VolumeS) {
		auto PoiVol = (AFortPoiVolume*)Vol;
		AIServicePlayerBots->AddPOIVolume(PoiVol, EAthenaAIServicePOIList::MainBusDrop);
		FCachedPOIVolumeLocations newLoc{};
		newLoc.POIVolume = PoiVol;
		AIServicePlayerBots->CachedValidPOIVolumeLocations.Add(newLoc);
	}

	FVector2f Base(35.f, 35.f);
	FDeprecateSlateVector2D Vec;
	Vec.X = Base.X;
	Vec.Y = Base.Y;
	AIServicePlayerBots->DebugMinimapData.DebugMinimapIconBrush.ImageSize = FDeprecateSlateVector2D(Vec);
	AIServicePlayerBots->DebugMinimapData.DebugMinimapIconScale = FVector2D(0.5, 0.5);
	AIServicePlayerBots->DebugMinimapData.DebugCompassIconBrush.ResourceObject = StaticLoadObject<UTexture2D>("/Game/Athena/HUD/Spectator/DeimosRiftMapIcon.DeimosRiftMapIcon");
	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"Phoebe.Minimap 1", nullptr);

	FMMRSpawningInfo NewSpawningInfo{};
	NewSpawningInfo.BotSpawningDataInfoTargetELO = 1000.f;
	NewSpawningInfo.BotSpawningDataInfoWeight = 100.f;
	NewSpawningInfo.NumBotsToSpawn = 50;
	NewSpawningInfo.AISpawnerData = AIServicePlayerBots->DefaultBotAISpawnerData;

	AIServicePlayerBots->DefaultAISpawnerDataComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(AIServicePlayerBots->DefaultBotAISpawnerData, UWorld::GetWorld());
	AIServicePlayerBots->CachedMMRSpawningInfo.SpawningInfos.Add(NewSpawningInfo);
	AIServicePlayerBots->bWaitForNavmeshToBeLoaded = false;
	AIServicePlayerBots->GamePhaseToStartSpawning = EAthenaGamePhase::Warmup;
	*(bool*)(__int64(AIServicePlayerBots) + 0x930) = true;
}

char AthenaAIServicePlayerBots::CheckForBotBrainActivation(UAthenaAIServicePlayerBots* AiService) {
	AiService->CachedAIPopulationTracker = (UAthenaAIPopulationTracker*)UGameplayStatics::SpawnObject(UAthenaAIPopulationTracker::StaticClass(), AiService);
	if (!AiService->CachedAIPopulationTracker)
		return CheckForBotBrainActivationOG(AiService);

	AiService->CachedAIPopulationTracker->CachedGameMode = GetGameMode();
	AiService->CachedAIPopulationTracker->AISystem = (UAISystem*)UWorld::GetWorld()->AISystem;
	
	return CheckForBotBrainActivationOG(AiService);
}

void AthenaAIServicePlayerBots::UpdateControlRotations(UAthenaAIServicePlayerBots* AiService, double Delta) {
	for (auto Bot : GetGameMode()->AliveBots) {
		void (*UpdateControlRotation)(AFortAthenaAIBotController*, double, bool) = decltype(UpdateControlRotation)(Bot->VTable[0x8D8 / 8]);
		UpdateControlRotation(Bot, Delta, true);
	}
	/*
	for (auto BotInfo : AiService->PlayerBots) {
		if (BotInfo.BotController) {
			void (*UpdateControlRotation)(AFortAthenaAIBotController*, double, bool) = decltype(UpdateControlRotation)(BotInfo.BotController->VTable[0x8D8/8]);
			UpdateControlRotation(BotInfo.BotController, Delta, true);
			UE_LOG(LogAI, Log, "Updating Control Rotation");
		}
	}
	for (auto BotInfo : AiService->NonAthenaParticipantBots) {
		if (BotInfo.BotController) {
			void (*UpdateControlRotation)(AFortAthenaAIBotController*, double, bool) = decltype(UpdateControlRotation)(BotInfo.BotController->VTable[0x8D8 / 8]);
			UpdateControlRotation(BotInfo.BotController, Delta, true);
			UE_LOG(LogAI, Log, "Updating Control Rotation");
		}
	}
	*/
	
	UpdateControlRotationsOG(AiService, Delta);
}

void ModifyInstructionLEA(uintptr_t instrAddr, uintptr_t targetAddr, int offset)
{
	int64_t delta = static_cast<int64_t>(targetAddr) -
		static_cast<int64_t>(instrAddr + offset + 4);

	auto patchLocation = reinterpret_cast<int32_t*>(instrAddr + offset);

	DWORD oldProtect;
	VirtualProtect(patchLocation, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &oldProtect);

	*patchLocation = static_cast<int32_t>(delta);

	DWORD temp;
	VirtualProtect(patchLocation, sizeof(int32_t), oldProtect, &temp);
}

bool NeedsLoadForClient() {
	return true;
}

void AthenaAIServicePlayerBots::InitializeForWorld(UNavigationSystemV1* Nav,UWorld* World, EFNavigationSystemRunMode Mode)
{
	Nav->bAutoCreateNavigationData = true;
	Nav->bAutoCreateNavigationData = true;
	Nav->SupportedAgentsMask.bSupportsAgent3 = 1;
	Nav->bSpawnNavDataInNavBoundsLevel = true;
	
	return InitializeForWorldOG(Nav, World, Mode);
}

void AthenaAIServicePlayerBots::CreateAndConfigureNavigationSystem(UAthenaNavSystemConfig* ModuleConfig, UWorld* World)
{
	ModuleConfig->bPrioritizeNavigationAroundSpawners = true;
	ModuleConfig->bAutoSpawnMissingNavData = true;
	ModuleConfig->bAllowAutoRebuild = true;
	ModuleConfig->bSupportRuntimeNavmeshDisabling = false;
	ModuleConfig->bUsesStreamedInNavLevel = true;

	return CreateAndConfigureNavigationSystemOG(ModuleConfig, World);
}
void AthenaAIServicePlayerBots::Init() {

	UHook* Hook = new UHook();

	Hook->Address = ImageBase + 0xA954E36;
	Hook->Byte = 0xEB;
	UKismetHookingLibrary::Hook(Hook, Byte);

	ModifyInstructionLEA(ImageBase + 0xA950FB4, ImageBase + 0xAd370C4, 3);

	Hook->Address = ImageBase + 0xA927B58;
	Hook->Detour = CheckForBotBrainActivation;
	Hook->Original = (void**)&CheckForBotBrainActivationOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = ImageBase + 0x4C2E634;
	Hook->Detour = UpdateControlRotations;
	Hook->Original = (void**)&UpdateControlRotationsOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = ImageBase + 0xAd370C4;
	Hook->Detour = InitializeMMRInfos;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Class = AAthenaAIDirector::StaticClass();
	Hook->Address = 0x108 / 8;
	Hook->Detour = NeedsLoadForClient;
	UKismetHookingLibrary::Hook(Hook, VFT);

	Hook->Class = AFortAIDirector::StaticClass();
	Hook->Address = 0x108 / 8;
	Hook->Detour = NeedsLoadForClient;
	UKismetHookingLibrary::Hook(Hook, VFT);

	Hook->Class = AAthenaNavMesh::StaticClass();
	Hook->Address = 0x108 / 8;
	Hook->Detour = NeedsLoadForClient;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
	Hook->Class = UNavigationSystemV1::StaticClass();
	Hook->Address = 0x108 / 8;
	Hook->Detour = NeedsLoadForClient;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
	Hook->Class = UNavigationSystemConfig::StaticClass();
	Hook->Address = 0x2B8 / 8;
	Hook->Detour = CreateAndConfigureNavigationSystem;
	Hook->Original = (void**)&CreateAndConfigureNavigationSystemOG;
	//UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Class = UNavigationSystemV1::StaticClass();
	Hook->Address = 0x2F0 / 8;
	Hook->Detour = InitializeForWorld;
	Hook->Original = (void**)&InitializeForWorldOG;
	//UKismetHookingLibrary::Hook(Hook, EveryVFT);

	delete Hook;
}