/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <string>
using namespace std;

//! This Clipboard class handles placing Unicode data on the clipboard in RTF, Unicode, 
//! HTML formats. 

class Clipboard
{
public:

	Clipboard();
	virtual ~Clipboard();

	//! frees up the Global memory allocated for use of the clipboard
	void freeGlobalMem();

	//! emulates the shift-key going up
	void shiftUp();

	//! emulates a CTRL-V sequence to enact a Paste
	void paste();

	//! copies the appropiate format to the clipboard
	void copyToClipboard(const HWND& hwndOwner, PWCHAR pchar);

	//! sets information about which format to use & the appropiate font size
	void setFormats(bool _Unicode, bool _Rtf, bool _Html, int fontsize);

private:

	enum {LOTSOFTEXT = 524};
	enum {MAX_SIZE   = 524};

	//! handles to global memory for each of the 3 formats
	HGLOBAL hHtml;
	HGLOBAL hRtf;
	HGLOBAL hUnicode;

	//! which clipboard format to use
	bool Unicode, Rtf, Html;

	//! Font size; this information comes from the user interface window
	int fs;						

	//! utility functions

	//! copies from one buffer to the other base on size, NOT null-termination
	void  my_strcpy(char* ptr, PTCHAR text, UINT size);

	//! formulates HTML format for the clipboard
	void  makeHtml(PWCHAR pchar, PTCHAR text, const int fs);

	//! formulates RTF format for the clipboard
	void  makeRtf(PWCHAR pchar, PTCHAR text, const int fs);

	//! formulates Unicode format for the clipboard
	const UINT makeUnicode(PWCHAR pchar, PTCHAR text);

	//! retrieves the current default font name from the registery
	void getFontName(string& fName);

};

#endif