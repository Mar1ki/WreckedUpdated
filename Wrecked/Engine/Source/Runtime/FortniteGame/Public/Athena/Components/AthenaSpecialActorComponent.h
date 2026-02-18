#pragma once
#include "pch.h"

namespace AthenaSpecialActorComponent
{
    void Init();
    DefineOriginal(void, Activate, UAthenaSpecialActorComponent* Component, bool bReset);
    
    static void (__fastcall* AddActorToReplicationList)(AFortSpecialActorReplicationInfo* ReplicationInfo, FSpecialActorRepData* RepData) = (decltype(AddActorToReplicationList))(ImageBase + 0x9A7BF4C);
    // 0x9A7BF4C 
};
