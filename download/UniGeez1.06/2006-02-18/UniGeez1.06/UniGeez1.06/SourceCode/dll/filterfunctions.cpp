/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#include "windows.h"

//! simulates a keystroke
void emulateKeyboard(const WPARAM& wp)
{
	keybd_event(wp, 0, 0,               0);
	keybd_event(wp, 0, KEYEVENTF_KEYUP, 0);
}

//! emulate the appropiate mouse event
void emulateMouseEvent(const WPARAM& wp, const LPARAM& lp)
{
	DWORD xPos = LOWORD(lp);
	DWORD yPos = HIWORD(lp);

	if(wp == WM_LBUTTONDOWN)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN,  xPos, yPos, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP,    xPos, yPos, 0, 0);
	}
	else if(wp == WM_RBUTTONDOWN)
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, xPos, yPos, 0, 0);
		mouse_event(MOUSEEVENTF_RIGHTUP,   xPos, yPos, 0, 0);
	}
				
}

//! returns true if the input is numeric
bool isANum(const int& var)	{ return (var >= 0x30 && var <= 0x39); }

//! keystrokes that need to trigger output of the numbers or signal the engine's
//! character buffer to be reset
bool triggerEvent(const WPARAM& wParam, const bool& isExcel)
{
	//function keys 
	if(wParam >= 0x70 && wParam  <= 0x7A)
		return true;

	switch (wParam)
	{
	case VK_SPACE:
		if(!isExcel) return true;
		else return false;
		break;
	case VK_TAB:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_RETURN:
		return true;
		break;
	default:
		return false;
	}

}

//! converts a Keydown message to its ASCII lowercase equivalent
const WPARAM toLowercase(WPARAM wp)
{
	return wp + 0x20;
}

////////////////////////////////////////////////////////////////////////////////////////
//! the success of this function depends entirely upon using the default ASCII code page//
////////////////////////////////////////////////////////////////////////////////////////

/*! bufferable keystrokes are those which should be run through the engine and thus
    placed in the clipboard to be pasted in the document
    trigger events, function keys, and other non-character strokes do not need to be pasted
    into a doc; they can pass through the Keystroke filter function as normal */

const WCHAR keydownConvertor(const bool& useLowercase, bool& bufferableKeystroke, 
							 WPARAM wParam, const bool& isExcel)
{
	const int SPACE = 0x20;
	WCHAR ch;

	switch (wParam)
	{
	case VK_SPACE:
		if(isExcel)
		{
			bufferableKeystroke = true;
			return ch = SPACE;
		}
		else
		{
			bufferableKeystroke = false;
			return 0;
		}
	case VK_DIVIDE:
		bufferableKeystroke = true;
		return ch = '/';
	case VK_MULTIPLY:
		bufferableKeystroke = true;
		return ch = '*';
	case VK_SUBTRACT:
		bufferableKeystroke = true;
		return ch = '-';
	case VK_ADD:
		bufferableKeystroke = true;
		return ch = '+';
	case VK_DECIMAL:
		bufferableKeystroke = true;
		return ch = '.';
	}

	//! proceed using all lowercase
	if(useLowercase)
	{	
		//! convert uppercase letters (A-Z) to lowercase
		if(wParam >= 0x40 && wParam <= 0x5A)	
		{ 
			bufferableKeystroke = true;
			return ch = toLowercase(wParam);
		}

		//! convert numpad (0-9) keys to numbers
		if(wParam >= 0x60 && wParam <= 0x69)
		{
			bufferableKeystroke = true;
			return ch = wParam -= 0x30;
		}

		//! see if it's a number (0-9)
		if(wParam >= 0x30 && wParam <= 0x39)
		{
			bufferableKeystroke = true;
			return ch = wParam;
		}

		switch (wParam)
		{
		case 0xC0:							// `
			bufferableKeystroke = true;
			return ch = '`';
		case 0xBC:							// ,
			bufferableKeystroke = true;
			return ch = ',';
		case 0xBA:							// ;
			bufferableKeystroke = true;
			return ch = ';';
		case 0xBD:							// -
			bufferableKeystroke = true;
			return ch = '-';
		case 0xBB:							// =
			bufferableKeystroke = true;
			return ch = '=';
		case 0xDC:							// '\'
			bufferableKeystroke = true;
			return ch = 0x5C;
		case 0xDB:							// [
			bufferableKeystroke = true;
			return ch = '[';
		case 0xDD:							// ]
			bufferableKeystroke = true;
			return ch = ']';
		case 0xDE:							// '
			bufferableKeystroke = true;
			return ch = 0x27;
		case 0xBF:							// '/'
			bufferableKeystroke = true;
			return ch = '/';
		case 0xBE:							// .
			bufferableKeystroke = true;
			return ch = '.';
		default:
			bufferableKeystroke = false;
			return 0;
		}
	}

	//! proceed using all uppercase
	else
	{
		//! keydown strokes for A-Z are already uppercase
		if(wParam >= 0x40 && wParam <= 0x5A)	
		{ 
			bufferableKeystroke = true;
			return ch = wParam;
		}

		//! all these keystrokes need converting to upper
		switch (wParam)
		{	
		case '1':
			bufferableKeystroke = true;
			return ch = '!';
		case '2':
			bufferableKeystroke = true;
			return ch = '@';
		case '3':
			bufferableKeystroke = true;
			return ch = '#';
		case '4':
			bufferableKeystroke = true;
			return ch = '$';
		case '5':
			bufferableKeystroke = true;
			return ch = '%';
		case '6':
			bufferableKeystroke = true;
			return ch = '^';
		case '7':
			bufferableKeystroke = true;
			return ch = '&';
		case '8':
			bufferableKeystroke = true;
			return ch = '*';
		case '9':
			bufferableKeystroke = true;
			return ch = '(';
		case '0':
			bufferableKeystroke = true;
			return ch = ')';
		case 0xC0:
			bufferableKeystroke = true;
			return ch = '~';
		case 0xBA:						
			bufferableKeystroke = true;
			return ch = ':';
		case 0xBF:							
			bufferableKeystroke = true;
			return ch = '?';
		case 0xBD:
			bufferableKeystroke = true;
			return ch = '_';
		case 0xBB:
			bufferableKeystroke = true;
			return ch = '+';
		case 0xDC:
			bufferableKeystroke = true;
			return ch = '|';
		case 0xDB:
			bufferableKeystroke = true;
			return ch = '{';
		case 0xDD:
			bufferableKeystroke = true;
			return ch = '}';
		case 0xDE:
			bufferableKeystroke = true;
			return ch = '"';
		case 0xBC:
			bufferableKeystroke = true;
			return ch = '<';
		case 0xBE:
			bufferableKeystroke = true;
			return ch = '>';
		default:
			bufferableKeystroke = false;	//! anything else should not pass thru
			return 0;
		}
	}
}

