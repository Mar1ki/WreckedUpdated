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
							UE_LOG(LogGameMode, Log, "Spawning Cactus at (%d, %d, %d)", ContentCenter.X, ContentCenter.Y, ContentCenter.Z);
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
							UE_LOG(LogGameMode, Log, "Spawning Cactus at (%d, %d, %d)", ContentCenter.X, ContentCenter.Y, ContentCenter.Z);
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
			GameMode->AIDirector = (AFortAIDirector*)GetWorld()->SpawnActor(AFortAIDirector::StaticClass(), {});

		GameMode->AIDirector->Activate();

		if (!GameMode->AIGoalManager)
			GameMode->AIGoalManager = (AFortAIGoalManager*)GetWorld()->SpawnActor(AFortAIGoalManager::StaticClass(), {});
	}

	if (CurrentGamemode == Athena && GameState->Teams.Num() == 0) return false;
	static bool bFirst = false;
	if (!bFirst)
	{
		FURL Url{};
		Url.Port = 7777;
		GetWorld()->Listen(Url);
		GameMode->bWorldIsReady = true;
		if (CurrentGamemode == Athena) {
			static void* (*InitializeFlightPath)(AFortAthenaMapInfo*, AFortGameStateAthena*, UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, bool, double, float, float) = decltype(InitializeFlightPath)(ImageBase + 0x8F454DC);
			InitializeFlightPath(GameState->MapInfo, GameState, UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(GetWorld()), false, 0.f, 0.f, 0.f);
		}
		bFirst = true;
	}

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
	UE_LOG(LogTemp, Log, "Spawning pawn!");
	if (CurrentGamemode == Athena)
	{
		SpawnFloorLoot();
		SpawnCactuses();
		FortAthenaVehicle::SpawnVehicles();
	}
	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AJunoTeamStart::StaticClass(), &Starts);
	FTransform Transform = Starts.Num() > 0 ? Starts[0]->GetTransform() : StartSpot->GetTransform();
	auto Pawn = (AFortPlayerPawnAthena*)GameMode->SpawnDefaultPawnAtTransform(NewPlayer, Transform);
	AFortInventory* Inventory = (AFortInventory*)NewPlayer->WorldInventory.GetObjectRef();
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

class UFunction* FindFunctionByFName(UClass *Class, FName FuncName)
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

void BotEnterAircraft(AFortAthenaAIBotController* Controller, AFortAthenaAircraft* InAircraft)
{
	// Enter aircraft through aircraft component
	UFortControllerComponent_Aircraft* AircraftComponent =
		(UFortControllerComponent_Aircraft*)Controller->GetComponentByClass(
			UFortControllerComponent_Aircraft::StaticClass());

	if (AircraftComponent && !AircraftComponent->CurrentAircraft)
	{
		void (*EnterAircraftOG)(UFortControllerComponent_Aircraft * Comp, AFortAircraft * InAircraft) = decltype(EnterAircraftOG)(ImageBase + 0x9505D40);
		EnterAircraftOG(AircraftComponent, InAircraft);
	}

	// Handle thank bus driver logic
	if (Controller->CachedAIServicePlayerBots)
	{
		UWorld* World = GetWorld();
		if (World && GetGameState())
		{
			// Check if game state is AFortGameStateAthena
			if (GetGameState()->IsA(AFortGameStateAthena::StaticClass()))
			{
				float ThankProbability = Controller->CachedAIServicePlayerBots->ThankBusDriverProbability.Evaluate();

				if (ThankProbability > 0 && (rand() & 0x7FFF) < (ThankProbability * 32767.0f))
				{
					UAthenaAIServicePlayerBots* AIService = Controller->CachedAIServicePlayerBots;
					float MaxTime = AIService->ThankBusDriverMaxTime.Evaluate();
					float MinTime = AIService->ThankBusDriverMinTime.Evaluate();

					float RandomTime = MinTime + ((rand() & 0x7FFF) / 32767.0f) * (MaxTime - MinTime);

					if (RandomTime > 0.0f)
					{

					}
				}
			}
		}
	}

	UBlackboardComponent* BlackboardComp = (UBlackboardComponent*)Controller->Blackboard;
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsBool(UKismetStringLibrary::Conv_StringToName(TEXT("AIEvaluator_Global_IsInBus")), true);
	}
}
void AFortGameModeAthena::StartAircraftPhase()
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Logic = GetLogic();

	auto Time = UGameplayStatics::GetTimeSeconds(GetWorld());
	auto Aircraft = Logic->Aircrafts_GameState[0].Get();
	auto GameState = (AFortGameStateAthena*)GetWorld()->GameState;
	auto& FlightInfo = GameState->MapInfo->FlightInfos[0];
	Aircraft->FlightElapsedTime = 0;
	Aircraft->DropStartTime = Time + FlightInfo.TimeTillDropStart;
	Aircraft->DropEndTime = Time + FlightInfo.TimeTillDropEnd;
	Aircraft->FlightStartTime = Time;
	Aircraft->FlightEndTime = Time + FlightInfo.TimeTillFlightEnd;
	Aircraft->ReplicatedFlightTimestamp = Time;
	Aircraft->StartFlightPath(Time);

	for (auto& Player : AlivePlayers)
	{
		if (!Player)
			continue;
		
		auto Pawn = Player->MyFortPawn;
		Player->LastDamager = 0;
		Player->LastFallInstigator = 0;
		Pawn->ServerInternalEquipWeapon(0);
		Pawn->bIsInsideSafeZone = true;
		Player->GetAircraftComponent()->CurrentAircraft = Aircraft;
		Player->GetAircraftComponent()->ClientEnterAircraft(Aircraft);
		Player->GetAircraftComponent()->PlacePlayerInAircraft(Aircraft, false);
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
		Player->ClientGotoState(UKismetStringLibrary::Conv_StringToName(L"Spectating"));
	}
	for (auto& Player : AliveBots)
	{
		BotEnterAircraft(Player, Aircraft);
	}

	Logic->bAircraftIsLocked = true;
	Logic->WarmupCountdownEndTime = Time;
	Logic->OnRep_WarmupCountdownEndTime();
	Logic->SetGamePhase(EAthenaGamePhase::Aircraft);
}

