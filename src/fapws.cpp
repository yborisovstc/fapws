// fapws.cpp : Defines the entry point for the DLL application.
//

#include "fapws.h"

#if defined (_PLAT_WIN32_)
// For WIN32 we need Dll entry point

BOOL APIENTRY FapDllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#endif // _PLAT_WIN32_

