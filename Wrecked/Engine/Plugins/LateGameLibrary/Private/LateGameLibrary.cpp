#include "pch.h"
#include "../Public/LateGameLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponModFunctionLibrary.h"

TArray<FItemAndCount> Rifles;
TArray<FItemAndCount> Shotguns;

TArray<FItemAndCount> WhiteHeals;
TArray<FItemAndCount> BlueHeals;

TArray<FItemAndCount> RotationItems;


void Init()
{
    FItemAndCount CombatRifle;
    CombatRifle.Item = StaticLoadObject<UFortWeaponRangedItemDefinition>("/WeaponsUpdated/Gameplay/TacticalAR/WID_Assault_SunRose_Tactical_Athena_SR.WID_Assault_SunRose_Tactical_Athena_SR");
    CombatRifle.Count = 1;

    FItemAndCount EnforcerRifle;
    EnforcerRifle.Item = StaticLoadObject<UFortWeaponRangedItemDefinition>("/PaprikaCoreWeapons/Items/Weapons/PaprikaAR_Infantry/WID_Assault_Paprika_Infantry_Athena_SR.WID_Assault_Paprika_Infantry_Athena_SR");
    EnforcerRifle.Count = 1;

    FItemAndCount WarForgedRifle;
    WarForgedRifle.Item = StaticLoadObject<UFortWeaponRangedItemDefinition>("/SunRoseWeaponsGameplay/Items/Weapons/WarforgedAR/WID_Assault_SunRose_Athena_SR.WID_Assault_SunRose_Athena_SR");
    WarForgedRifle.Count = 1;

    Rifles.Add(CombatRifle);
    Rifles.Add(EnforcerRifle);
    Rifles.Add(WarForgedRifle);

    FItemAndCount CombatShotgun;
    CombatShotgun.Item = StaticFindObject<UFortWeaponRangedItemDefinition>("/WeaponsUpdated/Gameplay/CombatShotgun/WID_Shotgun_Moonflax_Combat_Athena_SR.WID_Shotgun_Moonflax_Combat_Athena_SR");
    CombatShotgun.Count = 1;

    FItemAndCount GateKeeperShotgun;
    GateKeeperShotgun.Item = StaticFindObject<UFortWeaponRangedItemDefinition>("/SunRoseWeaponsGameplay/Items/Weapons/CerberusSG/WID_Shotgun_Break_Cerberus_Athena_SR.WID_Shotgun_Break_Cerberus_Athena_SR");
    GateKeeperShotgun.Count = 1;

    FItemAndCount HammerShotgun;
    HammerShotgun.Item = StaticFindObject<UFortWeaponRangedItemDefinition>("/PaprikaCoreWeapons/Items/Weapons/PaprikaShotgun_Pump/WID_Shotgun_Pump_Paprika_Athena_SR.WID_Shotgun_Pump_Paprika_Athena_SR");
    HammerShotgun.Count = 1;

    Shotguns.Add(CombatShotgun);
    Shotguns.Add(GateKeeperShotgun);
    Shotguns.Add(HammerShotgun);

    FItemAndCount Medkit;
    Medkit.Item = StaticLoadObject<UItemDefinitionBase>("/PaprikaConsumables/Gameplay/MedRefresh/WID_Paprika_MedBox.WID_Paprika_MedBox");
    Medkit.Count = 3;

    WhiteHeals.Add(Medkit);

    FItemAndCount Fizz;
    Fizz.Item = StaticLoadObject<UItemDefinitionBase>("/PaprikaConsumables/Gameplay/TeamSpray/WID_Paprika_TeamSpray_LowGrav.WID_Paprika_TeamSpray_LowGrav");
    Fizz.Count = 1;

    FItemAndCount SmallShield;
    SmallShield.Item = StaticLoadObject<UItemDefinitionBase>("/PaprikaConsumables/Gameplay/ShieldSmallRefresh/WID_Paprika_ShieldSmall.WID_Paprika_ShieldSmall");
    SmallShield.Count = 6;

    FItemAndCount Shield;
    Shield.Item = StaticLoadObject<UItemDefinitionBase>("/PaprikaConsumables/Gameplay/ShieldRefresh/WID_Paprika_ShieldPot.WID_Paprika_ShieldPot");
    Shield.Count = 3;


    FItemAndCount NitroGloves;
    Shield.Item = StaticLoadObject<UItemDefinitionBase>("/NitroGauntletsGameplay/Gameplay/WID_Moonflax_NitroGauntlet_Mythic.WID_Moonflax_NitroGauntlet_Mythic");
    Shield.Count = 1;

    FItemAndCount NitroSplash;
    Shield.Item = StaticLoadObject<UItemDefinitionBase>("/MoonflaxNitroSplash/Gameplay/WID_Moonflax_NitroSplash.WID_Moonflax_NitroSplash");
    Shield.Count = 4;

    FItemAndCount Shockwave;
    Shield.Item = StaticLoadObject<UItemDefinitionBase>("/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade");
    Shield.Count = 6;

    RotationItems.Add(Shockwave);
    RotationItems.Add(NitroSplash);
    RotationItems.Add(NitroGloves);

    BlueHeals.Add(SmallShield);
    BlueHeals.Add(Fizz);
    BlueHeals.Add(Shield);
}

