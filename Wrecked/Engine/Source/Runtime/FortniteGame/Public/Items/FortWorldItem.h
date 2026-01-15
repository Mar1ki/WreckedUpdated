#pragma once
#include "pch.h"

namespace FortWorldItem {
	//bool __fastcall UFortWorldItem::SetLoadedAmmo(UFortWorldItem *this, int InCount)
	DefineOriginal(bool, SetLoadedAmmo, UFortWorldItem* Item, int InCount);
	DefineOriginal(bool, SetPhantomReserveAmmo, UFortWorldItem* Item, int InCount);
	void Init();
}