#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace AthenaMarkerComponent
{

    DefineOriginal(void, ServerAddMapMarker, UObject* Context, FFrame& Stack);
    DefineOriginal(void, ServerRemoveMapMarker,UObject* Context, FFrame& Stack);
    static char (__fastcall* BuildMarkerDataForActor)(UAthenaMarkerComponent* Component, AActor* HitActor, FFortWorldMarkerData* OutMarker) = (decltype(BuildMarkerDataForActor))(ImageBase + 0x9131F68);
    static char (__fastcall* LocalPlaceMapMarker)(UAthenaMarkerComponent* Component, const FFortWorldMarkerData *InMarkerData) = (decltype(LocalPlaceMapMarker))(ImageBase + 0x914371C);
    
    void Init();
};
