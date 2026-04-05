#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Player/FortPlayerController.h"
#include "Engine/Plugins/LateGameLibrary/Public/LateGamelibrary.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Plugins/PapayaManagerLibrary/Private/PapayaManager.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldManager.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaVehicle.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Movement/FortMovementMode_BaseExtLogic.h"
#include "SDK/SDK/VaultingCodeRuntime_classes.hpp"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaCreativePortal.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortQuestManager.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPlayerAugmentItemDefinition.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldEvent.h"
void FortPlayerController::ServerAcknowledgePossession(AFortPlayerControllerAthena* Controller, AFortPawn* Pawn)
{
    auto PlayerState = (AFortPlayerStateZone*)Controller->PlayerState;
    Controller->AcknowledgedPawn = Pawn;

    if (Controller->CosmeticLoadoutPC.Character)
    {
        PlayerState->HeroType = Controller->CosmeticLoadoutPC.Character->HeroDefinition;
        UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);
    }
    
    reinterpret_cast<void(*)(void*)>(ImageBase + 0x993F158)(PlayerState->GetInterfaceAddress<IAbilitySystemInterface>()); // ability stuff

    if (CurrentGamemode == EGameModeType::Athena || CurrentGamemode == EGameModeType::BlastBerry) {
        Controller->XPComponent->bRegisteredWithQuestManager = true;
        Controller->XPComponent->OnRep_bRegisteredWithQuestManager();
        Controller->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(Pawn);
        
        auto Comp = (UFortControllerComponent_MapDiscoverability*)Controller->GetComponentByClass(UFortControllerComponent_MapDiscoverability::StaticClass());
        TArray<AActor*> Actors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPoiVolume::StaticClass(), &Actors);
        for (auto Actor : Actors) {
            auto PoiVolume = (AFortPoiVolume*)Actor;
            void(__fastcall * HandleInitializedPOIVolume)(UFortControllerComponent_MapDiscoverability*, AFortPoiVolume*) = decltype(HandleInitializedPOIVolume)(ImageBase + 0x8EA5F18);
            HandleInitializedPOIVolume(Comp, PoiVolume);
        }
    }
}

void FortPlayerController::ServerPlayEmoteItem(AFortPlayerController* PlayerController, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber)
{

    if (!PlayerController || !EmoteAsset || !PlayerController->MyFortPawn)
        return;

    static UClass* DanceAbility = StaticLoadObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
    static UClass* SprayAbility = StaticLoadObject<UClass>("/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");
    static UClass* GroupEmoteAbility = StaticLoadObject<UClass>("/Game/Abilities/Emotes/GAB_GroupEmoteRequest.GAB_GroupEmoteRequest_C");

    if (!DanceAbility || !SprayAbility)
        return;

    auto EmoteDef = (UAthenaDanceItemDefinition*)EmoteAsset;
    if (!EmoteDef)
        return;

    PlayerController->MyFortPawn->bMovingEmote = EmoteDef->bMovingEmote;
    PlayerController->MyFortPawn->EmoteWalkSpeed = EmoteDef->WalkForwardSpeed;
    PlayerController->MyFortPawn->bMovingEmoteForwardOnly = EmoteDef->bMoveForwardOnly;
    PlayerController->MyFortPawn->EmoteWalkSpeed = EmoteDef->WalkForwardSpeed;
    
    PlayerController->MyFortPawn->LastEmoteItemDef = EmoteDef;
    PlayerController->MyFortPawn->LastReplicatedEmoteExecuted = EmoteDef;
    PlayerController->MyFortPawn->OnRep_LastReplicatedEmoteExecuted(PlayerController->MyFortPawn->LastReplicatedEmoteExecuted);

    FGameplayAbilitySpec Spec{};
    UGameplayAbility* Ability = nullptr;
    if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass()))
    {
        Ability = (UGameplayAbility*)SprayAbility->DefaultObject;
    }
    if (EmoteDef->bGroupEmote == true)
    {
        std::cout << "group emote" << std::endl;
        Ability = (UGameplayAbility*)GroupEmoteAbility->DefaultObject;
    }
    if (Ability == nullptr) {
        Ability = (UGameplayAbility*)DanceAbility->DefaultObject;
    }
    if (Ability) {
        static __int64 (*SpecConstructor)(FGameplayAbilitySpec*, UObject*, int, int, UObject*) = decltype(SpecConstructor)(ImageBase + 0x7B28E6C);
        static FGameplayAbilitySpecHandle(*GiveAbilityAndActivateOnce)(UAbilitySystemComponent * ASC, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec, __int64) = decltype(GiveAbilityAndActivateOnce)(ImageBase + 0x7AEF168);
        FGameplayAbilitySpec Spec{};
        SpecConstructor(&Spec, Ability, 1, -1, EmoteAsset);
        GiveAbilityAndActivateOnce(((AFortPlayerStateAthena*)PlayerController->PlayerState)->AbilitySystemComponent, &Spec.Handle, Spec, 0);
    }
}

