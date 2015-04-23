//! Support functions for KeyboardInterceptor.cpp */

#ifndef IO_H
#define IO_H

#include "windows.h"
#include <string>
using namespace std;

const int MAX_CH = 80;

//! populates combobox with font sizes
void insertPitch(const HWND& hCtrl, string pitch[], const int MAX_ENTRIES);

//! reads format info from a file
const int readFormatFile(char appName[][MAX_CH], char format[][MAX_CH], 
						 const int MAX_APPS);

/*! Registery related */

//! determines if a reg key has been created specifically for the info prompt
bool isRegKeyCreated(const string& regKeyName, unsigned char& val);

//! determines if a reg key has been created
bool isRegKeyCreated(const string& regKeyName);

//! create a reg key for the info prompt
void createRegKey(const string& regKeyName, const bool& infoState);

//! create a reg key for a font name
void createRegKey(const string& regKeyName, const string& font);

//! change the info prompt reg key
void updateRegKey(const string& regKeyName, const bool& infoState);

//! populate a combobox with font names from a reg key
void insertFontNames(const HWND& hCtrl, const string& font, string* fName);

//! update the font names in the registry key
void updateFontName(const HWND& hCtrl, const char* buff, string* fName, 
					const string& regKeyName, const int MAX_FONTS);
  
#endif