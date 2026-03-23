#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortWeaponRanged {
	
	void ServerNotifyPawnHit(UObject* Context, FFrame& Stack);
	void Init();
}
