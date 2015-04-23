/* KeyboardInterceptor.cpp
   Main source file for Keyboard Interceptor

Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
This program is free software; redistribution or modification is permitted under the 
terms of the GNU General Public License version 2 as published by the Free Software 
Foundation.*/

#include "windows.h"
#include <stdio.h>
#include "resource.h"
#include "io.h"
#include "unigeezlib.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Function declarations

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);
LRESULT CALLBACK KeyboardWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI About(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
BOOL WINAPI Info (HWND hDlg, unsigned message, WORD wParam, LONG lParam);
BOOL WINAPI Font (HWND hDlg, unsigned message, WORD wParam, LONG lParam);
BOOL WINAPI FontName (HWND hDlg, unsigned message, WORD wParam, LONG lParam);

/////////////////////////////////////////////////////////////////////////////////////////

//Globals
#define WM_NOTIFYICON (WM_USER + 1)

const int ID_ON	 = 1;
const int ID_OFF = 2;

bool infoState = false;					//do we display the prompt?
bool overRideInfoState = false;         //do we override the prompt?

string regKeyFont  ("Software\\UniGeez\\Font");

HINSTANCE hInstance = NULL;				//! instance handle for UniGeez Transliterator
HWND	  hwndMain  = NULL;				//! needed in callback -- main handle
HICON     hIcon     = NULL;

//! global to track the font size state within the main window
bool      fontSizeState = false;		

//! global to track the font size state when the main window losses focus
bool      previousFontState = false;	
HMENU     hMenu;							//! handle to the menu
bool      State = false;					//! current hook state
int       indexFS = 4;						//! index of the selected font size
enum { NO_SHOW_PROMPT, SHOW_PROMPT};

/////////////////////////////////////////////////////////////////////////////////////////

int	WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
    MSG        msgMain;
    WNDCLASS   wc;
    hInstance = hInst;
	int cy, cx;

	//! variables to be filled with format info from the formats.txt file
	const int MAX_APPS  = 100;
	const int MAX_CHARS = 80;
	static int howmany    = 0;
	static char appName[MAX_APPS][MAX_CHARS];
	static char format [MAX_APPS][MAX_CHARS];

    //! Register the window class if this is the first instance.
    if (hInstPrev == NULL)
    {
		wc.lpszMenuName		= "LangOptions";
		wc.lpszClassName	= "UniGeez v1.05";
        wc.hInstance        = hInst;
		wc.hIcon			= LoadIcon(hInst, "UniGeezIcon");
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(CreateSolidBrush(RGB(255, 76, 1)));
        wc.style            = 0;
		wc.lpfnWndProc		= KeyboardWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;

        if (!RegisterClass(&wc))
            return(0);
    }

	hIcon = wc.hIcon;

    // Get center point of user's screen
	cy = ::GetSystemMetrics(SM_CYSCREEN) / 2; 
    cx = ::GetSystemMetrics(SM_CXSCREEN) / 2;
	
	//! Create the main window
    if ((hwndMain = CreateWindow("UniGeez v1.05", "UniGeez Transliterator",
                                 WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | 
								 WS_OVERLAPPED, cx - 250, cy - 135, 275, 150,
                                 NULL, NULL, hInst, NULL)) == NULL)
        return(0);

    //! show window and update it
    ShowWindow(hwndMain, cmdShow);		
    UpdateWindow(hwndMain);

	//! read format info from a file and pass it to the dll into shared memory
	howmany = readFormatFile(appName, format, MAX_APPS);
	InitHooksDll(hwndMain, appName, format, howmany);

    //! Main message loop
    while (GetMessage((LPMSG) &msgMain, NULL, 0, 0))	
    {	
		
       TranslateMessage((LPMSG) &msgMain);
		
       DispatchMessage((LPMSG) &msgMain);	
    }

    return(0);
}

HWND hProc = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
//! KeyboardWndProc