void FortPlayerController::MovingEmoteStopped(UObject* Context, FFrame& Stack)
{
    Stack.IncrementCode();

    AFortPawn* Pawn = (AFortPawn*)Context;
    Pawn->bMovingEmote = false;
    Pawn->bMovingEmoteForwardOnly = false;
    Pawn->bMovingEmoteFollowingOnly = false;
}

void FortPlayerController::ServerAttemptInventoryDrop(AFortPlayerController* Controller, const struct FGuid& ItemGuid, int32 Count, bool bTrash)
{
    if (Count < 1)
        return;
    auto Inventory = (AFortInventory*)Controller->WorldInventory.ObjectPointer;
    if (!Inventory)
        return;

    auto Entry = Inventory->GetItemEntryByGuid(ItemGuid);
    if (!Entry)
        return;
    if (Entry->Count < Count)
        return;
    
    FortKismetLibrary::SpawnPickup(Controller->Pawn->K2_GetActorLocation(), *Entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::TossedByPlayer, Controller->MyFortPawn, Count, true, false, false);
    Entry->Count -= Count;
    Inventory->ReplaceEntry(*Entry);
    if (Entry->ItemDefinition->IsA(UFortPlayerAugmentItemDefinition::StaticClass()))
    {
        FortPlayerAugmentItemDefinition::OnItemInstanceRemoved((UFortPlayerAugmentItemDefinition*)Entry->ItemDefinition, Controller->GetInterfaceAddress<IFortInventoryOwnerInterface>(), 1);
    }
}

