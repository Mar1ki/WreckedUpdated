#pragma once
#include "pch.h"

namespace FortMovementMode_BaseExtLogic {
	UFortMovementMode_BaseExtLogic* GetMovementModeExtById(AFortPlayerPawn* Pawn, FMovementMode_ExtID Id);
	UClass* GetMovementModeExtClassById(AFortPlayerPawn* Pawn, FMovementMode_ExtID Id);

}