LRESULT CALLBACK KeyboardWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	hProc = hwnd;
	const int TWO_PUSHBUTTONS = 2;
	static HWND hButton[TWO_PUSHBUTTONS];
	char* button[TWO_PUSHBUTTONS] = {"Ge'ez Script ON", "Ge'ez Script OFF"};
	string regKeyPrompt("Software\\UniGeez\\Prompt");
	string defaultFont ("GF Zemen Unicode");
	int i = 0;
	unsigned char val = 0;
	hMenu = GetMenu(hwnd);
	static bool fEnable   = false;				//do we enable font size selection?
	static bool isPromptRegKey  = false;		//registery key for prompt
	static bool isFontRegKey    = false;        //registery key for font name

	//the dll is going to pass us info about whether or not to enable the font
	//size selection feature
	PCOPYDATASTRUCT pcds  = (PCOPYDATASTRUCT)lParam;
	int* ipcds = NULL;
	enum { DISABLE_FONTSIZE, ENABLE_FONTSIZE };

	//data for loading taskbar icon
	static NOTIFYICONDATA pnid;
	pnid.cbSize = sizeof(NOTIFYICONDATA);
	pnid.hWnd   = hwnd;
	pnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	pnid.hIcon  = (HICON)LoadImage(hInstance, "UniGeezIcon", IMAGE_ICON, 16, 16, 0);
	lstrcpyn(pnid.szTip, "UniGeez", sizeof(pnid.szTip)); 
	pnid.uCallbackMessage = WM_NOTIFYICON;
	POINT pt;
	static HMENU hTBMenu = NULL;

	//variables for bitmap background
    static HBITMAP hBitmap;
    static int cxClient = 0;
	static int cyClient = 0;
	static int cxSource = 0;
	static int cySource = 0;
    BITMAP bitmap;
    HDC hdc, hdcMem;
    int x = 0;
	int y = 0;
    PAINTSTRUCT ps;

    switch(msg) 
	{
		case WM_CREATE:

			//load bitmap
            hBitmap = LoadBitmap(hInstance, TEXT("Paper"));

            GetObject(hBitmap, sizeof(BITMAP), &bitmap);

			//get dimensions of bitmap
            cxSource = bitmap.bmWidth;
            cySource = bitmap.bmHeight;

			//get handle to taskbar menu
            hTBMenu = LoadMenu(hInstance, "TBMENU") ;
            hTBMenu = GetSubMenu(hTBMenu, 0) ;

			//add icon to taskbar status area
			Shell_NotifyIcon(NIM_ADD, &pnid);

			//uncomment to delete the font name reg key
			//RegDeleteKey(HKEY_CURRENT_USER, regKeyFont.c_str());

			//determine whether a registry key has been created for the prompt
			isPromptRegKey = isRegKeyCreated(regKeyPrompt, val);

			//determine whether a registry key has been created for the font name
			isFontRegKey = isRegKeyCreated(regKeyFont);

			//create a registry key for GF Zemen Unicode
			if(!isFontRegKey)
				createRegKey(regKeyFont, defaultFont);

			//a registry key for UniGeez does not exist, so show the prompt
			if(!isPromptRegKey || val == SHOW_PROMPT)
			{
				CheckMenuItem(hMenu, ID_OPTIONS_INFO, MF_CHECKED);
				infoState = true;
			}

			//don't show the prompt
			else if(val == NO_SHOW_PROMPT)
			{
				CheckMenuItem(hMenu, ID_OPTIONS_INFO, MF_UNCHECKED);
				infoState = false;
			}

			//create a push button
			hButton[0] = CreateWindow(TEXT("button"), 0, WS_CHILD | WS_VISIBLE | 
				                      BS_BITMAP, 10, 10, 120, 25, hwnd,
									  (HMENU)ID_ON, ((LPCREATESTRUCT) lParam)->hInstance, 
									  NULL);

			//load bitmap image on the button
			SendMessage(hButton[0], BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)
				LoadImage(hInstance, "GZON", IMAGE_BITMAP, 120, 25, 0));

			//create another push button
			hButton[1] = CreateWindow(TEXT("button"), 0, WS_CHILD | WS_VISIBLE | 
				                      BS_BITMAP, 10, 45, 120, 25, hwnd,
									  (HMENU)ID_OFF, ((LPCREATESTRUCT) lParam)->hInstance, 
									  NULL);

			//load bitmap image on the button
			SendMessage(hButton[1], BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)
				LoadImage(hInstance, "GZOFF", IMAGE_BITMAP, 120, 25, 0));

			return 0;

		case WM_SIZE:

			//resize bitmap 
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);
            return 0;

		//taskbar icon has been clicked on
		case WM_NOTIFYICON:

			switch(lParam)
			{
			case WM_LBUTTONDOWN:

				//minimize the main window
				SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return 0;

			case WM_LBUTTONDBLCLK:

				//restore the main window
				SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
				return 0;


			case WM_RBUTTONDOWN:

				pt.x = ::GetSystemMetrics(SM_CXSCREEN) - 100;
				pt.y = ::GetSystemMetrics(SM_CYSCREEN) - 50;

				//display taskbar popup menu
				SetForegroundWindow(hwnd);
				TrackPopupMenuEx(hTBMenu, TPM_RIGHTBUTTON | TPM_RIGHTALIGN | 
					             TPM_BOTTOMALIGN, pt.x, pt.y, hwnd, NULL);
				PostMessage(hwnd, WM_NULL, 0, 0);
          
				return 0;
			}
			break;

		case WM_INITMENUPOPUP:

		  //enables or disables font size selection based on fEnable
          EnableMenuItem((HMENU)wParam, IDM_OPTIONS_FONTSIZE, 
			             fEnable ? MF_ENABLED : MF_GRAYED);
		  break;

		//we are receiving this message from the dll
		case WM_COPYDATA:

			if(pcds->lpData)	
			{ 
				ipcds = static_cast <int*> (pcds->lpData);

				//if we get a 0 from the dll, we disable the font size selector
				if(*ipcds == DISABLE_FONTSIZE && !fontSizeState)	
				{ 
					fEnable = false; 
				}

				//we got a 1 so enable the same feature
				else if(*ipcds == ENABLE_FONTSIZE)	
				{ 
					fEnable           = true;  
					fontSizeState     = true;		
					previousFontState = true;	//global to track the font size state
				}
			}
			break;

		case WM_KILLFOCUS:

			//this window is losing focus so deselect the font size
			fontSizeState = false;
			break;
			
		case WM_COMMAND:

			switch (LOWORD(wParam) ) 
			{

				case ID_OPTIONS_INFO:

					//if selected, deselect
					if(infoState)
					{
						CheckMenuItem(hMenu, ID_OPTIONS_INFO, MF_UNCHECKED);
						CheckMenuItem(hTBMenu, ID_OPTIONS_INFO, MF_UNCHECKED);
						infoState = false;
					}

					//it's deselected, so select it!
					else
					{
						CheckMenuItem(hMenu, ID_OPTIONS_INFO, MF_CHECKED);
						CheckMenuItem(hTBMenu, ID_OPTIONS_INFO, MF_CHECKED);
						infoState = true;
					}
					break;

				case ID_ON:	
				case ID_MYMENU_GEEZ_ON:

					SendMessage(hButton[1], BM_SETSTATE, 0, 0);
					SendMessage(hButton[0], BM_SETSTATE, 1, 0);
					EnableMenuItem(hTBMenu, ID_MYMENU_GEEZ_ON, MF_GRAYED);
					EnableMenuItem(hTBMenu, ID_MYMENU_GEEZ_OFF, MF_ENABLED);

					//if the hook is not already installed
					if(!State)	
					{ 
						//then install it!!!
						InstallFilter(TRUE);
						State = TRUE;

						//only launch this dialog box if the prompt is deselected
						if(infoState && !overRideInfoState)	
						{ 
							DialogBox(hInstance, "INFOBOX", hwndMain,(DLGPROC)Info);
						}

						overRideInfoState = false;
					}
					break;

				case ID_OFF:
				case ID_MYMENU_GEEZ_OFF:

					SendMessage(hButton[0], BM_SETSTATE, 0, 0);
					SendMessage(hButton[1], BM_SETSTATE, 1, 0);
					EnableMenuItem(hTBMenu, ID_MYMENU_GEEZ_ON, MF_ENABLED);
					EnableMenuItem(hTBMenu, ID_MYMENU_GEEZ_OFF, MF_GRAYED);

					if(State)
					{
						//turn the system-wide hook off
						InstallFilter(FALSE);
						State = FALSE;
					}
					break;

				case IDM_ABOUT:

					//display the About Box
					DialogBox(hInstance, "ABOUTBOX", hwndMain,(DLGPROC)About);
					break;

				//display the Font Size box
				case IDM_OPTIONS_FONTSIZE:

					DialogBox(hInstance, "FONTSIZE", hwndMain, (DLGPROC)Font);
					break;

				//display the Font Name box
				case IDM_OPTIONS_FONTNAME:

					DialogBox(hInstance, "FONTNAME", hwndMain, (DLGPROC)FontName);
					break;

				case IDM_FILE_EXIT:

					PostMessage(hwnd, WM_DESTROY, 0, 0);
					break;
			}					

	    break;

		//paint the bitmap
		case WM_PAINT:

            hdc = BeginPaint(hwnd, &ps);

            hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, hBitmap);

            for (y = 0; y < cyClient; y += cySource)
				for (x = 0; x < cxClient; x += cxSource)
					BitBlt(hdc, x, y, cxSource, cySource, hdcMem, 0, 0, SRCCOPY);

            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);

            return 0;

        case WM_DESTROY:			//closing application window

			//delete bitmap object
			DeleteObject(hBitmap);

			//remove icon from taskbar
			Shell_NotifyIcon(NIM_DELETE, &pnid);

			//if app is closed & hook is still installed -- remove it!
			if ( State )	
			{
				InstallFilter(FALSE);
			}

			//create a registry key if one has not been created 
			if(!isPromptRegKey)
				createRegKey(regKeyPrompt, infoState);

			//otherwise, simply update it
			else
				updateRegKey(regKeyPrompt, infoState);

			PostQuitMessage(0);	
			break;

        default:
            return(DefWindowProc(hwnd, msg, wParam, lParam));
    }

    return(0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Dialog procedure for about box

BOOL WINAPI About(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	//if font size was previously enabled, 
	//don't let the creation of this dialog box disable it
	if(previousFontState)		{ fontSizeState = true; }

    switch(message) 
	{
		case WM_INITDIALOG:

			//place the GNU bitmap on the push button
			SendMessage(GetDlgItem(hDlg, IDC_GNU), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)
				LoadImage(hInstance, "GNUBMP", IMAGE_BITMAP, 32, 32, 0));

			//place the UniGeez icon on the update push button
			SendMessage(GetDlgItem(hDlg, IDC_UPDATES), BM_SETIMAGE, IMAGE_ICON, (LPARAM)
				LoadImage(hInstance, "UniGeezIcon", IMAGE_ICON, 32, 32, 0));

			return (TRUE);

		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
			case IDCANCEL:
				
				EndDialog(hDlg, TRUE);		
				return (TRUE);

			case IDC_GNU:

				//launch the default browser
				ShellExecute(hDlg, NULL, "www.gnu.org/copyleft/gpl.html", 
					         NULL, NULL, SW_SHOWNORMAL);
				return (TRUE);

			case IDC_UPDATES:

				//launch the default browser
				ShellExecute(hDlg, NULL, "www.punchdown.org/rvb/email/UniGeez2k09.html", 
					         NULL, NULL, SW_SHOWNORMAL);
				return (TRUE);
			}

	    break;
    }
    return (FALSE);							 
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Dialog procedure for info box