// TODO: switch to AFortPlayerControllerAthena::OnPawnDied (proper func, fixes some issues)
void FortPlayerController::ClientOnPawnDied(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport& DeathReport) {

    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    auto GameState = (AFortGameStateAthena*)GameMode->GameState;
    auto GamePhaseLogic = GetLogic();
    if (!PlayerController || GamePhaseLogic->GamePhase == EAthenaGamePhase::Warmup)
        return ClientOnPawnDiedOG(PlayerController, DeathReport);

    if (auto Projectile = DeathReport.DamageCauser.Get() ? Cast<AFortProjectileBase>(DeathReport.DamageCauser.Get()) : nullptr)
    {
        auto Weapon = Projectile->GetOwnerWeapon();
        if (Weapon) {
            if (Weapon->WeaponData) {
                for (auto Tag : Weapon->WeaponData->GameplayTags.GameplayTags) {
                    DeathReport.Tags.GameplayTags.Add(Tag);
                }
            }
        }
    }

    auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;

    auto KillerPlayerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;
    auto KillerPawn = (AFortPlayerPawnAthena*)DeathReport.KillerPawn.Get();
    auto KillerPlayerController = (AFortPlayerControllerAthena*)KillerPawn->GetController();

    PlayerState->PawnDeathLocation = PlayerController->MyFortPawn ? PlayerController->MyFortPawn->K2_GetActorLocation() : FVector();
    //PlayerState->DeathInfo.bDBNO = PlayerController->MyFortPawn ? PlayerController->MyFortPawn->IsDBNO() : false;
    PlayerState->DeathInfo.bDBNO = PlayerController->MyFortPawn->bWasDBNOOnDeath;
    PlayerState->DeathInfo.DeathLocation = PlayerState->PawnDeathLocation;
    PlayerState->DeathInfo.DeathTags = DeathReport.Tags;
    PlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(PlayerState->DeathInfo.DeathTags, PlayerState->DeathInfo.bDBNO);
    if (PlayerState->DeathInfo.bDBNO)
        PlayerState->DeathInfo.Downer = KillerPlayerState;
    PlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState;


    PlayerState->DeathInfo.bInitialized = true;
    PlayerState->OnRep_DeathInfo();

    bool bWasVictory = false;

    if (GameMode->AlivePlayers.Num() < 1 || GameMode->AlivePlayers.Num() == 1)
    {
        bWasVictory = true;
    }
    if (KillerPlayerState && KillerPawn && KillerPawn->Controller && KillerPawn->Controller->IsA(AFortPlayerControllerAthena::StaticClass()) && KillerPawn->Controller != PlayerController)
    {
        KillerPlayerState->KillScore++;
        KillerPlayerState->OnRep_Kills();
        KillerPlayerState->TeamKillScore++;
        KillerPlayerState->OnRep_TeamKillScore();

        KillerPlayerState->ClientReportKill(PlayerState);
        KillerPlayerState->ClientReportTeamKill(KillerPlayerState->TeamKillScore);

        auto KillerPC = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;
    }
    if (GameState->IsRespawningAllowed(PlayerState)) {
        AFortWeapon* DamageCauser = nullptr;
        if (auto Projectile = DeathReport.DamageCauser.Get() ? Cast<AFortProjectileBase>(DeathReport.DamageCauser.Get()) : nullptr)
        {
            DamageCauser = Projectile->GetOwnerWeapon();
        }
            
        else if (auto Weapon = DeathReport.DamageCauser.Get() ? Cast<AFortWeapon>(DeathReport.DamageCauser.Get()) : nullptr)
            DamageCauser = Weapon;

        return ClientOnPawnDiedOG(PlayerController, DeathReport);
    }
    if (bLateGame)
    {
        if (KillerPawn && KillerPawn->GetHealth() < 100.f)
        {
            float HealthNeeded = 100.f - KillerPawn->GetHealth();
            float HealthToAdd = FMath::Min(50.f, HealthNeeded);
            float Overflow = 50.f - HealthToAdd;

            KillerPawn->SetHealth(KillerPawn->GetHealth() + HealthToAdd);

            if (Overflow > 0.f)
            {
                KillerPawn->SetShield(FMath::Min(100.f, KillerPawn->GetShield() + Overflow));
            }
        }
        else
        {
            KillerPawn->SetShield(FMath::Min(100.f, KillerPawn->GetShield() + 50.f));
        }
    }
    FortQuestManager::GiveAccolade(KillerPlayerController, StaticFindObject<UFortAccoladeItemDefinition>("/BattlePassPermanentQuests/Items/Accolades/AccoladeId_012_Elimination.AccoladeId_012_Elimination"));

    if (!bLateGame && !GameState->IsRespawningAllowed(PlayerState) && (PlayerController->MyFortPawn ? !PlayerController->MyFortPawn->IsDBNO() : true))
    {
        PlayerState->Place = GameState->PlayersLeft;
        PlayerState->OnRep_Place();
        FAthenaMatchStats& Stats = PlayerController->MatchReport->MatchStats;
        FAthenaMatchTeamStats& TeamStats = PlayerController->MatchReport->TeamStats;

        Stats.Stats[3] = PlayerState->KillScore;
        Stats.Stats[8] = PlayerState->SquadID;
        PlayerController->ClientSendMatchStatsForPlayer(Stats);

        TeamStats.Place = PlayerState->Place;
        TeamStats.TotalPlayers = GameState->TotalPlayers;
        PlayerController->ClientSendTeamStatsForPlayer(TeamStats);

        AFortWeapon* DamageCauser = nullptr;
        if (auto Projectile = DeathReport.DamageCauser.Get() ? Cast<AFortProjectileBase>(DeathReport.DamageCauser.Get()) : nullptr)
        {
            DamageCauser = Projectile->GetOwnerWeapon();
        }
        else if (auto Weapon = DeathReport.DamageCauser.Get() ? Cast<AFortWeapon>(DeathReport.DamageCauser.Get()) : nullptr)
            DamageCauser = Weapon;

        ((void (*)(AFortGameModeAthena*, AFortPlayerController*, APlayerState*, AFortPawn*, UFortWeaponItemDefinition*, EDeathCause, char))(ImageBase + 0x8FD53FC))(GameMode, PlayerController, KillerPlayerState == PlayerState ? nullptr : KillerPlayerState, KillerPawn, DamageCauser ? DamageCauser->WeaponData : nullptr, PlayerState->DeathInfo.DeathCause, 0);
        
        PlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, PlayerController->MatchReport->EndOfMatchResults);
        if (/*PlayerController->MyFortPawn && ((KillerPlayerState && KillerPlayerState->Place == 1) || PlayerState->Place == 1)*/ false)
        {
            if (PlayerState->Place == 1)
            {
                KillerPlayerState = PlayerState;
                KillerPawn = (AFortPlayerPawnAthena*)PlayerController->MyFortPawn;
            }
            auto KillerPlayerController = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;
            auto KillerWeapon = DamageCauser ? DamageCauser->WeaponData : nullptr;

            KillerPlayerController->PlayWinEffects(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause, false, ERoundVictoryAnimation::ERoundVictoryAnimation_MAX);
            KillerPlayerController->ClientNotifyWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);
            KillerPlayerController->ClientNotifyTeamWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);

            if (KillerPlayerState != PlayerState)
            {
                KillerPlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, KillerPlayerController->MatchReport->EndOfMatchResults);

                FAthenaMatchStats& KillerStats = KillerPlayerController->MatchReport->MatchStats;
                FAthenaMatchTeamStats& KillerTeamStats = KillerPlayerController->MatchReport->TeamStats;


                KillerStats.Stats[3] = KillerPlayerState->KillScore;
                KillerStats.Stats[8] = KillerPlayerState->SquadID;
                KillerPlayerController->ClientSendMatchStatsForPlayer(KillerStats);

                KillerTeamStats.Place = KillerPlayerState->Place;
                KillerTeamStats.TotalPlayers = GameState->TotalPlayers;
                KillerPlayerController->ClientSendTeamStatsForPlayer(KillerTeamStats);
            }

            GameState->WinningTeam = KillerPlayerState->TeamIndex;
            GameState->OnRep_WinningTeam();
            GameState->WinningPlayerState = KillerPlayerState;
            GameState->OnRep_WinningPlayerState();
        }
    }
    else {
      /*  if (GameState->PlayersLeft == 1) {
            ULateGameLibrary::RestartSafeZoneIndicator();
        }*/
    }

    PlayerController->StateName = UKismetStringLibrary::Conv_StringToName(L"Spectating");

    return ClientOnPawnDiedOG(PlayerController, DeathReport);
}

