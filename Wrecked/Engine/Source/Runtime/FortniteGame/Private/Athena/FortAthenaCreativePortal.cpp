#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaCreativePortal.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

void FortAthenaCreativePortal::CreatePortal(AFortPlayerControllerAthena* PlayerController)
{
	if (!CurrentGamemode == Creative) return;
	
	auto GameMode = GetGameMode();
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;
	auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
	auto PortalManager = GameState->CreativePortalManager;
	AFortAthenaCreativePortal* PlayerPortal = nullptr;
	PlayerController->MaxPlotCount = 10; // set it to 10 when profile fails
	
	// loop through all portals to check for portal that doesnt have any replication bytes and then we assign to the player 
	for (auto Portal : PortalManager->AllPortals)
	{
		if (Portal->OwningPlayer.ReplicationBytes.Num() == 0 )
		{
			PlayerPortal = Portal;
			break;
		}
	}

	// i think its enough to make it work
	PlayerPortal->OwningPlayer = PlayerState->UniqueID;
	PlayerPortal->OnRep_OwningPlayer();
	PlayerPortal->bPortalOpen = true;
	PlayerPortal->OnRep_PortalOpen();

	PlayerPortal->bIsPublishedPortal = false;
	PlayerPortal->OnRep_PublishedPortal();


	// alr we will use this shit rn for players that are in that island ig
	//	PlayerPortal->PlayersReady.Add(PlayerPortal->OwningPlayer);
	//	PlayerPortal->OnRep_PlayersReady();

	// that will do the job
	UpdateAllPortals(PortalManager);
	
	
	PlayerController->OwnedPortal = PlayerPortal;
	PlayerController->CreativePlotLinkedVolume = PlayerPortal->LinkedVolume;
	PlayerController->OnRep_CreativePlotLinkedVolume();
	auto CreativePlotDef = StaticLoadObject<UFortCreativeRealEstatePlotItemDefinition>("/CR_Legacy/Playgrounds/Items/Plots/Shark_Large.Shark_Large");
	
	
	auto LevelSaveComponent = (UFortLevelSaveComponent*)PlayerPortal->GetLinkedVolume()->GetComponentByClass(UFortLevelSaveComponent::StaticClass());

	LevelSaveComponent->AccountIdOfOwner = PlayerState->UniqueID;
	LevelSaveComponent->bIsLoaded = true;
	LevelSaveComponent->RestrictedPlotDefinition = CreativePlotDef;
	LevelSaveComponent->bLoadPlaysetFromPlot = true;
	LevelSaveComponent->bAutoLoadFromRestrictedPlotDefinition = true;

	LevelSaveComponent->OnRep_AccountIdOfOwner();
	
	auto PlayspacePermissionComponent = (UPlayspaceComponent_CreativeToolsPermission*)PlayerPortal->GetLinkedVolume()->Playspace->GetComponentByClass(UPlayspaceComponent_CreativeToolsPermission::StaticClass());
	PlayspacePermissionComponent->AccountIdOfOwner = PlayerState->UniqueID;
	PlayspacePermissionComponent->PlotPermissions.Permission = EFortCreativePlotPermission::Private;
	PlayspacePermissionComponent->OnRep_PlotPermissionsChanged();

	auto LevelStreamComponent = (UPlaysetLevelStreamComponent*)PlayerPortal->GetLinkedVolume()->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass());
	LevelStreamComponent->SetPlayset(CreativePlotDef->BasePlayset);
	
	auto MinigameSettingsMachineClass = StaticLoadObject<UClass>("/Game/Athena/Items/Gameplay/MinigameSettingsControl/MinigameSettingsMachine.MinigameSettingsMachine_C");

	auto static MinigameSettingsMachine = GetWorld()->SpawnActor<AFortMinigameSettingsBuilding>(MinigameSettingsMachineClass,PlayerPortal->LinkedVolume->K2_GetActorLocation(), {}, PlayerPortal->LinkedVolume);
	
	MinigameSettingsMachine->SettingsVolume = PlayerPortal->LinkedVolume;
	MinigameSettingsMachine->OnRep_SettingsVolume();
	MinigameSettingsMachine->CreativeLinkComponent = (UFortCreativeVolumeLinkComponent*)PlayerPortal->GetLinkedVolume()->GetComponentByClass(UFortCreativeVolumeLinkComponent::StaticClass());

	auto MinigameVolumeComponent = (UFortMinigameVolumeComponent*)PlayerPortal->GetLinkedVolume()->GetComponentByClass(UFortMinigameVolumeComponent::StaticClass());
	MinigameVolumeComponent->CurrentMinigameSettingsMachine = MinigameSettingsMachine;
	

	
	// prob async one is better but idc
	 __int64 (*LoadPlaysetOG)(UPlaysetLevelStreamComponent*) = decltype(LoadPlaysetOG)(__int64(ImageBase + 0xA29DB8C));
	LoadPlaysetOG(LevelStreamComponent);
	
	PlayerPortal->LinkedVolume->VolumeState = ESpatialLoadingState::Ready;
	PlayerPortal->LinkedVolume->OnRep_VolumeState();
	
}


