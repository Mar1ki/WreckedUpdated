#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Movement/FortMovementMode_BaseExtLogic.h"

// i will use this mostly for debug values, to check if smth changes in mme itself
UFortMovementMode_BaseExtLogic* FortMovementMode_BaseExtLogic::GetMovementModeExtById(AFortPawn* Pawn, FMovementMode_ExtID Id) 
{
	auto Value = Pawn->RegisteredMovementModeExtentionLogic.Find(Id, [](const FMovementMode_ExtID& Left, const FMovementMode_ExtID& Wright) {return Left == Wright; });
    if (Value.IsValid())
        return Pawn->RegisteredMovementModeExtentionLogic[Value.GetIndex()].Second;
    else
        return nullptr;
}
// Alr, ig this could be used to call UFortMovementComp_Character::ActivateRegisteredMovementModeExtension,  but actually, we dont really care abt that(we can find a better way)
UClass* FortMovementMode_BaseExtLogic::GetMovementModeExtClassById(AFortPawn* Pawn, FMovementMode_ExtID Id)
{
    auto Value = Pawn->RegisteredMovementModeExtentionLogic.Find(Id, [](const FMovementMode_ExtID& Left, const FMovementMode_ExtID& Wright) {return Left == Wright; });
    if (Value.IsValid())
        return Pawn->RegisteredMovementModeExtentionLogic[Value.GetIndex()].Second->Class;
    else
        return nullptr;
}