void FortPlayerController::ServerCreateBuildingActor(AFortPlayerControllerAthena* PlayerController, FCreateBuildingActorData& CreateBuildingData)
{
    auto GameState = GetGameState();
    TArray<ABuildingActor*> BuildingActors;

    for (auto& BuildableClass : GameState->AllPlayerBuildableClassesIndexLookup)
    {
        if (BuildableClass.Value() == CreateBuildingData.BuildingClassHandle)
            CreateBuildingData.BuildingClassData.BuildingClass = BuildableClass.Key();
    }

    if (auto BuildingClass = CreateBuildingData.BuildingClassData.BuildingClass)
    {
        EFortBuildPreviewMarkerOptionalAdjustment Adjustment;
        EFortStructuralGridQueryResults QueryResult = GameState->StructuralSupportSystem->CanAddBuildingActorClassToGrid(UWorld::GetWorld(), BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, CreateBuildingData.bMirrored, &BuildingActors, &Adjustment, false);

        if (QueryResult == EFortStructuralGridQueryResults::CanAdd)
        {
            // what in the fuck is that 
            for (int32 i = 0; i < BuildingActors.Num(); i++)
            {
                ABuildingActor* ExistingBuilding = BuildingActors[i];
                if (!ExistingBuilding)
                    continue;

                ExistingBuilding->K2_DestroyActor();
            }
 
            if (!PlayerController->CanAffordToPlaceBuildableClass(&CreateBuildingData.BuildingClassData))
                return;
            
            PlayerController->PayBuildableClassPlacementCost(&CreateBuildingData.BuildingClassData);
            
            if (auto BuildingSMActor = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot))
            {
                BuildingSMActor->CurrentBuildingLevel = CreateBuildingData.BuildingClassData.UpgradeLevel = 0;
                BuildingSMActor->OnRep_CurrentBuildingLevel();

                BuildingSMActor->SetMirrored(CreateBuildingData.bMirrored);
                BuildingSMActor->bPlayerPlaced = true;
                
                BuildingSMActor->InitializeKismetSpawnedBuildingActor(BuildingSMActor, PlayerController, true, nullptr, false);

                
                if (auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState)
                {
                    BuildingSMActor->TeamIndex = PlayerState->TeamIndex;
                    BuildingSMActor->SetTeam(PlayerState->TeamIndex);
                    BuildingSMActor->OnRep_Team();
                }
            }
        }
    }
}

void FortPlayerController::ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
    AFortPlayerStateZone* PlayerStateZone = Cast<AFortPlayerStateZone>(PlayerController->PlayerState);
    if (!PlayerStateZone || !BuildingActorToEdit || !PlayerController->MyFortPawn)
        return;

    BuildingActorToEdit->SetNetDormancy(ENetDormancy(2 - (PlayerStateZone != nullptr)));
    BuildingActorToEdit->ForceNetUpdate();
    BuildingActorToEdit->EditingPlayer = PlayerStateZone;
    BuildingActorToEdit->OnRep_EditingPlayer();
    
    static UFortEditToolItemDefinition* EditToolItemDefinition = GetAssetManager()->GameDataCommon->EditToolItem.Get();
    if (!EditToolItemDefinition)
    {
        EditToolItemDefinition = GetAssetManager()->GameDataCommon->EditToolItem.Get();
    }

    // clean up whatever this is
    auto ItemEntry = PlayerController->GetInventory()->GetItemEntryByDef(EditToolItemDefinition);
    if (ItemEntry && EditToolItemDefinition && PlayerController->MyFortPawn->EquipWeaponDefinition(EditToolItemDefinition, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false))
    {
        AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerController->MyFortPawn->CurrentWeapon);

        if (EditingTool)
        {
            EditingTool->EditActor = BuildingActorToEdit;
            EditingTool->ForceNetUpdate();
            EditingTool->OnRep_EditActor();
        }
    }
}

