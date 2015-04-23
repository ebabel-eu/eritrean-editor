#include <fstream>
#include <string>
using namespace std;
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "io.h"

//! populate combobox with font sizes
void insertPitch(const HWND& hCtrl, string pitch[], const int MAX_ENTRIES)
{
	for(int i = 0; i < MAX_ENTRIES; i++)
		SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)pitch[i].c_str());
}

//! populate combobox with font names from the registery
void insertFontNames(const HWND& hCtrl, const string& font, string* fName)
{
	const int MAX = 64;
	char valName[MAX];
	unsigned long nameLen = MAX;
	HKEY hkey;

	//open the reg key
    LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, font.c_str(), 0, KEY_QUERY_VALUE, &hkey);

	if(l == ERROR_SUCCESS)
	{
		l = NO_ERROR;

		//get each font name that is in the registry 
        for(int nIndex = 0; l != ERROR_NO_MORE_ITEMS; nIndex++)
		{

			nameLen = MAX;
			l = RegEnumValue(hkey, nIndex, valName, &nameLen, 0, 0, 0, 0);
			if (l == ERROR_NO_MORE_ITEMS) continue;

			string name(valName);
			fName[nIndex] = name;

			//set the name within the combobox
			SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)name.c_str());
		}
	}

	//close the reg key
	RegCloseKey(hkey);
}

//! read app name & its associated format from a file
const int readFormatFile(char appName[][MAX_CH], char format[][MAX_CH], 
						 const int MAX_APPS)
{
	int index = 0;

	//check for problems opening file
	struct _stat buf;
    assert((_stat("formats.txt", &buf )) == 0);

	ifstream inStream;
	inStream.open("formats.txt");

	//read in data from file to arrays until EOF is encountered
	while(!inStream.eof())
	{
		inStream >> appName[index] >> format[index];
		++index;

		assert (index < MAX_APPS); 
	}

	//close the file
	inStream.close();

	return index;
}

//! return false if the registery key does not exist
bool isRegKeyCreated(const string& regKeyName, unsigned char& val)
{
	HKEY hkey;

	//open the reg key
    LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, regKeyName.c_str(), 0, 
		                  KEY_QUERY_VALUE, &hkey);

	//the key has not been created
	if(l != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return false;
	}

	//extract prompt info from key
	else
	{
		const int MAX = 16;
		char valName[MAX];
		unsigned long nameLen = MAX;
		DWORD dwType = 0;
		unsigned long valSize = 1;

		//get either a 0 or 1 from this reg key
		l = RegEnumValue(hkey, 0, valName, &nameLen, 0, &dwType, &val, &valSize);

		if(dwType != REG_BINARY) return false;

		//close the reg key
		RegCloseKey(hkey);
	}

	return true;
}

//! return false if the registery key does not exist
bool isRegKeyCreated(const string& regKeyName)
{
	HKEY hkey;
    LONG l = RegOpenKeyEx(HKEY_CURRENT_USER, regKeyName.c_str(), 0, 
		                  KEY_QUERY_VALUE, &hkey);
	RegCloseKey(hkey);
	return (l == ERROR_SUCCESS);
}

//! creates a reg key for the info prompt; only storing a 0 or a 1
void createRegKey(const string& regKeyName, const bool& infoState)
{
	HKEY hkey;
	LONG l  = 0;
	unsigned char tf = 0;
	if(infoState) tf = 1;
	DWORD dispo = 0;

	//create the reg key
	l = RegCreateKeyEx(HKEY_CURRENT_USER, regKeyName.c_str(), 0, "", 
					   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &dispo);

	if(l != ERROR_SUCCESS)
		MessageBox(0, "Prompt Info", "Failed to create registery key.",  
			       MB_ICONWARNING);

	if(l == ERROR_SUCCESS)
	{
		l = RegSetValueEx(hkey, TEXT("promptInfo"), 0, REG_BINARY, &tf, 1);

		if(l != ERROR_SUCCESS)
			MessageBox(0, "Prompt Info", "Failed to set registery key value.",  
			           MB_ICONWARNING);
	}
	
	//close the key
	RegCloseKey(hkey);
}

//! create a reg key for a string
void createRegKey(const string& regKeyName, const string& font)
{
	HKEY hkey;
	LONG l  = 0;
	unsigned char tf = 0;
	DWORD dispo = 0;

	l = RegCreateKeyEx(HKEY_CURRENT_USER, regKeyName.c_str(), 0, "", 
					   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &dispo);

	if(l != ERROR_SUCCESS)
		MessageBox(0, "Font Name", "Failed to create registery key.",  
			       MB_ICONWARNING);

	if(l == ERROR_SUCCESS)
	{
		l = RegSetValueEx(hkey, font.c_str(), 0, REG_BINARY, &tf, 1);

		if(l != ERROR_SUCCESS)
			MessageBox(0, "Font Name", "Failed to set registery key value.", 
			           MB_ICONWARNING);
	}
		
	RegCloseKey(hkey);
}

//! update a reg key for the info prompt
void updateRegKey(const string& regKeyName, const bool& infoState)
{
	//delete the registry key
	LONG l = RegDeleteKey(HKEY_CURRENT_USER, regKeyName.c_str());

	//create a replacement one
	createRegKey(regKeyName.c_str(), infoState);
}

//! update the font names in the registry key
void updateFontName(const HWND& hCtrl, const char* buff, string* fName, 
					const string& regKeyName, const int MAX_FONTS)
{
	bool duplicate = false;
	string currFontName(buff);
	string temp(fName[0]);
	HKEY hkey;
	unsigned char tf = 0;
	DWORD dispo = 0;

	//get number of items in the cb
	int items = SendMessage(hCtrl, CB_GETCOUNT, 0, 0);

	//check to see if we already have the current selection
	for(int index = 0; index < items; index++)
	{
		if(fName[index] == currFontName) 
		{
			duplicate = true;
			break;
		}
	}

	//place the current font at index 0 so that it can be the default
	fName[0] = currFontName;

	if(duplicate)
		fName[index] = temp;
	else
	{
		fName[items] = temp;
		++items;
	}

	//delete the registry key
	LONG l = RegDeleteKey(HKEY_CURRENT_USER, regKeyName.c_str());

	//create a new reg key
	l = RegCreateKeyEx(HKEY_CURRENT_USER, regKeyName.c_str(), 0, "", 
					   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &dispo);

	if(l != ERROR_SUCCESS)
		MessageBox(0, "Font Names", "Failed to create registery key.",  
			       MB_ICONWARNING);

	if(l == ERROR_SUCCESS)
	{
		/* copy the 1st 10 font names to the registery */

		if(items > MAX_FONTS) items = MAX_FONTS;

		for(index = 0; index < items; index++)
		{
			tf = (unsigned char)index;
			l = RegSetValueEx(hkey, fName[index].c_str(), 0, REG_BINARY, &tf, 1);

			if(l != ERROR_SUCCESS)
			{
				MessageBox(0, "Font Names", "Failed to set registery key value.", 
				           MB_ICONWARNING);
				break;
			}
		}
	}
		
	RegCloseKey(hkey);
}