/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#include "windows.h"
#include "clipboard.h"
#include <fstream>
#include <string>
using namespace std;
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>

Clipboard::Clipboard()
{
	hHtml    = NULL;
	hRtf     = NULL;
	hUnicode = NULL;
	Unicode  = false;
	Rtf      = false;
	Html     = false;
	fs       = 0;	
}

Clipboard::~Clipboard()
{
	freeGlobalMem();
}

//! set the formats and fontsize
void Clipboard::setFormats(bool _Unicode, bool _Rtf, bool _Html, int fontsize)
{
	Unicode = _Unicode;
	Rtf     = _Rtf;
	Html    = _Html;
	fs      = fontsize;
}

//! frees the Global memory allocated when data was placed on the clipboard
void Clipboard::freeGlobalMem()
{
	if(hHtml    != NULL)		{ GlobalFree(hHtml);    }
	if(hRtf     != NULL)		{ GlobalFree(hRtf);     }
	if(hUnicode != NULL)		{ GlobalFree(hUnicode); }
}

//! copies the specified format to the clipboard
void Clipboard::copyToClipboard(const HWND& hwndOwner, PWCHAR pchar)
{
	//register HTML & RTF formats
	UINT cf_html = RegisterClipboardFormat("HTML Format");
	UINT cf_rtf  = RegisterClipboardFormat("Rich Text Format");

	TCHAR text[LOTSOFTEXT];
	PWCHAR remember_pchar = pchar;
	char* ptr = NULL;

	/* Place the appropiate format on the clipboard */

	//convert to UNICODE
	if(Unicode)
	{
		UINT size_of_text = makeUnicode(pchar, text);
		PTCHAR ptr_text = text;

		hUnicode = GlobalAlloc(GHND | GMEM_SHARE, size_of_text + 4);
	
		ptr = (char*)GlobalLock(hUnicode);
		my_strcpy(ptr, ptr_text, size_of_text);
		GlobalUnlock(hUnicode);

		OpenClipboard(hwndOwner);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hUnicode);
		CloseClipboard();
	}

	//convert to RTF
	if(Rtf)
	{
		pchar = remember_pchar;
		makeRtf(pchar, text, fs);

		hRtf = GlobalAlloc(GHND | GMEM_SHARE, (lstrlen (text) + 1) * sizeof (TCHAR));
	
		ptr = (char*)GlobalLock(hRtf);
		lstrcpy(ptr, text);
		GlobalUnlock(hRtf);

		OpenClipboard(hwndOwner);
		EmptyClipboard();
		SetClipboardData(cf_rtf, hRtf);
		CloseClipboard();
	}
	
	//convert to HTML
	if(Html)
	{
		pchar = remember_pchar;
		makeHtml(pchar, text, fs);

		hHtml = GlobalAlloc(GHND | GMEM_SHARE, strlen(text)+4);

		ptr = (char*)GlobalLock(hHtml);
		strcpy(ptr, text);
		GlobalUnlock(hHtml);

		OpenClipboard(hwndOwner);
		EmptyClipboard();
		SetClipboardData(cf_html, hHtml);
		CloseClipboard();
	}
	
	//free global memory allocated by CopyToClipboard
	freeGlobalMem();
}

//! simulates a paste sequence
void Clipboard::paste()
{	
	//CTRL-V sequence
	keybd_event(VK_CONTROL, 0x1D, 0,               0);
	keybd_event('V',        0x2F, 0,               0);
	keybd_event('V',        0x2F, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);
}

//! simulates the shift-key going back up
//! necessary b/c the shift being down interfers with the ctrl in the paste sequence
void Clipboard::shiftUp()
{
	keybd_event(VK_SHIFT,	0x2A, KEYEVENTF_KEYUP, 0);
}

////////////////////////////////////////////////////////////////////////////////////////
//utility functions

//! creates a Rich Text Format to place in the clipboard
void Clipboard::makeRtf(PWCHAR pchar, PTCHAR text, const int fs)
{
	TCHAR addText  [MAX_SIZE];
	TCHAR startText[MAX_SIZE]; 

	//retrieve the current font name
	string fName;
	getFontName(fName);
	
	//the start of the RTF format
	sprintf(startText, 
	"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\fnil\\fprq2"
	"\\fcharset0 %s;}{\\f1\\fnil\\fcharset0 Times New Roman;}}"
	"\\viewkind4\\uc1\\pard\\f0\\fs%d", fName.c_str(), fs*2);

	text[0] = '\0';				//reset text to ""
	lstrcat (text, startText);

	//each Unicode character that needs to be added
	while(*pchar != NULL)
	{	
		WCHAR charNum = *pchar++;
		sprintf(addText, "\\u%d?", charNum);
		lstrcat (text, addText);
	}

	//the end of the format
	PTCHAR ending = "\\f1\\fs20}";
	lstrcat (text, ending);

	assert (lstrlen(text) < LOTSOFTEXT); 
}