void FortPlayerController::ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored)
{
    if (!PlayerController || !BuildingActorToEdit || !BuildingActorToEdit->IsA(ABuildingSMActor::StaticClass()) || !PlayerController->MyFortPawn)
        return;
    
   auto NewBuild =  BuildingActorToEdit->ReplaceBuildingActor(EBuildingReplacementType::BRT_Edited, NewBuildingClass, BuildingActorToEdit->CurrentBuildingLevel, RotationIterations, bMirrored, PlayerController);
   if (NewBuild)
    {
        NewBuild->bPlayerPlaced = true;
    }
}

void FortPlayerController::ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
    if (!BuildingActorToEdit || BuildingActorToEdit->EditingPlayer != PlayerController->PlayerState || BuildingActorToEdit->bDestroyed || !PlayerController->MyFortPawn) return;
    auto PlayerState = (AFortPlayerState*)PlayerController->PlayerState;

    
    BuildingActorToEdit->EditingPlayer = nullptr;
    BuildingActorToEdit->ForceNetUpdate();
    BuildingActorToEdit->OnRep_EditingPlayer();

    AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerController->MyFortPawn->CurrentWeapon);
    if (EditingTool)
    {
        EditingTool->EditActor = nullptr;
        EditingTool->ForceNetUpdate();
        EditingTool->OnRep_EditActor();
    }
}

void FortPlayerController::ServerRepairBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair)
{
    auto aa = BuildingActorToRepair->GetCostToRepair(PlayerController);
    PlayerController->PayBuildingRepairCost(BuildingActorToRepair);
    std::cout << "repair" << std::endl;
    BuildingActorToRepair->RepairBuilding(PlayerController, aa);
}

void FortPlayerController::ServerExecuteInventoryItem(AFortPlayerController* Controller, FGuid ItemGuid)
{
    if (!Controller || !Controller->MyFortPawn) return;
    auto Inventory = (AFortInventory*)Controller->WorldInventory.GetObjectRef();
    auto ItemEntry =  Inventory->GetItemEntryByGuid(ItemGuid);

    if (ItemEntry)
    {
      /*  if (Entry->ItemDefinition->IsA(UFortDecoItemDefinition::StaticClass()))
        {
            auto DecoItemDefinition = Cast<UFortDecoItemDefinition>(Entry->ItemDefinition);
            if (!DecoItemDefinition) return;
    
            Controller->MyFortPawn->PickUpActor(nullptr, DecoItemDefinition);
            Controller->MyFortPawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

            if (auto ContextTrap = Cast<AFortDecoTool_ContextTrap>(Controller->MyFortPawn->CurrentWeapon))
                ContextTrap->SetContextTrapItemDefinition(Cast<UFortContextTrapItemDefinition>(DecoItemDefinition));
            return;
        } */
        if (ItemEntry->ItemDefinition->IsA(UFortGadgetItemDefinition::StaticClass()))
        {
            auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(ItemEntry->ItemDefinition);
            Controller->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)GadgetItemDefinition->WeaponItemDefinition, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false); 
        }
        else
        {
            Controller->MyFortPawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemEntry->ItemDefinition, ItemEntry->ItemGuid, ItemEntry->TrackerGuid, false); 
        }
    }
}

