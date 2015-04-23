//! Source code for standard Dll functions

#include "windows.h"

//Declarations
#ifdef WIN32
BOOL CALLBACK LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved);
#else
int WINAPI LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
#endif

//////////////////////
//Globals
HANDLE	hInstance;		// DLL instance handle


#ifdef WIN32
BOOL CALLBACK LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
#else
int WINAPI LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
#endif
{
    hInstance = hModule;
    return 1;
}
