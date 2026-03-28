#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortAthenaPatrolPath
{
    void GetPatrolPoints(UObject* Object, FFrame& Params);
    AFortAthenaPatrolPoint* GetRandomPatrolPointFromPatrolPath(AFortAthenaPatrolPath* PatrolPath);
    void Init();
}