void ULateGameLibrary::GiveRandomLoadoutToPlayer(AFortPlayerControllerAthena* PlayerCotntroller)
{
    static bool bInited = false;
    if (!bInited)
    {
        bInited = true;
        Init();
        GetGameState()->DefaultParachuteDeployTraceForGroundDistance = 2500.f;
        GetGameState()->bCanSpawnLoot = false;
    }

    auto Inventory = PlayerCotntroller->GetInventory();
    if (!Inventory) return; // um i got a random crash so a null check

    auto RandomRifle = Rifles[rand() % Rifles.Num()];
    if (RandomRifle.Item)
    {
        auto CastedRifle = (UFortWeaponItemDefinition*)RandomRifle.Item;
        if (CastedRifle && CastedRifle->GetStats())
        {
            auto Item = Inventory->AddItem((UFortItemDefinition*)RandomRifle.Item, RandomRifle.Count, CastedRifle->GetStats()->ClipSize, 1, FortWeaponModFunctionLibrary::GetRandomModsetForWeapon(CastedRifle));
            UpdateBaseWeaponAmmo(Item);
            Inventory->AddItem(CastedRifle->GetAmmoWorldItemDefinition_BP(), 500, 0);
        }
    }

    auto RandomShotgun = Shotguns[rand() % Shotguns.Num()];
    if (RandomShotgun.Item)
    {
        auto CastedShotgun = (UFortWeaponItemDefinition*)RandomShotgun.Item;
        if (CastedShotgun && CastedShotgun->GetStats())
        {
            auto Item = Inventory->AddItem((UFortItemDefinition*)RandomShotgun.Item, RandomShotgun.Count, CastedShotgun->GetStats()->ClipSize, 1, FortWeaponModFunctionLibrary::GetRandomModsetForWeapon(CastedShotgun));
            UpdateBaseWeaponAmmo(Item);
            Inventory->AddItem(CastedShotgun->GetAmmoWorldItemDefinition_BP(), 500, 0);
        }
    }

    auto RandomWhiteHeal = WhiteHeals[rand() % WhiteHeals.Num()];
    if (RandomWhiteHeal.Item)
    {
        auto idk = (UFortWeaponItemDefinition*)RandomWhiteHeal.Item;
        Inventory->AddItem((UFortItemDefinition*)RandomWhiteHeal.Item, RandomWhiteHeal.Count, idk->GetStats()->ClipSize);
    }

    auto RandomBlueHeal = BlueHeals[rand() % BlueHeals.Num()];
    if (RandomBlueHeal.Item)
    {
        auto idk = (UFortWeaponItemDefinition*)RandomBlueHeal.Item;
        Inventory->AddItem((UFortItemDefinition*)RandomBlueHeal.Item, RandomBlueHeal.Count, idk->GetStats()->ClipSize);
    }

  /*  auto RandomRotation = RotationItems[rand() % RotationItems.Num()];
    if (RandomRotation.Item)
    {
        auto idk = (UFortWeaponItemDefinition*)RandomRotation.Item;
        Inventory->AddItem((UFortItemDefinition*)RandomRotation.Item, RandomRotation.Count, idk->GetStats()->ClipSize);
    }*/

    auto GameDataCommon = GetAssetManager()->GameDataCommon;
    Inventory->AddItem(GameDataCommon->WoodItemDefinition.Get(), 500, 0);
    Inventory->AddItem(GameDataCommon->StoneItemDefinition.Get(), 500, 0);
    Inventory->AddItem(GameDataCommon->MetalItemDefinition.Get(), 500, 0);
}

