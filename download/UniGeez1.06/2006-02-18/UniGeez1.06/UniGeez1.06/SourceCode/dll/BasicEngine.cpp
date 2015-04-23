/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#include "windows.h"
#include "BasicEngine.h"
#include <string>

//! removes the last value from the ToSend buffer & places it in the previous position
void BasicEngine::shift_left()
{
	int len = 0;

	//determine length
	while(ToSend[len]) ++len;

	ToSend[len -2] = ToSend[len -1];
	ToSend[len -1] = '\0';
}

//! removes the last character from the ToSend buffer
//! used to effectively backspace in Excel
void BasicEngine::backspace()
{
	int len = 0;

	//determine the length
	while(ToSend[len]) ++len;

	if(len == 1) reset();

	ToSend[len -1] = '\0';
}

//! adds a character to the end of a buffer
void BasicEngine::append(PWCHAR str, const WCHAR& ch)
{
  //skip to end ... put ch where '\0' was ... append '\0'
  while (*str) ++str;   
  *str++ = ch;              
  *str = '\0';
}
