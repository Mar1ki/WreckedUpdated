#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortQuestManager.h"


void FortQuestManager::GiveAccolade(AFortPlayerControllerAthena* PlayerController, UFortAccoladeItemDefinition* AccoladeDef, UFortQuestItemDefinition* QuestDef)
{
    if (!PlayerController || !PlayerController->XPComponent || !AccoladeDef)
        return;

    FXPEventInfo Info{};
    Info.Accolade = UKismetSystemLibrary::GetPrimaryAssetIdFromObject(AccoladeDef);
    Info.EventXpValue = AccoladeDef->GetAccoladeXpValue();
    Info.Priority = AccoladeDef->GetPriority();
    Info.QuestDef = QuestDef;
    Info.RestedXPRemaining = PlayerController->XPComponent->RestXp - Info.EventXpValue;
    Info.RestedValuePortion = 100;
    Info.SeasonBoostValuePortion = PlayerController->XPComponent->CachedSeasonMatchXpBoost;
    Info.TotalXpEarnedInMatch = PlayerController->XPComponent->TotalXpEarned += Info.EventXpValue;
    Info.SimulatedText = AccoladeDef->ItemShortDescription;
    Info.SimulatedName = AccoladeDef->ItemName;
    Info.PlayerController = PlayerController;
    PlayerController->XPComponent->TotalXpEarned += Info.EventXpValue;
    PlayerController->XPComponent->OnXPEvent(Info);
}