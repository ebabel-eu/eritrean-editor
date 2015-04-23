//! \file UniGeezLibdll.cpp
//! \author Marcus Wright and Will Briggs (Lynchburg College)
//! \date October2000

/*!	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. 

    UniGeez consists of 2 parts:  the user interface window and this DLL.  The Dll 
	utilizes 2 system-wide hooks to intercept the messages of all the processes running 
	in the OS. When the user clicks the 'ON' button in the user interface window, the 
	hook is installed by injecting this DLL into the address space of all processes 
	running.  When the user clicks 'OFF' the hook is uninstalled.

	The first hook, WH_GETMESSAGE, hooks or intercepts messages whenever an application
	calls the GetMessage function to retrieve a message from the application message 
	queue. Before passing the retrieved message to the destination window procedure, the 
	system passes the message to the hook procedure for evaluation and potential 
	modification. UniGeez uses the WH_GETMESSAGE hook to intercept and modify 3 messages:  
	WM_KEYDOWN, WM_KEYUP, and WM_CHAR.  These messages can be effectively nullified by 
	setting them equal to WM_NULL and then passing them on to the destination window 
	procedure. They can also be changed by changing their wParam value.  Say, for 
	instance that you have a WM_KEYDOWN message with a wParam equal to 'A'; this can be 
	changed to 'X', and an 'X' will be output in the window with cursor focus.  The 
	particular challenge of this project was to change the wParam of these keyboard 
	messages from single-byte ASCII to double-byte Unicode.  However, prior to Win2000, 
	this cannot be done; the high-byte gets truncated.

    The second hook, WH_CBT, hooks actions such as activating, creating, destroying, 
	minimizing, maximizing, moving, or resizing a window.  It is used specifically by this 
	DLL to hook the WM_SETFOCUS message, so that when a application first receives cursor 
	focus a determination can be made as to which application it is and what clipboard 
	format needs to be utilized.

    That brings us to the question of how we are able to use these hooks to achieve 
	Unicode character output in an application.  The WM_KEYDOWN messages are looked at 
	to determine what key was pressed, and they are converted to appropiate character 
	codes.  For example, the WM_KEYDOWN with wParam == 'A' means that the 'a' key was 
	pressed, but does not tell you whether it's uppercase or lowercase.  The GetShiftState
	function is used to determine whether or not the shift-state in the keyboard driver 
	is true or false, and this information along with the WM_KEYDOWN code is used to 
	determine the appropiate character code.  These character codes are stored in the 
	BasicEngine class, and the AmharicEngine uses a finite state machine to determine the 
	appropiate Unicode value that needs to replace one or more of these character codes.  
	When a Unicode value is ready for output, it is formatted as either RTF, UNICODE, 
	or HTML and placed on the system clipboard.  Then the keybd_event function is used 
	to simulate the press and release of the CTRL-key and the V-key, so that the window 
	with cursor focus receives a paste message and the Unicode character(s) that was
	previously placed on the clipboard is pasted into the application.  In some instances, 
	the second key pressed by the user changes the meaning of the previous character 
	output so a backspace event is created to erase the last character and a new one 
	is pasted in to replace it.  Using this method, we can achieve the output of Unicode 
	characters in numerous applications.

	Different applications require different clipboard formats to achieve the pasting of 
	Unicode characters into their window.  A text file is used with different application 
	class names and the corresponding format to use with that application.  When the user 
	interface window is opened, this information is read from the file, passed to the 
	DLL, and stored in shared memory, so that the appropiate format can be used with the 
	proper application. The Unicode format is the default format, unless the format.txt 
	file specifies the use of another format. The .txt file is used so that format use 
	can be easily changed without recompiling the program.  If it is found that an 
	application can better use another format other than Unicode, then this can be easily 
	changed.  This is a good overview of what is taking place.  There are more notes at  
    the end of this file, and other comments can be found throughout the files. */

/////////////////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include <windowsx.h>
#include <string>
using namespace std;
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define  UNIGEEZLIBAPI __declspec(dllexport)
#include "UniGeezLib.h"
#include "clipboard.h"
#include "filterfunctions.h"
#include "AmharicEngine.h"
#include "msgfilter.h"
#include "mode.h"
#include "NTandWord.h"


/////////////////////////////////////////////////////////////////////////////////////////
//	Function declarations