void FortPlayerController::ServerCheat(AFortPlayerControllerAthena* Controller, FString Msg) {
    auto PlayerState = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
    auto PlayerName = PlayerState->GetPlayerName().ToString();
    auto Pawn = (AFortPlayerPawnAthena*)Controller->Pawn;
    bool isMsgEmpty = Msg.Num() <= 0;

    std::vector<UEAllocatedString> Arguments;
    std::string OldMsg = "";
    if (!isMsgEmpty)
        OldMsg = Msg.ToString();

    if (!isMsgEmpty)
    {
        auto Message = Msg.ToString();
        while (Message.find(" ") != std::string::npos)
        {
            auto arg = Message.substr(0, Message.find(' '));
            Arguments.push_back(arg);
            Message.erase(0, Message.find(' ') + 1);
        }
        Arguments.push_back(Message);
    }
    
    auto NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;
    if (Arguments.size() >= 1)
    {
        auto& Command = Arguments[0];
        std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

        if (Command == "getsafezonecnt") {
            FFortSafeZoneDefinition Definition = GetGameState()->MapInfo->SafeZoneDefinition;
            float ZoneCount = Definition.Count.Evaluate();
            std::cout << ZoneCount << "\n";
        }
        if (Command == "startaircraft")
        {
            GetLogic()->StartAircraftPhase();
        }
        if (Command == "testvolumes") {
            TArray<AActor*> Actors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPoiVolume::StaticClass(), &Actors);
            for (auto Actor : Actors) {
                std::wstring LocationTagsStr = L"";
                for (auto Tag : ((AFortPoiVolume*)Actor)->LocationTags.GameplayTags) {
                    LocationTagsStr += std::wstring(Tag.TagName.GetRawWString().c_str()) + L" ";
                }
                auto Location = Actor->K2_GetActorLocation();
                std::wstring Out = std::wstring(Actor->Name.GetRawWString().c_str()) + std::wstring(LocationTagsStr) + LocationTagsStr;

                Controller->ClientMessage(Out.c_str(), FName(), 1);
            }
        }
        if (Command == "getpos") {
            auto Location = Pawn->K2_GetActorLocation();
            std::cout << Location.X << " " << Location.Y << " " << Location.Z << "\n";
        }
        if (Command == "god") {
            Controller->bCanBeDamaged = !(bool)Controller->bCanBeDamaged;
        }
        if (Command == "testdiscover") {
            auto Comp = (UFortControllerComponent_MapDiscoverability*)Controller->GetComponentByClass(UFortControllerComponent_MapDiscoverability::StaticClass());
            Comp->DiscoverAllAreas();
            Comp->DiscoverAllPois();
            Comp->DiscoveredData.MarkArrayDirty();
        }
        if (Command == "mmetest")
        {
            auto Comp = (UFortMovementComp_Character*)Pawn->CharacterMovement;
           auto array =  Comp->ActiveLayeredMovementModeRuntimeData;
            std::cout << "num of these: " << array.Num() << std::endl;
            for (auto idk : array)
            {
                std::cout << idk->Class->Name.ToString().c_str() << std::endl;
            }
        }
        if (Command == "forcemmeoff")
        {
            for (auto what: Pawn->RegisteredMovementModeExtentionLogic)
            {
                //what.Second->BP_OnExtensionDeactivate();
                ///what.Second->BP_OnExtensionEnd();
            }
        }
        if (Command == "testdiscover2") {
            auto Comp = (UFortControllerComponent_MapDiscoverability*)Controller->GetComponentByClass(UFortControllerComponent_MapDiscoverability::StaticClass());
            TArray<AActor*> Actors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPoiVolume::StaticClass(), &Actors);
            for (auto Actor : Actors) {
                FPoiDiscoverabilityDataEntry Entry = {};
                Entry.bWasInitiallyDiscovered = false;
                Entry.DiscoveredState = EPoiDiscoveredState::Undiscovered;
                Entry.VisitCounter++;
                FGameplayTag Tag;
                Tag.TagName = ((AFortPoiVolume*)Actor)->LocationTags.GameplayTags[0].TagName;
                Entry.POITag = Tag;
                Comp->DiscoveredData.Entries.Add(Entry);
                
            }
            Comp->DiscoveredData.MarkArrayDirty();
        }

        if (Command == "modbench")
        {
            auto loc = Controller->MyFortPawn->K2_GetActorLocation();
            auto newloc = FVector(loc.X + 250.0f, loc.Y, loc.Z);


            auto idkclass = StaticLoadObject<UClass>("/WeaponModStation/Gameplay/Actors/BP_WeaponModStation_v2.BP_WeaponModStation_v2_C");
            GetWorld()->SpawnActor(idkclass, newloc, Pawn->K2_GetActorRotation());
        }
        if (Command == "testmme") {
            auto Comp = (UFortMovementComp_Character*)Pawn->CharacterMovement;
            for (auto RuntimeData : Comp->GetReplicatedRuntimeDatas()) {
                if (RuntimeData->IsA(UFortMovementMode_TacSprintRuntimeData::StaticClass())) {
                    //Comp->NewlyAddedMovementModeExtensionRuntimeData.Add(RuntimeData);
                    Comp->PushedMovementModeExtensionRuntimeData.Add(RuntimeData);
                    __int64(__fastcall * IdkWhatIsThatFunction)(UFortMovementComp_Character * Comp) = decltype(IdkWhatIsThatFunction)(ImageBase+0x139B788);
                    IdkWhatIsThatFunction(Comp);
                    auto Num = Comp->ActiveLayeredMovementModeRuntimeData.Num();
                    Controller->ClientMessage(std::to_wstring(Num).c_str(), FName(), 1);
                }
            }
        }
        if (Command == "kill")
        {
            auto Accolade = StaticFindObject<UFortAccoladeItemDefinition>("/BattlePassPermanentQuests/Items/Accolades/AccoladeId_012_Elimination.AccoladeId_012_Elimination");
            FortQuestManager::GiveAccolade(Controller, Accolade);
        }
        if (Command == "teleport")
        {
            if (!Pawn) return;
            float X = 0, Y = 0, Z = 0;
    
            for (size_t i = 1; i <= 3; i++)
            {
                std::string arg = Arguments[i].c_str();
        
                if (arg.find("X=") != std::string::npos)
                    X = std::stof(arg.substr(2));
                else if (arg.find("Y=") != std::string::npos)
                    Y = std::stof(arg.substr(2));
                else if (arg.find("Z=") != std::string::npos)
                    Z = std::stof(arg.substr(2));
            }

            Pawn->K2_SetActorLocation(FVector(X, Y, Z), false, nullptr, true);
        }
        if (Command == "spawnbot")
        {
            static auto SpawnerDataClass = StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");
            auto DefaultObject = (UFortAthenaAIBotSpawnerData*)SpawnerDataClass->DefaultObject;
            auto ComponentList = DefaultObject->CreateComponentList(DefaultObject);
            auto Transform = FTransform();
            Transform.Translation = Controller->Pawn->K2_GetActorLocation();
            Transform.Rotation = FQuat();
            auto One = FVector();
            One.X = 1;
            One.Y = 1;
            One.Z = 1;
            Transform.Scale3D = One;

            ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, Transform, false);
        }
        // "/MoonFlaxBoss_TheOverseer/SpawnerData/BP_AIBSD_MoonFlaxBosses_TheOverseer.1"
        if (Command == "spawnboss")
        {
            static auto SpawnerDataClass = StaticLoadObject<UClass>("/MoonFlaxBoss_TheOverseer/SpawnerData/BP_AIBSD_MoonFlaxBosses_TheOverseer.BP_AIBSD_MoonFlaxBosses_TheOverseer_C");
            auto DefaultObject = (UFortAthenaAIBotSpawnerData*)SpawnerDataClass->DefaultObject;
            auto ComponentList = DefaultObject->CreateComponentList(DefaultObject);
            auto Transform = FTransform();
            Transform.Translation = Controller->Pawn->K2_GetActorLocation();
            Transform.Rotation = FQuat();
            auto One = FVector();
            One.X = 1;
            One.Y = 1;
            One.Z = 1;
            Transform.Scale3D = One;

            ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, Transform, false);
        }
        if (Command == "clearmme") {
            auto Comp = (UFortMovementComp_Character*)Pawn->CharacterMovement;
            Comp->PushedMovementModeExtensionRuntimeData.Clear();
            Comp->NewlyAddedMovementModeExtensionRuntimeData.Clear();
            Comp->ActiveLayeredMovementModeRuntimeData.Clear();
        }
        if (Command == "spawnnpc")
        {
            static auto SpawnerClass = StaticLoadObject<UClass>("/NPCLibrary/NPCs/BaoBros/BaoBros/BP_AIBotSpawnerData_NPC_BaoBros.BP_AIBotSpawnerData_NPC_BaoBros_C");
            auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerClass, UWorld::GetWorld());
            // that loc is aids
            auto Transform = FTransform();
            Transform.Translation = Controller->Pawn->K2_GetActorLocation();
            Transform.Rotation = FQuat();
            auto One = FVector();
            One.X = 1;
            One.Y = 1;
            One.Z = 1;
            Transform.Scale3D = One;
            
            ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, Transform, false);
        }
        if (Command == "giveitem")
        {
            if (NumArgs < 1)
            {
                return;
            }

            auto& weaponName = Arguments[1];
            int count = 1;

            try
            {
                if (NumArgs >= 2)
                    count = std::stoi(Arguments[2].c_str());
            }
            catch (...)
            {
            }

            auto WID = StaticFindObject<UFortWorldItemDefinition>(weaponName, nullptr, (UObject*)-1);

            if (!WID)
            {
                Controller->ClientMessage(L"Invalid Id", FName(0), 1);
            }


            Controller->GetInventory()->AddItem(WID, count, 1);

        }
        if (Command == "dev")
        {
            Controller->bBuildFree = !Controller->bBuildFree;
            Controller->bInfiniteAmmo = !Controller->bInfiniteAmmo;
        }

        if (Command == "idkbro")
        {
            auto objs = GetObjectsOfClass<AFortAthenaPatrolPath>();
            for (auto obj : objs)
            {
                for (auto tag : obj->GameplayTags.GameplayTags)
                {
                   std::cout <<  tag.TagName.ToString().c_str() << std::endl;
                }
            }
        }
        if (Command == "vend")
        {
            auto loc = Controller->MyFortPawn->K2_GetActorLocation();
            auto newloc = FVector(loc.X + 250.0f, loc.Y, loc.Z);
            auto vending = StaticLoadObject<UClass>("/Game/Building/ActorBlueprints/Stations/BP_StationProp_VendingMachine_E_Assault.BP_StationProp_VendingMachine_E_Assault_C");
            GetWorld()->SpawnActor(vending, newloc, Pawn->K2_GetActorRotation(), nullptr);
        }
        if (Command == "points")
        {
            auto objs = GetObjectsOfClass<AFortAthenaLivingWorldStaticPointProvider>();
            for (auto obj : objs)
            {
                std::cout << obj->Name.ToString().c_str() << std::endl;
                for (auto tag : obj->FiltersTags.GameplayTags)
                {
                    std::cout << tag.TagName.ToString().c_str() << std::endl;
                }
            }
        }
        if (Command == "getmmeinfo") {
            auto Comp = (UFortMovementComp_Character*)Pawn->CharacterMovement;
            Controller->ClientMessage(L"ActiveLayeredMovementModeRuntimeData", FName(), 1);
            for (auto RuntimeData : Comp->ActiveLayeredMovementModeRuntimeData) {
                auto Name = RuntimeData->GetName();
                Controller->ClientMessage(std::wstring(Name.begin(), Name.end()).c_str(), FName(), 1);
            }

            Controller->ClientMessage(L"PushedMovementModeExtensionRuntimeData", FName(), 1);
            for (auto RuntimeData : Comp->PushedMovementModeExtensionRuntimeData) {
                auto Name = RuntimeData->GetName();
                Controller->ClientMessage(std::wstring(Name.begin(), Name.end()).c_str(), FName(), 1);
            }

            Controller->ClientMessage(L"NewlyAddedMovementModeExtensionRuntimeData", FName(), 1);
            for (auto RuntimeData : Comp->NewlyAddedMovementModeExtensionRuntimeData) {
                auto Name = RuntimeData->GetName();
                Controller->ClientMessage(std::wstring(Name.begin(), Name.end()).c_str(), FName(), 1);
            }
            Controller->ClientMessage(L"ReceivedPackMovementModeExtensionRuntimeData", FName(), 1);
            for (auto RuntimeData : Comp->ReceivedPackMovementModeExtensionRuntimeData) {
                auto Name = RuntimeData->GetName();
                Controller->ClientMessage(std::wstring(Name.begin(), Name.end()).c_str(), FName(), 1);
            }
            Controller->ClientMessage(L"RuntimeData", FName(), 1);
            for (auto RuntimeData : Comp->GetReplicatedRuntimeDatas()) {
                auto Name = RuntimeData->GetName();
                Controller->ClientMessage(std::wstring(Name.begin(), Name.end()).c_str(), FName(), 1);
            }
            
        }
    }
}


