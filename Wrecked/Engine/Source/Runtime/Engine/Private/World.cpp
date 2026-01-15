#include "pch.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

bool UWorld::Listen(FURL URL)
{
    UE_LOG(LogTemp, Log, "World::Listen called!");
    
    static FWorldContext* (*GetWorldContextFromObject)(UEngine*, UWorld*) = decltype(GetWorldContextFromObject)(ImageBase + 0x1ABA190);
    FWorldContext* WorldContext = GetWorldContextFromObject(UFortEngine::GetEngine(), this);
    static UNetDriver* (*CreateNetDriver_Local)(UEngine*, FWorldContext&, FName, FName) = decltype(CreateNetDriver_Local)(ImageBase + 0x24BC7B0);
    FName GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
    UNetDriver* NetDriver = CreateNetDriver_Local(UEngine::GetEngine(), *WorldContext, GameNetDriver, GameNetDriver);
    if (!NetDriver) return false;
    *(bool*)(__int64(NetDriver) + 0x7C9) = true;

    this->NetDriver = NetDriver;
    this->NetDriver->World = this;
    this->NetDriver->NetDriverName = GameNetDriver;
    for (auto& Level : this->LevelCollections) Level.NetDriver = NetDriver;

    FString Err;    

    bool (*InitListen)(void*, void*, FURL&, bool, FString&) = decltype(InitListen)(ImageBase + 0x70453F0);
    void (*SetWorld)(UNetDriver*, UWorld*) = decltype(SetWorld)(ImageBase + 0x24BBD30);

    SetWorld(NetDriver, this);
    if (InitListen(NetDriver, this, URL, false, Err));
    {
        SetWorld(NetDriver, this);
        char buffer[100];
        time_t now = time(nullptr);
        tm localTime;
        localtime_s(&localTime, &now);
        strftime(buffer, sizeof(buffer), "Wrecked 30.00 - Listening on port 7777 - Joinable = true | Compiled at: %I:%M:%S %p", &localTime);
        SetConsoleTitleA(buffer);
    }
    
    return true;
}

AActor* UWorld::SpawnActor(UClass* Class, FTransform& Transform, AActor* Owner)
{
    FActorSpawnParameters addr{};
    
    addr.Owner = Owner;
    addr.bDeferConstruction = false;
    addr.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    addr.NameMode = Requested;
    addr.ObjectFlags = EObjectFlags::Transactional;
    addr.bNoFail = true;


    return ((AActor * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(ImageBase + 0x1A8087C))(GetWorld(), Class, &Transform, &addr);
}

AActor* UWorld::SpawnActor(UClass* Class, FVector Loc, FRotator Rot, AActor* Owner)
{
    FTransform Transform = UKismetMathLibrary::MakeTransform(Loc, Rot, FVector(1.0f, 1.0f, 1.0f));
    return SpawnActor(Class, Transform, Owner);
}