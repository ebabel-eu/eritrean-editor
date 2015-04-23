/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#ifndef UNIGEEZLIB_H
#define UNIGEEZLIB_H 

//! Entry functions for the DLL

#if !defined(UNIGEEZLIBAPI)
#define UNIGEEZLIBAPI __declspec(dllimport)
#endif

const int MAX_CH = 80;

UNIGEEZLIBAPI BOOL WINAPI InstallFilter(int Activate);

//! appName, format, & howmany are read into shared memory
/* when the user interface window is created, format information is read from a file
   and is passed via this function to the dll for format use determination */

UNIGEEZLIBAPI int  WINAPI InitHooksDll (const HWND hwndMainWindow, 
										char _appName[][MAX_CH], 
									    char _format[][MAX_CH], const int _howmany);

/* size is being passed in from the user interface window (UniGeez) and placed in shared 
   memory between UniGeez and the app into which the dll is injected
   it is the same shared memory regardless of the number of instances in progress */

UNIGEEZLIBAPI int  WINAPI setFontSize  (const int size, HWND hDlg);

#endif