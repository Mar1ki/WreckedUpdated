#include "pch.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetDriver.h"

void NetDriver::SendClientMoveAdjustments(UNetDriver* Driver)
{
	static void (*SendClientAdjustment)(INetworkPredictionInterface*) = decltype(SendClientAdjustment)(ImageBase + 0x6A3F8A8);

    for (UNetConnection* Connection : Driver->ClientConnections)
    {
        if (Connection == nullptr || Connection->ViewTarget == nullptr) continue;

        if (APlayerController* PC = Connection->PlayerController)
        {
            auto Pawn = (ACharacter *) PC->Pawn;
            if (Pawn == nullptr) continue;
            auto Interface = Pawn->CharacterMovement->GetInterfaceAddress<INetworkPredictionInterface>();
            SendClientAdjustment(Interface);
        }

        for (UNetConnection* ChildConnection : Connection->Children)
        {
            if (ChildConnection == nullptr) continue;

            if (APlayerController* PC = ChildConnection->PlayerController)
            {
                auto Pawn = (ACharacter *) PC->Pawn;
                if (Pawn == nullptr) continue;
                auto Interface = Pawn->CharacterMovement->GetInterfaceAddress<INetworkPredictionInterface>();
                SendClientAdjustment(Interface);
            }
        }
    }
}

void NetDriver::TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
{
    auto ReplicationSystem = *(UReplicationSystem**)(__int64(NetDriver) + 0x7c0);
	if (NetDriver->ClientConnections.Num() > 0 && ReplicationSystem)
	{
		static void(*UpdateReplicationViews)(UNetDriver*) = decltype(UpdateReplicationViews)(ImageBase + 0x6CE5844);
		static void(*PreSendUpdate)(UReplicationSystem*, FSendUpdateParams&) = decltype(PreSendUpdate)(ImageBase + 0x5F8FD5C);

		UpdateReplicationViews(NetDriver);
		SendClientMoveAdjustments(NetDriver);
		FSendUpdateParams Params;
		Params.DeltaSeconds = DeltaSeconds;
		PreSendUpdate(ReplicationSystem, Params);
	}
	
    return TickFlushOriginal(NetDriver, DeltaSeconds);
}

void NetDriver::Init()
{
	UHook* Hook = new UHook();
    
	Hook->Address = ImageBase + 0x1E66424;
	Hook->Original = (void**)&TickFlushOriginal;
	Hook->Detour = TickFlush;
	UKismetHookingLibrary::Hook(Hook, Address);

	delete Hook;
}