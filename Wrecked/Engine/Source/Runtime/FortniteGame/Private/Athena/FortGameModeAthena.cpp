#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortGameModeAthena.h"

#include <chrono>

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaCreativePortal.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaMapInfo.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaVehicle.h"

void SpawnFloorLoot()
{
	static bool Spawned = false;
	if (!Spawned) {

		TArray<AActor*> FloorLootSpawnersWarmup;
		UClass* SpawnerClass = StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnerClass, &FloorLootSpawnersWarmup);
		if (FloorLootSpawnersWarmup.Num() > 180) {
			for (size_t i = 0; i < FloorLootSpawnersWarmup.Num(); i++)
				FloorLootSpawnersWarmup[i]->K2_DestroyActor();
			Spawned = true;
			FloorLootSpawnersWarmup.Free();
		}
	}
}

void SpawnCactuses() {
	static bool bSpawned = false;
	if (!bSpawned)
	{
		bSpawned = true;
		auto Idk = StaticLoadObject<URuntimeSpatialHashExternalStreamingObject>("/CactusFruits/CB/Athena/Helios/Maps/Helios_Terrain/_Generated_/StreamingObject_3BK3RQHQ8JDZZ7LSWAHB5T60S.RuntimeHashExternalStreamingObjectBase");
		auto CactusClass = StaticLoadObject<UClass>("/CactusFruits/Gameplay/Actors/BGA_Moonflax_HealingCactus.BGA_Moonflax_HealingCactus_C", UBlueprintGeneratedClass::StaticClass());
		for (FSpatialHashStreamingGrid& StreamingGrid : Idk->StreamingGrids) {
			for (FSpatialHashStreamingGridLevel& GridLevel : StreamingGrid.GridLevels) {
				for (FSpatialHashStreamingGridLayerCell& LayerCell : GridLevel.LayerCells) {
					for (UWorldPartitionRuntimeCell* GridCell : LayerCell.GridCells) {
						UWorldPartitionRuntimeCellData* RuntimeCelData = GridCell->RuntimeCellData;
						if (RuntimeCelData) {
							FBox CellBounds = RuntimeCelData->ContentBounds;
							FVector Pos = UKismetMathLibrary::RandomPointInBoundingBox_Box(CellBounds);
							FVector ContentCenter = UFortKismetLibrary::FindGroundLocationAt(GetWorld(), nullptr, Pos, -1000, 2500, UKismetStringLibrary::Conv_StringToName(L"FortDynamicMeshPhysics"));
						//	UE_LOG(LogGameMode, Log, "Spawning Cactus at (%d, %d, %d)", ContentCenter.X, ContentCenter.Y, ContentCenter.Z);
							GetWorld()->SpawnActor(CactusClass, ContentCenter);
						}
					}
				}
			}
		}
	}
}

void SpawnBarrels() {
	static bool bSpawned = false;
	if (!bSpawned)
	{
		bSpawned = true;
		auto StreamingObject = StaticLoadObject<URuntimeSpatialHashExternalStreamingObject>("/NitroBarrel//CB/Athena/Helios/Maps/Helios_Terrain/_Generated_/StreamingObject_94UKJYY1A57OBRXJLKRIBFKBH.RuntimeHashExternalStreamingObjectBase");
		auto CactusClass = StaticLoadObject<UClass>("/NitroBarrel/Gameplay/Prop_Moonflax_PhysicsObject_NitroBarrel.Prop_Moonflax_PhysicsObject_NitroBarrel_C", UBlueprintGeneratedClass::StaticClass());
		for (FSpatialHashStreamingGrid& StreamingGrid : StreamingObject->StreamingGrids) {
			for (FSpatialHashStreamingGridLevel& GridLevel : StreamingGrid.GridLevels) {
				for (FSpatialHashStreamingGridLayerCell& LayerCell : GridLevel.LayerCells) {
					for (UWorldPartitionRuntimeCell* GridCell : LayerCell.GridCells) {
						auto RuntimeCelData = GridCell->RuntimeCellData;
						if (RuntimeCelData) {
							FBox CellBounds = RuntimeCelData->ContentBounds;
							FVector Pos = UKismetMathLibrary::RandomPointInBoundingBox_Box(CellBounds);
							FVector ContentCenter = UFortKismetLibrary::FindGroundLocationAt(GetWorld(), nullptr, Pos, -1000, 2500, UKismetStringLibrary::Conv_StringToName(L"FortDynamicMeshPhysics"));
						//	UE_LOG(LogGameMode, Log, "Spawning Cactus at (%d, %d, %d)", ContentCenter.X, ContentCenter.Y, ContentCenter.Z);
							GetWorld()->SpawnActor(CactusClass, ContentCenter);
						}
					}
				}
			}
		}
	}
}

