/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

/*! These are basically support functions for the UniGeezLibdll.cpp */

#ifndef FILTERFUNCTIONS_H
#define FILTERFUNCTIONS_H

//! keystrokes that need to trigger output of the numbers or signal the engine's
//! character buffer to be reset
bool triggerEvent(const WPARAM& wParam, const bool& isExcel);

//! returns true for a paste sequence
bool isPasteMsg(const WPARAM& wParam);

//! returns true if thisTime occurred more that 10 time units from the lastTime
//! used to distinguish a group of like messages from other messages
bool compareTimes(const UINT& thisTime, const UINT& lastTime);

//! converts a Keydown message to its ASCII lowercase equivalent
const WPARAM toLowercase(WPARAM wp);

/*!

////////////////////////////////////////////////////////////////////////////////////////
//the success of this function depends entirely upon using the default ASCII code page//
////////////////////////////////////////////////////////////////////////////////////////

  bufferable keystrokes are those which should be run through the engine and thus
  placed in the clipboard to be pasted in the document
  trigger events, function keys, and other non-character strokes do not need to be pasted
  into a doc; they can pass through the Keystroke filter function as normal */

//! converts keydown message to character messages
const WCHAR keydownConvertor(const bool& useLowercase, bool& bufferableKeystroke, 
							 WPARAM wParam, const bool& isExcel);

//! determines whether the window with the current focus is a child of the main app(cwin)
bool parentWindowFinder(HWND hChild, const LPCTSTR cwin);

//! simulates a keystroke going down & back up
void emulateKeyboard(const WPARAM& wp);

//! simulates a mouse event
void emulateMouseEvent(const WPARAM& wp, const LPARAM& lp);

//! returns true if the Keystrokes is one of the arrow keys
bool arrowKey(const WPARAM wp);

//! emualtes the pressing of the shift key in conjunction with an arrow key
void emulateArrowShift (const WPARAM wp);

//! return true if the user is in WinNT and editing with Word
bool isWinNTandWord(HWND hwnd);

//returns true if the input is numeric
bool isANum(const int& var);

//! simulates the shift-key going back up
//! necessary b/c the shift being down interfers with the ctrl in the paste sequence
void shiftUp();

#endif