void FortGameModeAthena::HandleMatchHasStarted(AFortGameModeAthena* GameMode)
{
	UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Logic = GetLogic();
	static bool bWarmupSet = false;
	if (CurrentGamemode != EGameModeType::Juno)
	{
		if (!bWarmupSet)
		{
			bWarmupSet = true;
    	
			auto Time = UGameplayStatics::GetTimeSeconds(GetWorld());
			Logic->WarmupCountdownStartTime = Time;
			Logic->WarmupCountdownEndTime = Time + WarmupDuration;
			Logic->WarmupCountdownDuration = WarmupDuration;
			Logic->WarmupEarlyCountdownDuration = 0.f;
			Logic->OnRep_WarmupCountdownEndTime();
    	
			AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
			if (CurrentGamemode == Athena)
			{
				int NumToSpawn = 1;
				if (GameState->CurrentPlaylistInfo.BasePlaylist->AirCraftBehavior == EAirCraftBehavior::OpposingAirCraftForEachTeam)
					NumToSpawn = GameState->TeamCount;

				TArray<AFortAthenaAircraft*> Aircrafts;
				for (int i = 0; i < NumToSpawn; i++)
					Aircrafts.Add(AFortAthenaAircraft::SpawnAircraft(GetWorld(), GameState->MapInfo->AircraftClass, GameState->MapInfo->FlightInfos[i]));
    	
				Logic->SetAircrafts(Aircrafts);
				Logic->SetGamePhase(EAthenaGamePhase::Warmup);
				Logic->OnRep_Aircrafts();
			}

			if (CurrentGamemode == EGameModeType::Papaya)
			{
				Logic->SetGamePhase(EAthenaGamePhase::Setup); // ig bro
			}
			if (CurrentGamemode == Creative)
			{
				Logic->SetGamePhase(EAthenaGamePhase::Setup);
			}
		}
	}
	
	return HandleMatchHasStartedOG(GameMode);
}

void FortGameModeAthena::HandleStartingNewPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer) {
	std::cout << "called handle player" << std::endl;
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

		PlayerState->SeasonLevelUIDisplay = NewPlayer->XPComponent->CurrentLevel;
		PlayerState->OnRep_SeasonLevelUIDisplay();
	}

	// tuff 
	for (auto MutatorIG : 	GameMode->MutatorListComponent->Mutators)
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

void FortGameModeAthena::StartNewSafeZonePhase(AFortGameModeAthena* GameMode)
{
	AFortGameStateAthena* GameState = (AFortGameStateAthena*)GameMode->GameState;
	AFortSafeZoneIndicator* SafeZoneIndicator = GetLogic()->SafeZoneIndicator;
	
	int32 CurrentPhase = SafeZoneIndicator->CurrentPhase;
	
	const float Time = (float)UGameplayStatics::GetTimeSeconds(GameState);
	const float ServerTime = GameState->ServerWorldTimeSecondsDelta + Time;
	
	FFortSafeZonePhaseInfo PrevPhase = SafeZoneIndicator->SafeZonePhases[CurrentPhase];
	SafeZoneIndicator->PreviousCenter = FVector_NetQuantize100(PrevPhase.Center);
	SafeZoneIndicator->PreviousRadius = PrevPhase.Radius;
	
	CurrentPhase++;
	
	FFortSafeZonePhaseInfo NextPhase = SafeZoneIndicator->SafeZonePhases[CurrentPhase];

	SafeZoneIndicator->NextCenter = FVector_NetQuantize100(NextPhase.Center);
	SafeZoneIndicator->NextRadius = NextPhase.Radius;
	SafeZoneIndicator->NextMegaStormGridCellThickness = NextPhase.MegaStormGridCellThickness;

	if (SafeZoneIndicator->SafeZonePhases.IsValidIndex(CurrentPhase + 1))
	{
		FFortSafeZonePhaseInfo& NextPhaseInfo = SafeZoneIndicator->SafeZonePhases[CurrentPhase + 1];

		SafeZoneIndicator->NextNextCenter = FVector_NetQuantize100(NextPhaseInfo.Center);
		SafeZoneIndicator->NextNextRadius = NextPhaseInfo.Radius;
		SafeZoneIndicator->NextNextMegaStormGridCellThickness = NextPhaseInfo.MegaStormGridCellThickness;
	}

	SafeZoneIndicator->SafeZoneStartShrinkTime = ServerTime + NextPhase.WaitTime;
	SafeZoneIndicator->SafeZoneFinishShrinkTime = SafeZoneIndicator->SafeZoneStartShrinkTime + NextPhase.ShrinkTime;

	SafeZoneIndicator->CurrentDamageInfo = NextPhase.DamageInfo;
	SafeZoneIndicator->OnRep_CurrentDamageInfo();

	SafeZoneIndicator->CurrentPhase = CurrentPhase;
	SafeZoneIndicator->OnRep_CurrentPhase();
	
	GetLogic()->GamePhaseStep = EAthenaGamePhaseStep::StormHolding;
	GetLogic()->HandleGamePhaseStepChanged(EAthenaGamePhaseStep::StormHolding);
}