bool FortGameModeAthena::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
	auto GameState = Cast<AFortGameStateAthena>(GameMode->GameState);

	if (CurrentGamemode == EGameModeType::Athena)
		if (!GameState || !GameState->MapInfo) return false;

	if (GameMode->CurrentPlaylistId == -1)
	{
		UFortPlaylistAthena* Playlist = nullptr;
		switch (CurrentGamemode)
		{
		    case Athena:
			Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
			break;

		    case Papaya:
			Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Papaya.Playlist_Papaya");
			break;

		    case Juno:
			Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Juno.Playlist_Juno");
			break;
			
		    case Creative:
			Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_PlaygroundV2.Playlist_PlaygroundV2");
			break;

		    case Delmar:
			Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DelMar_Tutorial.Playlist_DelMar_Tutorial");
			break;

			case BlastBerry:
			Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_VK_Play.Playlist_VK_Play");
			break;
		}
		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();
		GameMode->CurrentPlaylistId = Playlist->PlaylistId;
		GameMode->CurrentPlaylistName = Playlist->PlaylistName;
		GameState->CurrentPlaylistId = Playlist->PlaylistId;

		GameMode->MaxPlayerCount = Playlist->MaxPlayers;
		GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;
		
		GameState->CurrentPlaylistInfo.BasePlaylist->GarbageCollectionFrequency = 9999999999999999;
		GameMode->bDisableGCOnServerDuringMatch = true;
		GameMode->bPlaylistHotfixChangedGCDisabling = true;

		// ig it should be alr
		if (Playlist->DBNOType == EDBNOType::On)
		{
			GameMode->bDBNOEnabled = true;
			GameState->bDBNODeathEnabled = true;
			GameState->SetIsDBNODeathEnabled(true);
		}
		GameMode->bAllowSpectateAfterDeath = true;
		GameState->OnRep_CurrentPlaylistId();
		GameState->OnRep_CurrentPlaylistInfo();

		GameMode->WarmupRequiredPlayerCount = 1;
		//if (CurrentGamemode == EGameModeType::Athena || CurrentGamemode == EGameModeType::Papaya || CurrentGamemode == EGameModeType::Creative)
			//GameMode->DefaultPawnClass = APlayerPawn_Athena_C::StaticClass();
		GameState->DefaultParachuteDeployTraceForGroundDistance = 10000;
		
		for (auto& AdditionalLevel : Playlist->AdditionalLevels) {
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(GetWorld(), AdditionalLevel, {}, {}, nullptr, {}, {}, false);
			FAdditionalLevelStreamed LevelStreamed{};
			LevelStreamed.LevelName = AdditionalLevel.ObjectID.AssetPath.AssetName;
			LevelStreamed.bIsServerOnly = false;
			GameState->AdditionalPlaylistLevelsStreamed.Add(LevelStreamed);
		}
		for (auto& AdditionalLevel : Playlist->AdditionalLevelsServerOnly) {
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(GetWorld(), AdditionalLevel, {}, {}, nullptr, {}, {}, false);
			FAdditionalLevelStreamed LevelStreamed{};
			LevelStreamed.LevelName = AdditionalLevel.ObjectID.AssetPath.AssetName;
			LevelStreamed.bIsServerOnly = true;
			GameState->AdditionalPlaylistLevelsStreamed.Add(LevelStreamed);
		}
		
		GameState->OnFinishedStreamingAdditionalPlaylistLevel();
		GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		GameState->bIsUsingDownloadOnDemand = false;

		UAthenaAISettings* AISettingsInstance = nullptr;
		for (int i = 0; i < UObject::GObjects->Num(); i++)
		{
			auto Object = UObject::GObjects->GetByIndex(i);
			if (Object && Object->Class == UAthenaAISettings::StaticClass())
			{
				AISettingsInstance = (UAthenaAISettings*)Object;
				break;
			}
		}

		if (AISettingsInstance) {
			GameMode->AISettings = AISettingsInstance;

			GameMode->AISettings->AIServices.Free();
			GameMode->AISettings->AIServices.Add(StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/AIServices/BP_Phoebe_AIService_Loot.BP_Phoebe_AIService_Loot_C"));
			GameMode->AISettings->AIServices.Add(UAthenaAIServicePlayerBots::StaticClass());
			GameMode->AISettings->AIServices.Add(UAthenaAIServiceVehicle::StaticClass());
			GameMode->AISettings->AIServices.Add(UAthenaAIServiceGroup::StaticClass());
			GameMode->AISettings->AIServices.Add(UAthenaAIServiceZoneManager::StaticClass());
			GameMode->AISettings->AIServices.Add(StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/AIServices/BP_Phoebe_AIService_Cover.BP_Phoebe_AIService_Cover_C"));

			GameMode->AISettings->MaxFootstepHearingRange = 3000.0f;
			GameMode->AISettings->ReducedDeAggroRange = 3500.0f;
		}
		else {
			GameMode->AISettings = Playlist->AISettings.Get();
		}
		
		
		if (!GameMode->AIDirector)
			GameMode->AIDirector = (AFortAIDirector*)GetWorld()->SpawnActor(AFortAIDirector::StaticClass(), {}, {}, GameMode);

		GameMode->AIDirector->Activate();
		GameMode->AISettings->bAllowAIDirector = true;

		if (!GameMode->AIGoalManager)
			GameMode->AIGoalManager = (AFortAIGoalManager*)GetWorld()->SpawnActor(AFortAIGoalManager::StaticClass(), {}, {}, GameMode);

		GameMode->AISettings->bAllowAIGoalManager = true;

		if (!GameMode->SpawningPolicyManager)
			GameMode->SpawningPolicyManager = (AFortAthenaSpawningPolicyManager*)GetWorld()->SpawnActor(AFortAthenaSpawningPolicyManager::StaticClass(), {}, {}, GameMode);

		GameMode->SpawningPolicyManager->GameModeAthena = GameMode;
		GameMode->SpawningPolicyManager->GameStateAthena = GameState;
	}

	if (CurrentGamemode == Athena && GameState->Teams.Num() == 0) return false;
	static bool bFirst = false;
	if (!bFirst)
	{
		FURL Url{};
		Url.Port = 7777;
		GetWorld()->Listen(Url);
		GameMode->bWorldIsReady = true;
		if (CurrentGamemode == Athena && !bLateGame) {
			TArray<FVector4> Blacklist = {};
			auto MapCenter = GameState->MapInfo->GetMapCenter();
			GameState->MapInfo->GenerateSafeZoneLocations(*(TArray<FVector>*)(__int64(GetLogic()) + 0x468), Blacklist, &MapCenter, 0, false);
			static void* (*InitializeFlightPath)(AFortAthenaMapInfo*, AFortGameStateAthena*, UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, bool, double, float, float) = decltype(InitializeFlightPath)(ImageBase + 0x8F454DC);
			InitializeFlightPath(GameState->MapInfo, GameState, UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(GetWorld()), false, 0.f, 0.f, 0.f);
		}
		bFirst = true;
	}

	// add a check if all bots are spawned + lwm events that are in warmup phase spawened
	if (GameMode->AlivePlayers.Num() > 0)
	{
		auto& AlivePlayers = GameMode->AlivePlayers;
		for (AFortPlayerControllerAthena* Player : AlivePlayers)
		{
			if (Player->bHasServerFinishedLoading)
				return true;
		}
	}

	return false;
}

APawn* FortGameModeAthena::SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
	UE_LOG(LogGameMode, Log, "Spawning pawn!");
	if (CurrentGamemode == Athena)
	{
		SpawnFloorLoot();
		SpawnCactuses();
	}
	FTransform Transform;
	if (GetLogic()->GamePhase <= EAthenaGamePhase::Warmup)
	{
		if (NewPlayer->WarmupPlayerStart->PlayerClaimedCount > 0)
		{
			NewPlayer->WarmupPlayerStart = (AFortPlayerStartWarmup*)GetGameMode()->ChoosePlayerStart(NewPlayer);
		}
		Transform = NewPlayer->WarmupPlayerStart->GetTransform();
		NewPlayer->WarmupPlayerStart->PlayerClaimedCount++; // thats gay bc it should auto increase but whatever
	} else
	{
		Transform = StartSpot->GetTransform();
	}
	auto Pawn = (AFortPlayerPawnAthena*)GameMode->SpawnDefaultPawnAtTransform(NewPlayer, Transform);
	
	AFortInventory* Inventory = NewPlayer->GetInventory();
	for (int i = 0; i < GameMode->StartingItems.Num(); i++)
	{
		auto Item = GameMode->StartingItems[i];
		if (Item.Count <= 0) continue;
    	
		if (!Item.Item || (Item.Item && Item.Item->IsA(UFortResourceItemDefinition::StaticClass())))
			continue;
		if (Item.Item->IsA(UFortSmartBuildingItemDefinition::StaticClass()))
			continue;
		if (!Item.Item->IsA(UFortItemDefinition::StaticClass()))
			continue;
    	
		Inventory->AddItem((UFortItemDefinition*)Item.Item, Item.Count, 0);
	}
	
	if (CurrentGamemode == Creative)
	{
		FortAthenaCreativePortal::CreatePortal(NewPlayer);
		FortAthenaCreativePortal::TestProfile(NewPlayer);
		FortAthenaCreativePortal::UpdateAllPortals(GetGameState()->CreativePortalManager); // that will the issue where on start player has 0/0 population
	}

	Inventory->AddItem(NewPlayer->CosmeticLoadoutPC.Pickaxe->WeaponDefinition, 1, 0);
	return Pawn;
}

