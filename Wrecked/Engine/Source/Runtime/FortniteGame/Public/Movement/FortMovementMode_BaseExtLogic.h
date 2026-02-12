#pragma once
#include "pch.h"

namespace FortMovementMode_BaseExtLogic {
	UFortMovementMode_BaseExtLogic* GetMovementModeExtById(AFortPawn* Pawn, FMovementMode_ExtID Id);
	UClass* GetMovementModeExtClassById(AFortPawn* Pawn, FMovementMode_ExtID Id);

}
