/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#include "windows.h"
#include "AmharicEngine.h"
#include "mode.h"
#include "filterfunctions.h"
#include "clipboard.h"
#include "ntandword.h"

//! set ExcelMode if we are in Excel
void handleExcelMode(AmharicEngine& myEngine, const bool& isExcel, 
					 bool& shiftLeftForExcel)
{
	//! are we in Excel?
	myEngine.setExcelMode(isExcel);

	//! last iteration we shifted left in the ToSend buffer for Excel so we are
	//! now resetting these variables so that it will not happen again unless needed
	if(shiftLeftForExcel)
	{
		myEngine.setForceBackspace(false);
		shiftLeftForExcel    = false;
	}
}

//! we want to know when the mouse is clicked so that we can use it as a trigger event
void mouseClickForTriggerEvent(const WPARAM& msg, bool& isMouseClick, 
							   AmharicEngine& myEngine)
{
	if(msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)
	{
		isMouseClick = true;

		//reset the buffer for a mouse click when in Excel
		if(myEngine.inExcelMode() && !myEngine.InANumber())
		{
			myEngine.clearToSend();
			myEngine.reset();
		}
	}
}

//! throw out character messages; however, some apps need the char equivalent of
//! ctrl-v(0x16) to enact a paste message and only perform when we are not in ROMAN mode
void handleWM_CHAR(WPARAM& msg, const bool& bufferableKeystroke, const WPARAM& wparam)
{
	if(msg == WM_CHAR)
	{
		if(bufferableKeystroke && wparam != 0x16)		//^V		
		{ 
			msg = WM_NULL; 
		}
	}
}

//! so that events such as CTRL-A, etc. will work, we need the overrideStateRM
//! to temporarily place us in ROMAN mode, and once this is true and the CTRL
//! key is released, we can return to GEEZ mode
void handleWM_KEYUP(const MSG* msg, bool& overrideStateRM, const WPARAM& timeShiftDown, 
					bool& useLowercase, bool& shiftState, MODE& Mode)
{
	if(msg->message == WM_KEYUP && overrideStateRM && msg->wParam == VK_CONTROL)
	{
		Mode            = GEEZ;
		overrideStateRM = false;
	}
		
	//! switch back to using lowercase
	if(msg->message == WM_KEYUP && Mode != ROMAN)
	{
		//! trigger event for switching back to lowercase
		if(msg->wParam == VK_SHIFT && (compareTimes(msg->time, timeShiftDown)))
		{
			useLowercase = true;
			shiftState   = false;
		}
	}
}

//! switch to Roman mode for keyboard events that involve the CTRL-key
void switchToRM(MODE& Mode, const WPARAM& previousKeydown, const WPARAM& wparam,
				bool& overrideStateRM)
{
	if(previousKeydown == VK_CONTROL && wparam != VK_CONTROL 
		&& wparam != VK_MENU && wparam != 'V') 
	{
		Mode = ROMAN;

		//! keep track of the fact that we have purposedly switched modes
		//! so that we can return to GEEZ mode (see KeyUp Messages)
		overrideStateRM = true;
	}
}

//! handle everything that deals with the shift-key & caps lock
void shiftModes(bool& shiftState, WPARAM& timeShiftDown, bool& useLowercase, 
				bool& capsToggled, const WPARAM& time)
{
	//! determine whether to use lowercase
	//! if the high bit (<0) is set then the key is down
	if(GetKeyState(VK_SHIFT) < 0)						
	{ 
		//! the shift key is down!
		shiftState = true;
		timeShiftDown = time;
		useLowercase = false;

		//! in order for the paste to occurr we must immediately send the
		//! shift key back up
		shiftUp();  
	}

	//! determine whether the CAPS is toggled (ON)
	//! toggled if the low bit is set
	int iState = GetKeyState(VK_CAPITAL);
	if(iState & 1)		
	{ 
		useLowercase = false; 
		capsToggled = true;
	}

	//! CAPS-LOCK is off!!
	else 
	{	
		//! if the CAPS was toggled last time, but not this time
		//! reset the variables for tracking shift states
		if(!shiftState)
		{
			useLowercase = true; 
			capsToggled = false;
		}
	}

	//! CAPS on + shift yields lowercase
	if(capsToggled && shiftState)
	{
		useLowercase = true;
	}
}

//! handle backspacing in general
void handleBackspacing(AmharicEngine& myEngine, const WPARAM& previousKeydown, 
					   const WPARAM& wparam, bool& allowBackSpace, const bool& ExcelMode,
					   NTandWord& NTandWord)
{

	//! when we create the backspace for erasing the 6th form character, 
	//! we don't want that event thrown out

	//! the backspace that we created has passed through, so we no longer
	//! need to track it
	if(previousKeydown == VK_BACK && wparam != VK_BACK)
		allowBackSpace = false;

	//! reset buffer for all non-character events
	//! do not perform if we are in a number state
	if(triggerEvent(wparam, ExcelMode) && !myEngine.InANumber())
		myEngine.reset(); 
			
	//! reset buffer for bksp except when the backspace is created
	//! for erasing the 6th form
	if(!NTandWord.state && wparam == VK_BACK && !myEngine.isForceBackspace() 
		&& !allowBackSpace && !ExcelMode)
	{ 
		myEngine.reset();
	}

	/************ For WinNT & Word only! *****************************/

	//! we're getting the backspace that we created for erasing characters
	//! so reset the flag for tracking it and do not reset the buffer!
	if(NTandWord.state && wparam == VK_BACK && NTandWord.bkspForNT_Char)
	{
		NTandWord.bkspForNT_Char = false;
	}

	//! only reset the buffer if this is not the backspace that we created
	//! for erasing numbers or a 6th form character
	else if(NTandWord.state && wparam == VK_BACK && !NTandWord.bkspForNT_Num 
		                                         && !allowBackSpace)
	{
		myEngine.reset();
	}
	/*****************************************************************/
			
	//! we are now receiving the backspace message that we created, so we
	//! no longer need to force the backspace
	if(wparam == VK_BACK && myEngine.isForceBackspace())	
	{ 
		myEngine.setForceBackspace(false);
	}
}