void FortGameModeAthena::HandleMatchHasStarted(AFortGameModeAthena* GameMode)
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Logic = GetLogic();
	if (!Logic->StartWarmupPhase()) {
		if (!Logic->StartAircraftPhase()) {
			if (GetGameState()->CurrentPlaylistInfo.BasePlaylist->SafeZoneStartUp == ESafeZoneStartUp::StartsWithNoAirCraft) {
				if (!Logic->StartSafeZones(true, true) && GetGameState()->MapInfo)
					Logic->SetGamePhase(EAthenaGamePhase::None);
			}
		}
	}
	
	return HandleMatchHasStartedOG(GameMode);
}

void FortGameModeAthena::HandleStartingNewPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer) {
	auto PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);
	auto GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
	if (CurrentGamemode == Athena) {
		// that helps ig
		PlayerState->SquadID = PlayerState->TeamIndex - 3;
		PlayerState->OnRep_SquadId();
		
		FGameMemberInfo Member;
		Member.MostRecentArrayReplicationKey = -1;
		Member.ReplicationID = -1;
		Member.ReplicationKey = -1;
		Member.TeamIndex = PlayerState->TeamIndex;
		Member.SquadID = PlayerState->SquadID;
		Member.MemberUniqueId = PlayerState->UniqueID;

		GameState->GameMemberInfoArray.Members.Add(Member);
		GameState->GameMemberInfoArray.MarkItemDirty(Member);

		NotifyGameMemberAdded(GameState, Member.SquadID, Member.TeamIndex, &Member.MemberUniqueId);
		
		PlayerState->SeasonLevelUIDisplay = NewPlayer->XPComponent->CurrentLevel;
		PlayerState->OnRep_SeasonLevelUIDisplay();
	}

	// tuff 
	for (auto MutatorIG : GameMode->MutatorListComponent->Mutators)
	{
		if (MutatorIG->IsA(AJunoCoreMutators::StaticClass()))
		{
			auto mutator = (AJunoCoreMutators*)MutatorIG;
			std::cout << MutatorIG->GetName() << std::endl;
			mutator->bIsJunoPlayspaceReady = true;
			NewPlayer->bHasServerFinishedLoading = true;
			NewPlayer->OnRep_bHasServerFinishedLoading();
		}
	}
	
    if (CurrentGamemode != Juno)
    {
    	if (!NewPlayer->WorldInventory.GetObjectRef()) {
    		FActorSpawnParameters SpawnParams{};
    		SpawnParams.NameMode = Required_Fatal;
    		SpawnParams.ObjectFlags = EObjectFlags::Transactional;
    		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    		SpawnParams.bNoFail = true;

    		AFortInventory* Object = ((AFortInventory * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(ImageBase + 0x1A8087C))(GetWorld(), NewPlayer->WorldInventoryClass, new FTransform(FVector(), {}), &SpawnParams);
    		Object->SetOwner(NewPlayer);
    		NewPlayer->WorldInventory.InterfacePointer = Object->GetInterfaceAddress<IFortInventoryInterface>();
    		NewPlayer->WorldInventory.ObjectPointer = Object;
    		NewPlayer->bHasInitializedWorldInventory = true;
    	}
    }
	
	if (!NewPlayer->MatchReport && CurrentGamemode == Athena)
		NewPlayer->MatchReport = reinterpret_cast<UAthenaPlayerMatchReport*>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), NewPlayer));
	
	return HandleStartingNewPlayerOG(GameMode, NewPlayer);
}

