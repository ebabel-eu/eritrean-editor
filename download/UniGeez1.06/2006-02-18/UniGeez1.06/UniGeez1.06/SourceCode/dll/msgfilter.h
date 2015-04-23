/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

/*! These are support functions for 
    LRESULT CALLBACK MsgFilterFunc (INT nCode, WPARAM wParam, LPARAM lParam) */

#ifndef MSGFILTER_H
#define MSGFILTER_H

#include "mode.h"
#include "NTandWord.h"

//! set ExcelMode if we are in Excel
void handleExcelMode(AmharicEngine& myEngine, const bool& isExcel, 
					 bool& shiftLeftForExcel);

//! we want to know when the mouse is clicked so that we can use it as a trigger event
void mouseClickForTriggerEvent(const WPARAM& msg, bool& isMouseClick, 
							   AmharicEngine& myEngine);

//! throw out character messages; however, some apps need the char equivalent of
//! ctrl-v(0x16) to enact a paste message and only perform when we are not in ROMAN mode
void handleWM_CHAR(WPARAM& msg, const bool& bufferableKeystroke, const WPARAM& wparam);

//! so that events such as CTRL-A, etc. will work, we need the overrideStateRM
//! to temporarily place us in ROMAN mode, and once this is true and the CTRL
//! key is released, we can return to GEEZ mode
void handleWM_KEYUP(const MSG* msg, bool& overrideStateRM, const WPARAM& timeShiftDown, 
					bool& useLowercase, bool& shiftState, MODE& Mode);

//! switch to Roman mode for keyboard events that involve the CTRL-key
void switchToRM(MODE& Mode, const WPARAM& previousKeydown, const WPARAM& wparam,
				bool& overrideStateRM);

//! handle everything that deals with the shift-key & caps lock
void shiftModes(bool& shiftState, WPARAM& timeShiftDown, bool& useLowercase, 
				bool& capsToggled, const WPARAM& time);

//! handle backspacing in general
void handleBackspacing(AmharicEngine& myEngine, const WPARAM& previousKeydown, 
					   const WPARAM& wparam, bool& allowBackSpace, const bool& ExcelMode, 
					   NTandWord& NTandWord);

//! handles backspacing specifically for Excel
void backspacingForExcel(WCHAR& ch, WPARAM& msg, bool& bufferableKeystroke, 
						 AmharicEngine& myEngine);

//! handle tasks related to the numbers
void handleNumbers(WCHAR& ch, AmharicEngine& myEngine, const WPARAM& wparam, 
				   const bool& isMouseClick, bool& bufferOvrdNum, const bool& shiftState,
				   const bool& bufferableKeystroke, NTandWord& NTandWord);

//! Converting ASCII to Ge'ez
void handleAsciiConversion(const WPARAM& wparam, AmharicEngine& myEngine, 
						   NTandWord& NTandWord, bool& shiftLeftForExcel, 
						   const bool& isMouseClick, Clipboard& clipboard, 
						   const WCHAR& ch, bool& allowBackSpace, 
						   const HWND& hwndUniGeez, const bool& bufferOvrdNum);


#endif