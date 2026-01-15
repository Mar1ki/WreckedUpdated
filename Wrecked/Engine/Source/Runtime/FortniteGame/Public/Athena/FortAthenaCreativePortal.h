#pragma once
#include "pch.h"

namespace FortAthenaCreativePortal {

	void Init();
	void CreatePortal(AFortPlayerControllerAthena*);
	void TestProfile(AFortPlayerControllerAthena*);
	void UpdateAllPortals(AFortCreativePortalManager* PortalManager);
	__int64 test(UMcpProfile* a1, FProfileUpdateSingle* a2, __int64 a3, void *a4, char a5);
	DefineOriginal(__int64, test,  UMcpProfile* a1, FProfileUpdateSingle* a2, __int64 a3, void *a4, char a5);
}