EFortTeam FortGameModeAthena::PickTeam(AFortGameModeAthena* GameMode, uint8_t Team, AFortPlayerControllerAthena* PlayerController) {
	
	uint8_t returnvalue = CurrentTeam;
	if (++PlayersOnCurrentTeam >= ((AFortGameStateAthena*)GameMode->GameState)->CurrentPlaylistInfo.BasePlaylist->MaxSquadSize)
	{
		CurrentTeam++;
		PlayersOnCurrentTeam = 0;
	}

	return EFortTeam(returnvalue);
}

void FortGameModeAthena::Init()
{
	UHook* Hook = new UHook();

	Hook->Address = 0x124;
	Hook->Class = AGameModeBase::StaticClass();
	Hook->Detour = ReadyToStartMatch;
	Hook->Original = (void**)&ReadyToStartMatchOG;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Address = 0xE6;
	Hook->Class = AGameModeBase::StaticClass();
	Hook->Detour = SpawnDefaultPawnFor;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Address = 0xEC;
	Hook->Class = AGameModeBase::StaticClass();
	Hook->Detour = HandleStartingNewPlayer;
	Hook->Original = (void**)&HandleStartingNewPlayerOG;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Address = ImageBase + 0x8FCD25C;
	Hook->Detour = HandleMatchHasStarted;
	Hook->Original = (void**)&HandleMatchHasStartedOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = ImageBase + 0x8FD26F0;
	Hook->Detour = PickTeam;
	UKismetHookingLibrary::Hook(Hook, Address);

	delete Hook;
}