// alr so this whole shit can be made better but for now it works later i will need to make it load last island from the cloudstorage and spawn the playset but rn idc
void FortAthenaCreativePortal::TestProfile(AFortPlayerControllerAthena* PlayerController)
{
    auto State = PlayerController->CreativeModeProfile->InternalGetState();
	
    for (auto& ItemsByTypeArray : State->ItemsContainer.ItemsByType)
    {
        if (!ItemsByTypeArray.Second)
            continue;
            
        for (auto& Item : ItemsByTypeArray.Second)
        {
            if (!Item)
                continue;
                
          //  std::cout << "InstanceID: " << Item->InstanceId.ToStr().c_str() << std::endl
          //  std::cout << "TemplateID: " << Item->TemplateId.ToStr().c_str() << std::endl;
                
            FCreativeIslandData Island;
            Island.McpId = Item->InstanceId;
            Island.bIsDeleted = false;
            Island.bIsVKProject = false;

            auto CreativePlotDef = StaticLoadObject<UFortCreativeRealEstatePlotItemDefinition>("/CR_Legacy/Playgrounds/Items/Plots/" + Item->TemplateId.ToString() + "." + Item->TemplateId.ToString());
        	
            for (auto& Attribute : Item->Attributes.Attrs)
            {
                if (!Attribute.Second.IsValid())
                    continue;
                
                void** vtable = *(void***)Attribute.Second.Get();
                FString AttributeName = Attribute.First;
                
                switch (Attribute.Second->Type)
                {
                case EJson::String:
                {
                    auto TryGetString = (bool (__fastcall *)(FJsonValue*, FString*))vtable[11];
                    FString Value;
                    if (TryGetString(Attribute.Second.Get(), &Value))
                    {
                        std::string attrName = AttributeName.ToStr().c_str();
                    	
                        
                        if (attrName == "title")
                        {
                            Island.IslandName = UKismetTextLibrary::Conv_StringToText(Value);
                          //  std::cout << "  Title: " << Value.ToStr().c_str() << std::endl;
                        }
                        else if (attrName == "last_used_date")
                        {
                        	FDateTime Result;
                        	std::wstring WideValue(Value.CStr());
                        	if (!WideValue.empty() && WideValue.back() == L'Z')
                        	{
                        		WideValue.pop_back();
                        	}
                        	FString CleanedValue(WideValue.c_str());

                        	if (UKismetMathLibrary::DateTimeFromIsoString(CleanedValue, &Result))
                        		Island.LastLoadedDate = Result;
                        	
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
            PlayerController->CreativeIslands.Add(Island);
        }
    }

	if (PlayerController->CreativeIslands.Num() > 0)
    {
        PlayerController->OnRep_CreativeIslands();
    }

	// max plot count setup from the profile
	// fallback if no stat for max plot count and last used plot
	for (auto& Stat: State->StatsContainer.Attrs)
	{
		std::string attrName = Stat.First.ToStr().c_str();
		void** vtable = *(void***)Stat.Second.Get();
		if (attrName == "max_plot_count")
		{
			auto TryGetNumber = (bool (__fastcall *)(FJsonValue*, double*))vtable[10];
			double NumberValue;
			if (TryGetNumber(Stat.Second.Get(), &NumberValue))
			{
				int32 IntValue = (int32)NumberValue;
				PlayerController->MaxPlotCount = IntValue;
			}
		}
		else if (attrName == "last_used_plot")
		{
			auto TryGetString = (bool (__fastcall *)(FJsonValue*, FString*))vtable[11];
			FString Value;

			UE_LOG(LogCreative, Log, L"Last Used Plot Found");
			if (TryGetString(Stat.Second.Get(), &Value))
			{
			auto lastisland = PlayerController->CreativeIslands.Search([Value](FCreativeIslandData& IslandData)
			{
				return IslandData.McpId == Value;
			});
				// thats the portal information i think not the in playset information when u open the map
				PlayerController->OwnedPortal->IslandInfo.AltTitle = lastisland->IslandName;
				PlayerController->OwnedPortal->IslandInfo.CreatorName = PlayerController->PlayerState->GetPlayerName();
				PlayerController->OwnedPortal->IslandInfo.SupportCode = L"Test Support Code";
				PlayerController->OwnedPortal->OnRep_IslandInfo();
				std::cout << lastisland->IslandName.ToString().c_str() << std::endl;
			}
		}
	}
	
}


void MakeNewCreativePlot(UObject* Context, FFrame& Stack)
{
	UFortCreativeRealEstatePlotItemDefinition* PlotType = nullptr;
	 
    Stack.StepCompiledIn(&PlotType);
	auto& Locale = Stack.StepCompiledInRef<FString>();
	auto& Title = Stack.StepCompiledInRef<FString>();
	Stack.IncrementCode();

	 Title = Title.ToString().empty() ? UKismetTextLibrary::Conv_TextToString(PlotType->GetDisplayName()) : Title;

	auto PlayerController = (AFortPlayerControllerAthena*)Context;
	
	PlayerController->CreativeModeProfile->CreateNewIsland(UKismetStringLibrary::Conv_NameToString(PlotType->Name), Locale, Title, nullptr);



	FCreativeIslandData NewPlot;
	NewPlot.bIsDeleted = false;
	NewPlot.bIsVKProject = false;
	NewPlot.IslandName = UKismetTextLibrary::Conv_StringToText(Title);
	NewPlot.LastLoadedDate = UKismetMathLibrary::UtcNow();
	//NewPlot.McpId = nullptr; // pretty much useless rn bc backend generates it and its not passed on the creation
	
	PlayerController->CreativeIslands.Add(NewPlot);
	PlayerController->OnRep_CreativeIslands();
	
	PlayerController->ClientBroadcastOnMakeNewCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L"")); 
}

void DestroyCreativePlot(UObject* Context, FFrame& Stack)
{
	FString IslandId;
	Stack.StepCompiledIn(&IslandId);
	Stack.IncrementCode();
	
	auto PlayerController = (AFortPlayerControllerAthena*)Context;
	auto IslandToDestroy = PlayerController->CreativeIslands.Search([IslandId](FCreativeIslandData& IslandData)
	{
	    return IslandData.McpId == IslandId;
    });
	IslandToDestroy->bIsDeleted = true;
	IslandToDestroy->DeletedAt = UKismetMathLibrary::UtcNow();
	
	PlayerController->OnRep_CreativeIslands();
	PlayerController->CreativeModeProfile->DeleteIsland(IslandToDestroy->McpId, nullptr); // same issue could be as below
	PlayerController->ClientBroadcastOnDestroyCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
}

void RestoreCreativePlot(UObject* Context, FFrame& Stack)
{
	FString IslandId;
	Stack.StepCompiledIn(&IslandId);
	Stack.IncrementCode();
	
	auto PlayerController = (AFortPlayerControllerAthena*)Context;
	auto IslandToRestore = PlayerController->CreativeIslands.Search([IslandId](FCreativeIslandData& IslandData)
	{
		return IslandData.McpId == IslandId;
	});
	if (IslandToRestore->bIsDeleted)
		IslandToRestore->bIsDeleted = false;
		PlayerController->OnRep_CreativeIslands(); // we prob dont need onrep when we use client broadcast but idk
	
	PlayerController->ClientBroadcastOnRestoreCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
	PlayerController->CreativeModeProfile->RestoreDeletedIsland(IslandToRestore->McpId, nullptr); // mcpid might be bad but idk
}

void DuplicateCreativePlot(UObject* Context, FFrame& Stack)
{
	FString IslandId;
	FString locale;
	FString Title;
	Stack.StepCompiledIn(&IslandId);
	Stack.StepCompiledIn(&locale);
	Stack.StepCompiledIn(&Title);
	Stack.IncrementCode();
	
	auto PlayerController = (AFortPlayerControllerAthena*)Context;


	// search for the plot with the island id and duplicate it alr bro
	auto IslandToDuplicate = PlayerController->CreativeIslands.Search([IslandId](FCreativeIslandData& IslandData)
    {
	   return IslandData.McpId == IslandId;
    });
	FCreativeIslandData NewIsland;
	NewIsland.bIsDeleted = false;
	NewIsland.bIsVKProject = false;
	NewIsland.IslandName = UKismetTextLibrary::Conv_StringToText(Title);
	NewIsland.LastLoadedDate = UKismetMathLibrary::UtcNow();

	PlayerController->CreativeIslands.Add(NewIsland);
	PlayerController->OnRep_CreativeIslands();
	
	PlayerController->CreativeModeProfile->DuplicateIsland(IslandId, locale, Title, nullptr);
	PlayerController->ClientBroadcastOnDuplicateCreativePlotFinished(true, UKismetTextLibrary::Conv_StringToText(L""));
}

void ServerUpdateCreativePlotName(UObject* Context, FFrame& Stack)
{
	FString IslandId;
	FString locale;
	FString Title;
	Stack.StepCompiledIn(&IslandId);
	Stack.StepCompiledIn(&locale);
	Stack.StepCompiledIn(&Title);
	Stack.IncrementCode();
	
	auto PlayerController = (AFortPlayerControllerAthena*)Context;
	
	auto IslandToEdit = PlayerController->CreativeIslands.Search([IslandId](FCreativeIslandData& IslandData)
	{
		 return IslandData.McpId == IslandId;
	});
	IslandToEdit->IslandName = UKismetTextLibrary::Conv_StringToText(Title);
	PlayerController->OnRep_CreativeIslands();

	// i cant find a func to update the plot name somehow
	// maybe we need a diff way
	PlayerController->CreativeModeProfile->SetCreativePlotMetadata(IslandId, locale, Title, FString(), TArray<FString>(), FString(), nullptr);
	PlayerController->ClientBroadcastOnUpdateCreativePlotName(true, UKismetTextLibrary::Conv_StringToText(L""));
}



void TeleportPlayerToLinkedVolumeHook(UObject* Context, FFrame& Stack)
{
	AFortPlayerPawn* Pawn = nullptr;
	bool bUseSpawnTags;

	Stack.StepCompiledIn(&Pawn);
	Stack.StepCompiledIn(&bUseSpawnTags);
	Stack.IncrementCode();

	auto Portal = (AFortAthenaCreativePortal*)Context;


	auto Loc = Portal->LinkedVolume->K2_GetActorLocation();
	Loc.Z = 10000; 

	Pawn->K2_TeleportTo(Loc, FRotator());
	Pawn->BeginSkydiving(false);
	
	Portal->CurrentPopulation++;
	Portal->OnRep_PopulationChanged();
}
// that thing is not proper same as tp to the island
void ServerTeleportToPlaygroundLobbyIsland(UObject* Context, FFrame& Stack)
{
	auto GameMode = GetGameMode();
	auto PlayerController = (AFortPlayerControllerAthena*)Context;
	if (!PlayerController) return;

	// thats prob not proper but idc
	// so um this works but we need to get the players info like where the pawn is
	//auto idk = PlayerController->GetCurrentExperienceType();
	
	AActor* SpawnPoint = GameMode->ChoosePlayerStart(PlayerController); // not proper we should do creative starts bc some maps have starts 
	PlayerController->MyFortPawn->K2_TeleportTo(SpawnPoint->K2_GetActorLocation(), SpawnPoint->K2_GetActorRotation());
	// and prob set here that player is in the hub so the client knows it
}

// class AFortAthenaCreativePortal* Portal, const class FString& PlotItemId
void ServerLoadPlotForPortal(UObject* Context, FFrame& Stack)
{
	AFortAthenaCreativePortal* Portal = nullptr;
	FString PlotItemId;
	Stack.StepCompiledIn(&Portal);
	Stack.StepCompiledIn(&PlotItemId);
	Stack.IncrementCode();
	

	std::cout << "ServerLoadPlotForPortal" << std::endl;
}

// HandleProfileUpdate
__int64 FortAthenaCreativePortal::test(UMcpProfile* a1, FProfileUpdateSingle* a2, __int64 a3, void *a4, char a5)
{
	std::cout << "WTF EVEN IS THIS" << std::endl;
	
	
	std::cout << a1->ProfileId.ToStr().c_str() << std::endl;
	return testOG(a1, a2,  a3, a4,  a5);
}

// maybe theres a func to update them automaticly but idc
// + its not proper bc it doesnt see some shit so refactor that later
void FortAthenaCreativePortal::UpdateAllPortals(AFortCreativePortalManager* PortalManager)
{
	TArray<AFortPlayerController*> Controllers = UFortKismetLibrary::GetAllFortPlayerControllers(GetWorld(),true, true );
	for (auto Portal : PortalManager->AllPortals)
	{
		if (Portal->OwningPlayer.ReplicationBytes.Num() != 0)
		{
			Portal->MaxAvailablePopulation = Controllers.Num();
		}
	}
}

void FortAthenaCreativePortal::Init() {
	UHook* Hook = new UHook();
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.MakeNewCreativePlot";
	Hook->Detour = MakeNewCreativePlot;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume";
	Hook->Detour = TeleportPlayerToLinkedVolumeHook;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland";
	Hook->Detour = ServerTeleportToPlaygroundLobbyIsland;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.DestroyCreativePlot";
	Hook->Detour = DestroyCreativePlot;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.RestoreCreativePlot";
	Hook->Detour = RestoreCreativePlot;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.DuplicateCreativePlot";
	Hook->Detour = DuplicateCreativePlot;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.ServerUpdateCreativePlotName";
	Hook->Detour = ServerUpdateCreativePlotName;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	//Hook->Address = ImageBase + 0x7BFBC68;
	//Hook->Detour = test;
	//Hook->Original = (void**)&testOG;
	//UKismetHookingLibrary::Hook(Hook, Address);
	
	Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.ServerLoadPlotForPortal";
	Hook->Detour = ServerLoadPlotForPortal;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	delete Hook;
} 