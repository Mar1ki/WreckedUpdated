#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/FortniteAI/FortAthenaPatrolPath.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

// if thats ass then fix it
void FortAthenaPatrolPath::GetPatrolPoints(UObject* Object, FFrame& Params)
{
    TArray<AFortAthenaPatrolPoint*> OutPatrolPoints;
    Params.StepCompiledIn(&OutPatrolPoints);
    Params.IncrementCode();

    auto PatrolPath = (AFortAthenaPatrolPath*)Object;

    OutPatrolPoints = PatrolPath->PatrolPoints;
}


AFortAthenaPatrolPoint* FortAthenaPatrolPath::GetRandomPatrolPointFromPatrolPath(AFortAthenaPatrolPath* PatrolPath)
{
    TArray<AFortAthenaPatrolPoint*> RandArray;   

    for (auto PatrolPoint : PatrolPath->PatrolPoints)
    {
        if (PatrolPoint)
        {
            RandArray.Add(PatrolPoint);
        }
    }
    if (RandArray.IsEmpty())
    {
        return nullptr;
    }
    return RandArray[FMath::RandHelper(RandArray.Num())];
}

void FortAthenaPatrolPath::Init()
{
    UHook* Hook = new UHook();
    
    /*Hook->Path = "/Script/FortniteAI.FortAthenaPatrolPath.GetPatrolPoints";
    Hook->Detour = GetPatrolPoints;
    UKismetHookingLibrary::Hook(Hook, Exec); */
    
    delete Hook;
}