BOOL	CALLBACK LibMain (HANDLE hModule, DWORD dwReason, LPVOID lpReserved);
LRESULT CALLBACK MsgFilterFunc (int nCode, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK CBTFilterFunc (int nCode, WPARAM wParam, LPARAM lParam );

//////////////////////////////////////////////////////////////////////////////////////////
// Globals

HANDLE	hInstance;									//! DLL instance handle
BOOL	InitCalled = FALSE;							//! whether the hook is installed

BOOL hookState = FALSE;						        //! state of hooks
FARPROC msgHookProc = { (FARPROC) MsgFilterFunc };  //! address for msg filter function
FARPROC cbtHookProc = { (FARPROC) CBTFilterFunc };  //! address for CBT filter function

bool ExcelMode    = false;							//! are we in Excel??

//! Shared DATA set up between UniGeez and the injected dll
//! only one per instance of the UniGeez interface

#pragma data_seg("Shared")
HWND  hwndUniGeez    = NULL;				//! main handle from UniGeez Transliterator
HHOOK hMsgHook       = NULL;				//! handle to the installed WH_GETMESSAGE hook
HHOOK hCBTHook       = NULL;				//! handle to the installed WH_CBT hook
HWND  hCurrent       = NULL;
HWND  hFontDlg       = NULL;				//! handle to dialog box in UniGeez (not used)

//! determines what format is to be used
bool useRtf          = false;
bool useHtml         = false;
bool useUnicode      = true;
int howmany          = 0;
int fs               = 12;					//! used to determine font size

//! format info read in from UniGeez userinterface window
bool oneTimeReadFile = true;
const int MAX_APPS   = 100;
const int MAX_CHARS  = 80;
char appName[MAX_APPS][MAX_CHARS] = {"\0"};
char format [MAX_APPS][MAX_CHARS] = {"\0"};

#pragma data_seg()
#pragma comment(linker,"/SECTION:Shared,RWS")

///////////////////////////////////////////////////

BOOL CALLBACK LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    hInstance = hModule;
    return 1;
}

//! initializes the hook
int CALLBACK InitHooksDll(const HWND hwndMainWindow, char _appName[][MAX_CH], 
						  char _format[][MAX_CH], const int _howmany)
{
	//! get format info from user interface window one time only
	//! and read into shared memory
	if(oneTimeReadFile)
	{
		for(int i = 0; i < _howmany; i++)
		{
			int j = 0;
			while(_appName[i][j] != '\0')
				appName[i][j] = _appName[i][j++];
		
			j = 0;
			while(_format[i][j] != '\0')
				format [i][j] = _format [i][j++];
		}
		oneTimeReadFile = false;
		howmany = _howmany;
	}

    hwndUniGeez	= hwndMainWindow;		//handle to UniGeez
    InitCalled	= TRUE;
    return (0);
}