//! returns true for a paste sequence
bool isPasteMsg(const WPARAM& wParam)
{
	return (wParam == VK_CONTROL || wParam == 'V');	
}

//! determines whether the window with the current focus 
//! is a child of the main app(cwin)
bool parentWindowFinder(HWND hChild, const LPCTSTR cwin)
{
	//! finds the handle of the specified app; 
	//! returns false if that app is not currently running
	HWND hParent  = FindWindow(cwin, NULL);

	if(!hParent)	{ return false; }
	else
	{
		HWND hLook    = NULL;
	
		while(hChild != hParent && hChild != NULL)
		{
			hLook  = GetParent(hChild);
			hChild = hLook;
		}

		//! the window with the cursor focus has been traced to its parent's window handle
		//! and there is a match
		if(hChild == hParent)		{ return true; }
		else						{ return false;}
	}
}

//! returns true if thisTime occurred more that 10 time units from the lastTime
//! used to distinguish a group of like messages from other messages
bool compareTimes(const UINT& thisTime, const UINT& lastTime)
{
	const int CONTROL_UNIT = 10;
	return ((thisTime - CONTROL_UNIT) > lastTime);
}

//! returns true if the Keystrokes is one of the arrow keys
bool arrowKey(const WPARAM wp)
{
	switch(wp)
	{
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		return true;
		break;
	default:
		return false;
	}
}

//! emualtes the pressing of the shift key in conjunction with an arrow key
void emulateArrowShift(const WPARAM wp)
{
	keybd_event(VK_SHIFT, 0, 0,               0);
	keybd_event(wp,       0, 0,               0);
	keybd_event(wp,       0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
}

//! return true if the user is in WinNT and editing with Word
bool isWinNTandWord(HWND hwnd)
{
	OSVERSIONINFO vi = { sizeof(vi) };
	GetVersionEx(&vi);

	//! platform = WinNT and editor = Word "OpusApp"
	if(vi.dwPlatformId == VER_PLATFORM_WIN32_NT  && vi.dwMajorVersion <= 4
		               && parentWindowFinder(hwnd, "OpusApp"))
		return true;

	else 
		return false;
}

//! simulates the shift-key going back up
//! necessary b/c the shift being down interfers with the ctrl in the paste sequence
void shiftUp()
{
	keybd_event(VK_SHIFT,	0x2A, KEYEVENTF_KEYUP, 0);
}