void FortPlayerController::Init()
{
    UHook* Hook = new UHook();

    Hook->Address = 0x133;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerAcknowledgePossession;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = 0x22F;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerExecuteInventoryItem;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = 0x23A;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerAttemptInventoryDrop;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = 0x250;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerCreateBuildingActor;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);

    Hook->Address = 0x257;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerBeginEditingBuildingActor;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);

    Hook->Address = 0x252;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerEditBuildingActor;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);

    Hook->Address = 0x255;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerEndEditingBuildingActor;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);
    
    Hook->Address = 0x1260 / 8;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerRepairBuildingActor;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    Hook->Address = 0x1FB;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerPlayEmoteItem;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);

    Hook->Address = 0xA2541D0 + ImageBase;
    Hook->Detour = ClientOnPawnDied;
    Hook->Original = (void**)&ClientOnPawnDiedOG;
    UKismetHookingLibrary::Hook(Hook, EHook::Address);

    Hook->Address = 0x1F9;
    Hook->Class = AFortPlayerController::StaticClass();
    Hook->Detour = ServerCheat;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);

   /* Hook->Address = 0x2440 / 8;
    Hook->Class = AFortLiveSpectatorController::StaticClass();
    Hook->Detour =ServerSetFollowedPlayer_Implementation;
    UKismetHookingLibrary::Hook(Hook, EHook::EveryVFT);*/

    Hook->Path = "/Script/FortniteGame.FortPawn.MovingEmoteStopped";
    Hook->Detour = MovingEmoteStopped;
    UKismetHookingLibrary::Hook(Hook, EHook::Exec);

    delete Hook;
}