/*! When placing the HTML format on the clipboard, you have to know 4 byte counts:
    1) from position 0 to the end of the fragment (the byte prior to the one in #2)
    2) from the '<' in <html> to the '>' in <!--StartFragment-->
    3) from the byte after the previous byte in #2 to the byte prior to the byte in #4
    4) from the '<' in <!--EndFragment--> to the '>' in </html> */

//! creates an HTML fragment to place in the clipboard
void Clipboard::makeHtml(PWCHAR pchar, PTCHAR text, const int fs)
{
	const int MULT     = 6;			//for each Unicode character to add in
	const int END_HTML = 217;
	const int END_FRAG = 191;
	TCHAR   addText[MAX_SIZE];
	TCHAR startText[MAX_SIZE];
	PWCHAR rem_pchar = pchar;
	int pcharLength  = 0;

	//retrieve the current font name
	string fName;
	getFontName(fName);

	//determine length of pchar
	while(*pchar++ != 0)		{ ++pcharLength; }

	//remember where it starts
	pchar = rem_pchar;

	//adjust the length to reflect 6-bytes for each character
	pcharLength  = (pcharLength * MULT); 

	//determine the end of the HTML
	int end_html = pcharLength + END_HTML;

	//determine the end of the fragment
	int end_frag = pcharLength + END_FRAG;

	//this is the start of the HTML format
	sprintf(startText, 
	"Version:1.0\nStartHTML:70\nEndHTML:%d\nStartFragment:97\nEndFragment:%d\n<html>\n"
	"<!--StartFragment--><span style='font-size:%dpt;font-family:\"%s\";\n"
	"mso-bidi-language:AR-SA'>", end_html, end_frag, fs, fName.c_str());

	text[0] = '\0';				//reset text to ""
	lstrcat (text, startText);

	//add in all the available Unicode characters
	while(*pchar != NULL)
	{	
		WCHAR charNum = *pchar++;
		sprintf(addText, "&#%d", charNum);
		lstrcat (text, addText);
	}

	//now add the end of the format
	PTCHAR ending = "</span><!--EndFragment-->\n</html>";
	lstrcat (text, ending);

	assert (lstrlen(text) < LOTSOFTEXT); 
}

//! converts the keyboard buffer in the Engine class to Unicode
/*! when placing Unicode on the clipboard it has to be lowbyte followed by highbyte
    -- both bytes being 0 signifies the end */

const UINT Clipboard::makeUnicode(PWCHAR pchar, const PTCHAR text)
{
	UINT i = 0;
	text[0] = '\0';				//reset text to ""

	while(*pchar != NULL)
	{	
		WCHAR lobyte = LOBYTE(*pchar);
		WCHAR hibyte = HIBYTE(*pchar++);
		text[i++] = (char)lobyte; 
		text[i++] = (char)hibyte;
	}
	text[i] = '\0';

	return i;
}

//! used for the Unicode format in case the lowbyte is Zero
void Clipboard::my_strcpy(char* ptr, PTCHAR text, UINT size)
{
	while(size-- != 0)
	{
		*ptr++ = *text++;
	}
}

//! retrieve the current font name from the registery
void Clipboard::getFontName(string& fName)
{
	const int NIL = -1;
	const int MAX = 64;
	char fontName[MAX];
	unsigned long nameLen = MAX;
	DWORD dwType = REG_BINARY;
	HKEY hkey;
	string font("Software\\UniGeez\\Font");
	unsigned char tf = NIL;
	unsigned long len = 1;
	int i = 0;


	//attempt to open the UniGeez registery key
    LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, font.c_str(), 0, KEY_QUERY_VALUE, &hkey);

	//if successful, read the font name string
	if(l == ERROR_SUCCESS)
	{
		//the default font will have a 0 stored next to it; so look for the font name
		//with a 0 next to it and use it as the default font
		while(tf != 0)
		{
			l = RegEnumValue(hkey, i, fontName, &nameLen, NULL, &dwType, &tf, &len);
			++i;
		}
	}

	//if reading from the registery fails for some reason, use GF Zemen Unicode
	else { fName = "GF Zemen Unicode"; }
		
	//close the key
	RegCloseKey(hkey);

	//set the default font
	fName = fontName;
}
	
