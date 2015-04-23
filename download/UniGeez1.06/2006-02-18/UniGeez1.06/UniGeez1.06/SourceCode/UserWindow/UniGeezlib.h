#ifndef UNIGEEZLIB_H
#define UNIGEEZLIB_H

//! Entry functions for the DLL

#if !defined(UNIGEEZLIBAPI)
#define UNIGEEZLIBAPI __declspec(dllimport)
#endif

UNIGEEZLIBAPI int WINAPI InstallFilter(int Activate);

UNIGEEZLIBAPI int WINAPI InitHooksDll (const HWND hwndMainWindow, 
									   char _appName[][MAX_CH], 
									   char _format[][MAX_CH], const int _howmany);

UNIGEEZLIBAPI int WINAPI setFontSize  (const int size, HWND hDlg);

#endif