//! handles backspacing specifically for Excel
void backspacingForExcel(WCHAR& ch, WPARAM& msg, bool& bufferableKeystroke, 
						 AmharicEngine& myEngine)
{
	//! make it pass through the engine
	ch = 0;
	bufferableKeystroke = true;

	myEngine.backspace();			//! backspace in the buffer
	msg = WM_NULL;					//! throwout backspace messages

	//! simulate a DELETE if  the ToSend buffer is empty
	if(myEngine.bufferEmpty())
		emulateKeyboard(VK_DELETE);
}

//! handle tasks related to the numbers
void handleNumbers(WCHAR& ch, AmharicEngine& myEngine, const WPARAM& wparam, 
				   const bool& isMouseClick, bool& bufferOvrdNum, const bool& shiftState,
				   const bool& bufferableKeystroke, NTandWord& NTandWord)
{
	/************** Mouse Clicks *******************/
	//! output the buffer & reset it for a mouse click
	if(isMouseClick)
	{
		//! this will handle the numbers
		ch = 0;

		//! this the characters
		if(!myEngine.InANumber())
		{
			myEngine.reset();
			myEngine.setForceBackspace(false);
		}
	}
	/***********************************************/

	//! for a sequence of numbers followed by a keystroke that is not 
	//! defined as bufferable, create signal for output
	if(wparam != VK_SHIFT)
	{
		if(myEngine.InANumber() && !isNumeric(ch) && !bufferableKeystroke)
		{
			ch = 0;	//! won't paste but will trigger numbers to be outputted

			//! normally this message would by-pass the engine, but we don't
			//! want that to happen
			bufferOvrdNum = true;
		}
	}

	//! this series of comparisons is needed if the shift key is pressed
	else if(myEngine.InANumber() && !isNumeric(ch) && !bufferableKeystroke && shiftState)
	{
		ch = 0;	//! won't paste but will trigger numbers to be outputted

		//! normally this message would by-pass the engine, but we don't
		//! want that to happen
		bufferOvrdNum = true;
	}

	/************ For WinNT & Word only! *****************************/

	//! we're not in a number state so stop tracking the backspace
	if(NTandWord.state && !isANum(ch))
		NTandWord.bkspForNT_Num = false;

	//! we're in the number state and this is the backspace that we created
	//! so don't allow it to reset the buffer
	if(NTandWord.state && myEngine.InANumber() && wparam == VK_BACK)
		bufferOvrdNum = false;

	/*****************************************************************/
}

//! Converting ASCII to Ge'ez
void handleAsciiConversion(const WPARAM& wparam, AmharicEngine& myEngine, 
						   NTandWord& NTandWord, bool& shiftLeftForExcel, 
						   const bool& isMouseClick, Clipboard& clipboard, 
						   const WCHAR& ch, bool& allowBackSpace, 
						   const HWND& hwndUniGeez, const bool& bufferOvrdNum)
{
	//! buffers characters and converts to Ge'ez
	PWCHAR pchar = myEngine.convertASCII(ch);


	/************ For WinNT & Word only! *************************/

	//! used to track the number state
	if(NTandWord.state && myEngine.InANumber())
	{
		//! erase the number typed with a backspace
		emulateKeyboard(VK_BACK);

		//! we don't want this backspace to reset the buffer, so track it
		NTandWord.bkspForNT_Num = true;
	}

	//! this will handle erasing the characters
	if(NTandWord.state && !myEngine.InANumber())
	{ 
		//! erase the character typed with a backspace
		emulateKeyboard(VK_BACK);

		//! we don't want this backspace to reset the buffer 
		//! so we'll track it
		NTandWord.bkspForNT_Char = true;
	}

	/*************************************************************/

	//! erase the sixth form when appropiate
	if(myEngine.isForceBackspace())	
	{ 
		//! only create backspace for non-Excel apps
		if(!myEngine.inExcelMode())
		{
			//! create a backspace
			emulateKeyboard(VK_BACK);

			//! we want this backspace to occur and not get thrown out
			//! SEE: Backspacing and Resetting the buffer for when this is reset
			allowBackSpace = true;
		}
		//! in Excel mode
		else
			shiftLeftForExcel = true;
	}
	
	//! when the engine has returned data for output
	if(*pchar != NULL)
	{	
		//! place the data on the clipboard
		clipboard.copyToClipboard(hwndUniGeez, pchar);

		//! create a paste message
		clipboard.paste();

		//! causes incoming msg to occur after numbers have been outputted
		//! so as not to interfere with the paste
		if(bufferOvrdNum)		
		{ 
			//! recreate that message, other than a mouse message
			if(!isMouseClick)
			{
				emulateKeyboard(wparam);
			}
		}						
	}
}