void FortGameModeAthena::InitialSafeZone(AFortGameModeAthena* GameMode)
{
    AFortGameStateAthena* GameState = (AFortGameStateAthena*)GameMode->GameState;
    
    auto Logic = GetLogic();
    
    if (Logic->SafeZoneIndicator)
        return;

	FFortSafeZoneDefinition Definition = GameState->MapInfo->SafeZoneDefinition;
    float ZoneCount = Definition.Count.Evaluate();
	
    TArray<float> Radii;
    for (float i = 0; i < ZoneCount; i++)
        Radii.Add(Definition.Radius.EvaluateIndex(i));
    
    unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
    FortAthenaMapInfo::PrepareStormCircles(
        GameState->MapInfo, 
        seed,
        Radii
    );
	
    UClass* IndicatorClass = Logic->SafeZoneIndicatorClass;
    AFortSafeZoneIndicator* Indicator = GetWorld()->SpawnActor<AFortSafeZoneIndicator>(IndicatorClass, FVector{});
	
    if (!Indicator)
        return;
    
    if (Indicator->SafeZonePhases.IsValid())
    	Indicator->SafeZonePhases.Free();
    
    const float Time = (float)UGameplayStatics::GetTimeSeconds(GameState);
    const float ServerTime = GameState->ServerWorldTimeSecondsDelta + Time;
    
    for (float i = 0; i < ZoneCount; i++)
    {
        FFortSafeZonePhaseInfo Phase{};
        
        Phase.Radius = Radii[(int)i]; 
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
        Phase.Center = FortAthenaMapInfo::StormCircles[(int)i];
        
        UE_LOG(LogStorm, Log, "Phase %.0f - Radius: %.2f, WaitTime: %.2f, ShrinkTime: %.2f, PlayerCap: %d, Center: (%.2f, %.2f, %.2f), MegaStormGridCellThickness: %d",
               i, Phase.Radius, Phase.WaitTime, Phase.ShrinkTime, Phase.PlayerCap,
               Phase.Center.X, Phase.Center.Y, Phase.Center.Z, Phase.MegaStormGridCellThickness);
        
        Indicator->SafeZonePhases.Add(Phase);
        Indicator->PhaseCount++;
    }
	
    Indicator->OnRep_PhaseCount();
    Indicator->SetSafeZonePhases(Indicator->SafeZonePhases);
    
    Indicator->SafeZoneStartShrinkTime = ServerTime + Indicator->SafeZonePhases[0].WaitTime;
    Indicator->SafeZoneFinishShrinkTime = Indicator->SafeZoneStartShrinkTime + Indicator->SafeZonePhases[0].ShrinkTime;
    Indicator->CurrentPhase = 0;
    Indicator->OnRep_CurrentPhase();
	
    Logic->SafeZoneIndicator = Indicator;
    Logic->SafeZoneIndicator->OnSafeZoneStateChange(EFortSafeZoneState::Starting, true);
    Logic->GamePhaseStep = EAthenaGamePhaseStep::StormHolding;
    Logic->OnRep_SafeZoneIndicator();
    Logic->HandleGamePhaseStepChanged(EAthenaGamePhaseStep::StormHolding);

	StartNewSafeZonePhase(GameMode);
}

bool FortGameModeAthena::LoadMap(UEngine* Engine ,FWorldContext& WorldContext, FURL URL, class UPendingNetGame* Pending, FString& Error) {
	auto res = LoadMapOG(Engine, WorldContext, URL, Pending, Error);
	if (res) 
		GetWorld()->ServerStreamingLevelsVisibility = Cast<AServerStreamingLevelsVisibility>(GetWorld()->SpawnActor(AServerStreamingLevelsVisibility::StaticClass(), {}));

	return res;
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

	Hook->Address = ImageBase + 0x24B61E4;
	Hook->Detour = LoadMap;
	Hook->Original = (void**)&LoadMapOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	Hook->Address = ImageBase + 0x8FD26F0;
	Hook->Detour = PickTeam;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	delete Hook;
}