BOOL WINAPI Info(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	//if font size was previously enabled, 
	//don't let the creation of this dialog box disable it
	if(previousFontState)		{ fontSizeState = true; }

    switch (message) 
	{
		case WM_INITDIALOG:				
			return (TRUE);

		case WM_COMMAND:	
			switch (LOWORD(wParam))
			{	
			case IDCANCEL:
			case IDOK:

				EndDialog(hDlg, TRUE);		
				return (TRUE);

			case ID_CHECK:

				CheckMenuItem(hMenu, ID_OPTIONS_INFO, MF_UNCHECKED);
				infoState = false;

				return (TRUE);
			}
	    break;
    }
    return (FALSE);							 
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Dialog procedure for font box

BOOL WINAPI Font(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	//font sizes for the combo box
	const int MAX_ENTRIES = 16;
	string pitch[MAX_ENTRIES];
	pitch[0]  = "8";
	pitch[1]  = "9";
	pitch[2]  = "10";
	pitch[3]  = "11";
	pitch[4]  = "12";
	pitch[5]  = "14";
	pitch[6]  = "16";
	pitch[7]  = "18";
	pitch[8]  = "20";
	pitch[9]  = "22";
	pitch[10] = "24";
	pitch[11] = "26";
	pitch[12] = "28";
	pitch[13] = "36";
	pitch[14] = "48";
	pitch[15] = "72";
	
	//get the handle to the fontsize combobox
	HWND hCtrl = GetDlgItem(hDlg, IDC_COMBO_FONTSIZE);

	if(previousFontState)		{ fontSizeState = true; }
    switch (message) 
	{
		case WM_INITDIALOG:
			
			//populate combo box with font sizes
			insertPitch(hCtrl, pitch, MAX_ENTRIES);
			SendMessage(hCtrl, CB_SETCURSEL, (WPARAM)indexFS, 0);
			return (TRUE);

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
			case IDCANCEL:
			case IDOK:

				EndDialog(hDlg, TRUE);		
				return (TRUE);

			case IDC_COMBO_FONTSIZE:

				//get the font size and pass it to the dll
				indexFS = SendMessage(hCtrl, CB_GETCURSEL, 0, 0);
				setFontSize(atol(pitch[indexFS].c_str()), hDlg);
				return (TRUE);
			}

			break;

    }
    return (FALSE);							 
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Dialog procedure for fontname box

BOOL WINAPI FontName(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	const int MAX_FONTNAMES = 10;
	const int MAX_BUFFER = 64;
	char buff[MAX_BUFFER];
	memset(buff, 0, MAX_BUFFER);
	static bool tempHookOff = false;

	//if font size was previously enabled, 
	//don't let the creation of this dialog box disable it
	if(previousFontState)		{ fontSizeState = true; }

	//get the handle to the fontsize combobox
	HWND hCtrl = GetDlgItem(hDlg, IDC_COMBO_FONTNAME);

	//get handle to the insert font box
	HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_INSERT_FONT);

	//dynamic array to hold the font names from the combobox
	static string* fName = new string[MAX_FONTNAMES +1];

    switch (message) 
	{
		case WM_INITDIALOG:	

			//if hook is installed, uninstall it; necessary so the user can 
			//type in the edit box
			if(State)
			{
				tempHookOff = true;
				overRideInfoState = true;
				SendMessage(hProc, WM_COMMAND, MAKELONG(ID_OFF, 0), 0);
			}
			
			//populate combo box with font names from the registery
			insertFontNames(hCtrl, regKeyFont, fName);

			//select the font name at index 0
			SendMessage(hCtrl, CB_SETCURSEL, (WPARAM)0, 0);

			//set focus in the edit box
			SetFocus(hEdit);

			break;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
			case IDCANCEL:
			case IDOK:

				//get the current selection in the edit box
				GetDlgItemText(hDlg, IDC_EDIT_INSERT_FONT, buff, MAX_BUFFER);

				//if nothing is there, get the curr selection in the combobox
				if(buff[0] == '\0')
					GetDlgItemText(hDlg, IDC_COMBO_FONTNAME, buff, MAX_BUFFER);
			
				//update the font names in the registry key
				updateFontName(hCtrl, buff, fName, regKeyFont, MAX_FONTNAMES);

				//if we turned the hook off, turn it back on
				if(tempHookOff)
				{
					tempHookOff = false;
					SendMessage(hProc, WM_COMMAND, MAKELONG(ID_ON, 0), 0);
				}

				EndDialog(hDlg, TRUE);		
				return (TRUE);

			case IDC_COMBO_FONTNAME:

				break;
			}

			break;

    }
    return (FALSE);									 
}
