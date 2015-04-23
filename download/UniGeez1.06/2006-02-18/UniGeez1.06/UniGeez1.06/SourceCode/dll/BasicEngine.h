/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#ifndef BASICENGINE_H
#define BASICENGINE_H

//! returns true if the input is a number
inline bool isNumeric(const int& var)	{ return (var >= 0x30 && var <= 0x39); }

//! sets a pointer to NULL
inline void clear(PWCHAR str) { *str = NULL; }

#define _NOTHING 0

//! This class provides the basics for a finite-state machine to transliterate, but
//! does NOT contain any language-specific information. 

class BasicEngine
{

protected:

	typedef int StateType;

	enum { BIG_NUMBER = 524 };
  
	WCHAR Buffer       [BIG_NUMBER];	//! all chars read in so far
	StateType State;					//! Classification of buffer pattern
	WCHAR ToSend       [BIG_NUMBER];	//! What should be printed next
	WCHAR ActualChar;					//! Last thing read in, not yet processed.
	WCHAR numberBuffer [BIG_NUMBER];	//! buffer just for numbers 

	bool numberState;					//! Are we in the midst of a #?
	bool isExcel;						//! are we in Excel?

	//! returns true if we have a State other than Nothing for our state diagram
	bool stateSomething()   { return State != _NOTHING; }

	//! we have a syllable that is ready for output -- add it to ToSend & call reset
	void complete_syllable(const int& syllable)
	{
		reset(); append(ToSend, syllable);		
	}

	//! set the new state and append the last incoming character to Buffer
	void develop_state(const StateType& new_state)
	{
		State = new_state; append(Buffer, ActualChar);
	}

	//! adds a new syllable to the ToSend buffer and places the state diagram
	//! in a new state
	void make_syllable(const StateType& new_state, const int& syllable)
	{
		develop_state(new_state);
		append(ToSend, syllable);
	}

	//! adds a character to the end of a buffer
	void append(PWCHAR str, const WCHAR& ch);



public:

	BasicEngine ()
	{ 
		State = _NOTHING; 
		clear (Buffer); 
		clearToSend();
		numberState = false; 
		isExcel     = false;
	}

	//! returns a pointer to the ToSend buffer
	PWCHAR to_send()			{ return ToSend; }	

	//! Since numbers are handled differently than other characters 
	//! (i.e., they're converted as a group), we need to know if we're in a number state
	bool InANumber () const		{ return numberState; }

	//! we are no longer in a number state
	void resetNumState()		{ numberState = false; }

	//! sets the protected member isExcel to remember whether we are in Excel
	void setExcelMode(bool wh)	{ isExcel = wh; }

	//! While in Excel, we have to always repaste the contents of the entire cell as new 
	//! characters are typed, so remember whether or not we are in Excel
	bool inExcelMode()			{ return isExcel; }

	//! resets character buffer, number buffer, and 2 boolean states
	void  reset()	 
	{ 
		clear(Buffer); clear(numberBuffer); State = _NOTHING; numberState = false;
	}

	//! empties the ToSend buffer
	void clearToSend()	{ memset(ToSend, '\0', sizeof(WCHAR)*BIG_NUMBER); }

	//! removes the last value from the ToSend buffer & places it in the previous position
	void shift_left();

	//! used to effectively backspace in Excel
	void backspace();

	//! returns true if the ToSend buffer is empty
	bool bufferEmpty() const { return ToSend[0] == '\0'; }

};

#endif