void ULateGameLibrary::SpawnInitialLateGameSafeZone()
{
    AFortGameStateAthena* AthenaGameState = GetGameState();
    AFortGameModeAthena* GameMode = GetGameMode();
    UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Logic = GetLogic();

    if (!AthenaGameState || !AthenaGameState->MapInfo || !GameMode)
    {
        return;
    }

    bool ShouldSpawnSafeZoneIndicator = true;
    for (auto Mutator : GameMode->MutatorListComponent->Mutators) {
        EFortMutatorReturnValue(*ShouldSpawnSafeZoneIndicatorOG)(AFortGameplayMutator*, bool*) = decltype(ShouldSpawnSafeZoneIndicatorOG)(Mutator->VTable[0xA60 / 8]);
        if (ShouldSpawnSafeZoneIndicatorOG(Mutator, &ShouldSpawnSafeZoneIndicator) == EFortMutatorReturnValue::Ignore)
            break;
    }
    if (!ShouldSpawnSafeZoneIndicator) return;

    *(TArray<FVector>*)(__int64(Logic) + 0x468) = TArray<FVector>();
    auto& SafeZoneLocations = *(TArray<FVector>*)(__int64(Logic) + 0x468);
    TArray<FVector4> Blacklist = {};
    auto MapCenter = AthenaGameState->MapInfo->GetMapCenter();
    AthenaGameState->MapInfo->GenerateSafeZoneLocations(SafeZoneLocations, Blacklist, &MapCenter, 0, false);//RegenerateSafeZoneLocations

    if (!Logic->SafeZoneIndicator)
    {
        if (Logic->SafeZoneIndicatorClass)
        {
            Logic->SafeZoneIndicator = (AFortSafeZoneIndicator*)GetWorld()->SpawnActor(Logic->SafeZoneIndicatorClass, {});
        }
        if (Logic->SafeZoneIndicator)
        {
            FScriptDelegate Delegate;
            Delegate.Object = Logic;
            Delegate.FunctionName = UKismetStringLibrary::Conv_StringToName(L"OnSafeZonePhaseChanged");
            Logic->SafeZoneIndicator->OnSafeZonePhaseChanged.InvocationList.Add(Delegate);
            Logic->SafeZoneIndicator->InitializeWithMapDefaultPhases();
            Logic->SafeZoneIndicator->NextCenter = (FVector_NetQuantize100)Logic->SafeZoneIndicator->SafeZonePhases[7].Center;
            Logic->SafeZoneIndicator->NextRadius = Logic->SafeZoneIndicator->SafeZonePhases[7].Radius;
            Logic->SafeZoneIndicator->StartPhase(7);
            Logic->OnRep_SafeZoneIndicator();
        }
    }
}