//! sets the current font size
int CALLBACK setFontSize(const int size, HWND hDlg)
{
	//! get font size info from UniGeez
	fs = size;
	hFontDlg = hDlg;	//not used
	return (0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//! returns the HMODULE that contains the specified memory address

static HMODULE ModuleFromAddress(PVOID pv)
{
	MEMORY_BASIC_INFORMATION mbi;
	return ((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0)
		? (HMODULE) mbi.AllocationBase : NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Install / Remove Filter function

BOOL WINAPI InstallFilter (INT nCode)
{
	BOOL fOK;
	if (!InitCalled) { return (-1); }

    if (nCode)
	{
		assert(hMsgHook == NULL);		//! illegal to install twice in a row
		assert(hCBTHook == NULL);

		//! set the GETMESSAGE hook system-wide!!
		hMsgHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC) msgHookProc,
									ModuleFromAddress(InstallFilter), 0);

		//! set the CBT hook system-wide!!
		//! to determine whether or not a window has just received cursor focus
		hCBTHook = SetWindowsHookEx(WH_CBT,        (HOOKPROC) cbtHookProc,
									ModuleFromAddress(InstallFilter), 0);
	
		//! a hook has been installed!
		hookState = TRUE;
		fOK = (hMsgHook != NULL);
    }
    else 
	{	
		assert(hMsgHook != NULL);		//! can't uninstall if not installed
		assert(hCBTHook != NULL);

		//! release the hooks
		fOK = UnhookWindowsHookEx(hMsgHook); 	hMsgHook = NULL;
		      UnhookWindowsHookEx(hCBTHook);	hCBTHook = NULL;

		hookState = FALSE;		//no longer in a hook state
		InvalidateRect(hwndUniGeez, NULL, TRUE);
		UpdateWindow(hwndUniGeez);
    }

	return (fOK);
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Filter function for the WH_GETMESSAGE

/*! this is where we intercept the Keystrokes that are taking place system-wide
    not only are we hooking the window with the cuurent focus, but we are also
    hooking anything running in the background!!! */

static struct NTandWord NTandWord;  //! global for this function and the next

LRESULT CALLBACK MsgFilterFunc (INT nCode, WPARAM wParam, LPARAM lParam)
{	
	//! going to use this a lot so keep it simple
	#define wparam newMsg->wParam
	#define msg    newMsg->message

	//! static variables exist separately for each instance of an injected hook in a dll
	static MODE Mode                 = GEEZ;
	static bool bufferableKeystroke  = false;
	static bool useLowercase         = true;
	static bool capsToggled          = false;
	static bool shiftState           = false;
	static bool bufferOvrdNum        = false;
	static bool allowBackSpace       = false;
	static bool overrideStateRM      = false;
	static bool firstTime            = true;
	static bool shiftLeftForExcel    = false;
	static UINT timeShiftDown        = 0;
	static UINT previousKeydown      = 0;
	static AmharicEngine myEngine;
	static Clipboard clipboard;

	static int yy = 20;

	//! used to output the current mode to the UniGeez window
	HDC hDC;
	hDC = GetDC(hwndUniGeez);
	SetBkColor(hDC, RGB(255, 255, 185));

	MSG* newMsg;
	newMsg = (MSG *) lParam;
	bool isPaste      = false;		//! reset this everytime through the filter-function
	bool isMouseClick = false;

	//! looking for legitimate messages with legitimate handles
	//! PM_REMOVE flag specifies that the message has been removed from the queue
	if(nCode >= 0 && newMsg && newMsg->hwnd && (wParam & PM_REMOVE)) 
	{
		//! set ExcelMode if we are in Excel
		handleExcelMode(myEngine, ExcelMode, shiftLeftForExcel);

		//! use a mouse click as a trigger event
		mouseClickForTriggerEvent(msg, isMouseClick, myEngine);

		//! handle the WM_CHAR messages
		if(Mode != ROMAN && !NTandWord.state)
			handleWM_CHAR(msg, bufferableKeystroke, wparam);

		//! handle the WM_KEYUP messages
		handleWM_KEYUP(newMsg, overrideStateRM, timeShiftDown, useLowercase,
			           shiftState, Mode);

//---------------------Keydown Messages--------------------------------------------

		//! processing KEYDOWN's
		if(msg == WM_KEYDOWN || isMouseClick)
		{	
			/*! debug info for outputting text into the user interface window
		    char string[100];
			sprintf(string, "WM_KEYDOWN:  hwnd = %x  wParam = %x  lParam = %x  
			        time = %d  address = %d", newMsg->hwnd, newMsg->wParam, 
					newMsg->lParam, newMsg->time, (int)newMsg);
			TextOut(hDC, 20, yy, string, strlen(string));
			yy += 15;*/
		
			//! determine current mode -- 
			//! CTRL & ALT pressed together will switch between 2 modes
			if(GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CONTROL) < 0)
			{
				if(Mode == ROMAN)	
				{ 
					Mode = GEEZ; ; 
				}
				else			
				{  
					Mode = ROMAN; 
					myEngine.reset();
					myEngine.setForceBackspace(false);

					//! reset static variables so that when we return to GEEZ mode
					//! everything is cleared for a clean start
					bufferableKeystroke  = false;
					useLowercase         = true;
					capsToggled          = false;
					shiftState           = false;
					bufferOvrdNum        = false;
					allowBackSpace		 = false;
					overrideStateRM      = false;
					firstTime            = true;
					shiftLeftForExcel    = false;
					timeShiftDown        = 0;
					previousKeydown      = 0;
				}
			}

			//! switch to Roman mode for keyboard events that involve the CTRL-key
			switchToRM(Mode, previousKeydown, wparam, overrideStateRM);

//! --------------------Ge'ez Mode from here on out------------------------------
			
			if(Mode == GEEZ)
			{
				//! determine whether this 'V' part of a paste message
				if(GetKeyState(VK_CONTROL) < 0 && wparam == 'V')	
				{ 
					isPaste = true; 
					bufferOvrdNum = false; 
				}

				//! handle everything that deals with the shift-key & caps lock
				shiftModes(shiftState, timeShiftDown, useLowercase, capsToggled,
					       newMsg->time);

				//! Backspacing and Resetting the Buffer
				handleBackspacing(myEngine, previousKeydown, wparam, allowBackSpace, 
					              ExcelMode, NTandWord);

				//! change the Keydown code into its ASCII equivalent
				WCHAR ch = keydownConvertor(useLowercase, bufferableKeystroke, wparam, 
					                        ExcelMode);

				//! handle tasks related to the numbers
				handleNumbers(ch, myEngine, wparam, isMouseClick, bufferOvrdNum,
					          shiftState, bufferableKeystroke, NTandWord);
				
				//! set the clipboard formats and fontsize
				clipboard.setFormats(useUnicode, useRtf, useHtml, fs);

				//! handle backspacing for Excel Mode
				if(ExcelMode && wparam == VK_BACK)
					backspacingForExcel(ch, msg, bufferableKeystroke, myEngine);

				//! DO NOT buffer VK_CONTROL, 
				//! 'V' that is part of a paste sequence, & non-char's
				if(wparam != VK_CONTROL && !isPaste && 
					(bufferableKeystroke || bufferOvrdNum))

					//! Converting ASCII to Ge'ez
					handleAsciiConversion(wparam, myEngine, NTandWord, shiftLeftForExcel, 
					                      isMouseClick, clipboard, ch, allowBackSpace, 
										  hwndUniGeez, bufferOvrdNum);

				//! for Excel, trigger events always reset the buffer
				if(myEngine.inExcelMode() && triggerEvent(wparam, ExcelMode))
				{
					myEngine.reset();
					myEngine.clearToSend();
				}
				
				previousKeydown = wparam;		//! tracks the previous Keystroke

				//! determine which Keydown messages to throw out
				if(wparam != VK_CONTROL && !isPaste && !allowBackSpace &&
				   (bufferableKeystroke || bufferOvrdNum))			
				{ 
					if(!NTandWord.state)
						msg = WM_NULL;
				}
			}
			//! necessary to track the previous keystroke while in Roman mode
			if(overrideStateRM)
				previousKeydown = wparam;
		}
	}
	//! used to output the current mode
	ReleaseDC(hwndUniGeez, hDC);			

	//! passes the message on to the next hook (if any) and then to editor
	return (CallNextHookEx(hMsgHook, nCode, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////////////////
//! Filter function for the WH_CBT hook

/*! This hook catches the event of a window getting focus.
    It is used to determine when the user switches cursor focus; it takes the
    window's handle and traces it back up to its parent window.
    A determination is made on what clipboard format to use based on a format
    text file as the parents window's handle is compared to the handled returned
    from an application's name. */

LRESULT CALLBACK CBTFilterFunc (INT nCode, WPARAM wParam, LPARAM lParam)
{	
	hCurrent = (HWND)wParam;
	enum {OFF, ON};
	int bit = OFF;

	//! struct for sending data to the user interface (UniGeez)
	COPYDATASTRUCT cds;
	cds.dwData = 0;		
	cds.cbData = sizeof(bit);
	cds.lpData = &bit;

	if(nCode >= 0 && hCurrent && hCurrent != hwndUniGeez)
	{
		switch(nCode)
		{
		case HCBT_SETFOCUS:

			//! determine whether or not we are in WinNT and using Word
			//! and let that determine how we will erase char's (see note #10 at bottom)
			if(isWinNTandWord(hCurrent))
			{
				NTandWord.state = true;
			}
			else
				NTandWord.state = false;

			//! determine if we are in EXCEL
			if(parentWindowFinder(hCurrent, "XLMAIN"))
				ExcelMode = true;
			else
				ExcelMode = false;

			for(int j = 0; j < howmany; j++)
			{
				//! tell the user interface window to enable the font size selector
				//! for either the HTML format or RTF, if the current window belongs
				//! to one of the window owner's in the format file
				if(parentWindowFinder(hCurrent, appName[j]))
				{
					//! use HTML
					if(strncmp(format[j], "HTML", 4) == 0)	
					{ 
						useHtml = true; useUnicode = false; useRtf = false; bit = ON;
						SendMessage(hwndUniGeez, WM_COPYDATA, (WPARAM)hCurrent, 
							        (LPARAM)&cds);
						break;
					}

					//! useRTF
					else if(strncmp(format[j], "RTF", 3) == 0)
					{
						useRtf = true; useUnicode = false; useHtml = false; bit = ON;
						SendMessage(hwndUniGeez, WM_COPYDATA, (WPARAM)hCurrent, 
							        (LPARAM)&cds);
						break;
					}
				}
				else  
				{ 
					//! tell the user interface window to disable the font size selector
					useUnicode = true; useRtf = false; useHtml = false;
					SendMessage(hwndUniGeez, WM_COPYDATA, (WPARAM)hCurrent, 
						        (LPARAM)&cds);
				}
			}

			break;
		}
	}

	return (CallNextHookEx(hCBTHook, nCode, wParam, lParam));
}

/*! Notes:

1) To bring interception of the keyboard down to its most basic level, I deal with
   WM_KEYDOWN messages instead of WM_CHAR messages.  Blocking WM_CHAR messages in Excel 
   was not enough to prevent the output of the characters.  This entails much more coding, 
   since WM_KEYDOWN messages use different keycodes than their WM_CHAR counterpart, 
   but seemed to me necessary in order to incorporate as many apps as possible.

2) Since we are dealing with WM_KEYDOWN messages, it is necessary to kept track of the 
   shift states.

3) Also, you will notice that I make a distinction between bufferableKeystrokes and 
   non-bufferable ones.  By bufferable I mean keystrokes that I want to convert to their 
   character equivalent and run through the IdentifyChar function, which determines final 
   character output.  Futhermore, it is used to mean those characters that can be copied 
   to the clipboard and pasted successfully. For instance, you can't place the character 
   code for BackSpace into the clipboard and have it backspace after a paste sequence.  
   Keystrokes such as these and the function keys and others are better left to the system 
   for default handling.  This is why I don't simply throw out all WM_KEYDOWN messages, 
   but rather let some pass through unchanged.

4) For number output, it was necessary to set up a trigger event to signal the output of 
   those numbers when the typiest types a non-character directly after the input of a 
   sequence of numbers.  For example, in typing `1234 and then 'h', the 'h' triggers the 
   output of 1234 which is all handled by IdentifyChar.  But if `1234 is followed by 
   arrow-up key or a function key (2 examples of non-bufferableKeystrokes), IdentifyChar 
   doesn't handle this.  Therefore, I have set up a trigger mechanism to affect this 
   result.

5) For clarification, currently I use 3 clipboard formats to achieve the pasting of Ge'ez 
   into other apps (CF_UNICODETEXT, Rich Text Format, and HTML Format).  However, once a 
   character is pasted into an app like Word 2000 and you then copy that character from 
   Word to the clipboard, it goes from Word to the clipboard in some 18 different formats.  
   When you then attempt to paste into a variety of docs, the receiver app decides which 
   one of those 18 different formats it can use to paste that character into its window.  
   If the receiver can't read any of those formats, it doesn't get pasted.  Likewise with 
   UniGeez -- if the receiver app can't read HTML, RTF, or UnicodeText, the data doesn't 
   get pasted.  However, it is possible that the same receiver app can paste the data 
   from Word 2000, since it has more formats to work with.

6) Most apps will accept the CF_UNICODETEXT by itself. Furthermore, most apps attempt to 
   use the HTML Format if it is present whether or not it works properly in the app.  
   Case and point, Excel'97 doesn't like the HTML but readily accepts CF_UNICODETEXT.  
   The CF_UNICODETEXT is, I believe, a better format for the purposes of pasting Ge'ez, 
   and I don't like the fact that many apps ignore it in favor of HTML. As tested so far, 
   the only app that needs the HTML Format to work properly is MSFrontPage2000. 
   Therefore, I have chosen to determine if the typiest is editing in FrontPage2000, and 
   then and only then, is the HTML Format enabled.  If other apps are later determined to 
   need the HTML Format, then they too can be incorporated.  RTF is being used in Word and 
   WordPad only.  CF_UNICODETEXT allows font size to be changeable inside the editor.
   (See function: parentWindowFinder(HWND hChild, bool& useHTML).)

7) As a partial ammendment to the above (#7), the CF_UNICODETEXT is being used as the 
   default format. However, some app's are not capatible with this format.  To 
   incorporate other apps, a text file has been added that includes an application's 
   classname (obtainable through MS Spy++) and its preferred format, either RTF of HTML.  
   At run time, this info is loaded into shared memory so that the dll can use that data 
   to determine which format it should be using.  If the application with the current 
   focus resides in that format file, then its associated preferred format is used in
   place of CF_UNICODETEXT.  So basically, if an app needs to use a different format other 
   than Unicode, that can be controlled through the formats.txt file prior to execution of 
   UniGeez.

8) When a format is used other than CF_UNICODETEXT, then font size is selectable through 
   the interface window (UniGeez).

9) Sending the shift-key back up as soon as it comes in is necessary because the shift 
   being down prevents the ctrl, as part of a paste sequence, from being recognized and 
   thus the paste was not occurring.

10)  In WinNT4.0 with Microsoft Word, we had the problem of the Roman characters not being 
     thrown out.  Normally, I set the messages that I want thrown out to WM_NULL, but for 
	 some reason this is not working in NT for Word only.  As a quick (and hopefully 
	 temporary) solution, we have chosen to detect if the user is in NT and editing with 
	 Word, and for such an occasion we will backspace to erase the Roman.  If a better 
	 solution is found, please send an email back to the download site stating why this 
	 is happening in NT.

   MPW
*/
