#include "pch.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetDriver.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/FortControllerComponent_RechargeWeapons.h"
/*
 * your old code btw
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
}*/

void NetDriver::TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
{
    auto ReplicationSystem = *(UReplicationSystem**)(__int64(NetDriver) + 0x7C0);
	if (NetDriver->ClientConnections.Num() > 0 && ReplicationSystem)
	{
		NetDriver->UpdateIrisReplicationViews();
		NetDriver->SendClientMoveAdjustments();
		
		FSendUpdateParams Params{};
		Params.DeltaSeconds = DeltaSeconds;
		Params.SendPass = EReplicationSystemSendPass::TickFlush;
		
		PreSendUpdate(ReplicationSystem, Params);
		FortControllerComponent_RechargeWeapons::TickComponent();
	}
    return TickFlushOG(NetDriver, DeltaSeconds);
}
// unproper
float GetMaxTickRate(UEngine* Engine, float DeltaTime, bool bAllowFrameRateSmoothing)
{
	return 30.f;
}
void NetDriver::Init()
{
	UHook* Hook = new UHook();
    
	Hook->Address = ImageBase + 0x1E66424;
	Hook->Original = (void**)&TickFlushOG;
	Hook->Detour = TickFlush;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	Hook->Address = ImageBase + 0x1E65D44;
	Hook->Detour = GetMaxTickRate;
	UKismetHookingLibrary::Hook(Hook, Address);

	delete Hook;
}