void ULateGameLibrary::LateGameStartAircraft() {
    auto GamePhaseLogic = GetLogic();
    if (!GamePhaseLogic->SafeZoneIndicator)
        SpawnInitialLateGameSafeZone();
    GetGameState()->MapInfo->FlightInfos.Free();
    FAircraftFlightInfo AircraftInfo{};
    auto Center = GamePhaseLogic->SafeZoneIndicator->SafeZonePhases[7].Center;
    auto Radius = GamePhaseLogic->SafeZoneIndicator->SafeZonePhases[7].Radius;
    auto DistanceToBeInZone = Radius;
    FVector FlightDirection = GetSafeNormal(FVector(1, 1, 0));
    AircraftInfo.FlightStartRotation = VectorToRotatorManual(FlightDirection);
    AircraftInfo.FlightSpeed = 7500 / 3;
    AircraftInfo.FlightStartLocation = (FVector_NetQuantize100)(Center - (FlightDirection * (Radius+DistanceToBeInZone)));
    AircraftInfo.FlightStartLocation.Z = 30000;
    AircraftInfo.TimeTillDropStart = DistanceToBeInZone / AircraftInfo.FlightSpeed;
    AircraftInfo.TimeTillFlightEnd = AircraftInfo.TimeTillDropEnd + 10;
    AircraftInfo.TimeTillDropEnd = (2 * Radius) / AircraftInfo.FlightSpeed;
    GetGameState()->MapInfo->FlightInfos.Add(AircraftInfo);
    GetGameState()->FlightPathMidLine = AircraftInfo;
    //well ngl i would never think this would work first try
}

void ULateGameLibrary::RestartSafeZoneIndicator() {
    auto GamePhaseLogic = GetLogic();

    if (!GamePhaseLogic) {
        return;
    }

    if (GamePhaseLogic->SafeZoneIndicator) {
        GamePhaseLogic->SafeZoneIndicator->K2_DestroyActor();
    }
    SpawnInitialLateGameSafeZone();//Spawn Zone
    GamePhaseLogic->StartAircraftPhase();
}

void ULateGameLibrary::CustomSafeZoneHandler(float Time) {
    AFortGameStateAthena* GameState = GetGameState();
    AFortGameModeAthena* GameMode = GetGameMode();
    if (!GameState || !GameState->MapInfo)
    {
        return;
    }

    auto GamePhaseLogic = GetLogic();

    if (!GamePhaseLogic) {
        return;
    }

    if (!GamePhaseLogic->SafeZoneIndicator) {
        SpawnInitialLateGameSafeZone();
    }
    if (GamePhaseLogic->SafeZoneIndicator) {
        bool bFinishedHolding = GamePhaseLogic->SafeZoneIndicator->SafeZoneStartShrinkTime < Time;
        bool bFinishedShrinking = GamePhaseLogic->SafeZoneIndicator->SafeZoneFinishShrinkTime < Time;

        if (bFinishedHolding && bFinishedShrinking) {
            if (GamePhaseLogic->SafeZoneIndicator->CurrentPhase + 1 == 13) {
                RestartSafeZoneIndicator();
            }
            GamePhaseLogic->SafeZoneIndicator->StartPhase(GamePhaseLogic->SafeZoneIndicator->CurrentPhase + 1);
        }
        else if (!bFinishedHolding && !bFinishedShrinking && GamePhaseLogic->GamePhaseStep == EAthenaGamePhaseStep::StormHolding) {
            GamePhaseLogic->GamePhaseStep = EAthenaGamePhaseStep::StormShrinking;
            GamePhaseLogic->HandleGamePhaseStepChanged(GamePhaseLogic->GamePhaseStep);

            *(EFortSafeZoneState*)(__int64(GamePhaseLogic->SafeZoneIndicator) + 0x60C) = EFortSafeZoneState::Shrinking;
            GamePhaseLogic->SafeZoneIndicator->OnSafeZoneStateChange(EFortSafeZoneState::Holding, GamePhaseLogic->SafeZoneIndicator->CurrentPhase == 0);
        }
    }
}