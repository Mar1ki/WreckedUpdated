#include "pch.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Building/BuildingProp_ConversationCompatible.h"



// it might be ahh but whatever
char BuildingProp_ConversationCompatible::ServerOnAttemptInteract(ABuildingGameplayActorConversationCompatible* BuildingActor, const FInteractionType *InteractType)
{
	auto result = ServerOnAttemptInteractOG(BuildingActor, InteractType);


	return result;
}
void BuildingProp_ConversationCompatible::Init() {
    UHook* Hook = new UHook();

	Hook->Address = ImageBase + 0x93779B4;
	Hook->Detour = ServerOnAttemptInteract;
	Hook->Original = (void**)&ServerOnAttemptInteractOG;
	UKismetHookingLibrary::Hook(Hook, Address);

	delete Hook;
}