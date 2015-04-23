/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#ifndef AmharicEngine_H
#define AmharicEngine_H

#include "BasicEngine.h"

//! finite state machine that converts to Amharic

/*! The AmharicEngine contains the language specific information necessary to convert
    from ASCII character codes to Amharic.  It utilizes a finite state machine to 
    make this determination.  It also contains an algorithm for converting Roman numbers 
    to the Amharic number scheme. */

class AmharicEngine: public BasicEngine
{
 public:

	 AmharicEngine() : BasicEngine () { ForceBackspace = false; }
	 
	 //! calls IdentifyChar which will actually handle the conversion
	 const PWCHAR convertASCII(const WCHAR& incoming);

	 //! returns the state of ForceBackspace
	 bool isForceBackspace()			{ return ForceBackspace; }

	 //! sets the ForceBackspace private member
	 void setForceBackspace(bool fB)	{ ForceBackspace = fB; }

 private:

#include "amharic.h"	//! defines Amharic syllables
						//! of for enum Amharic { syllablename = 0xUnicodevalue, ...};

	//! These are states of the current buffer.
    //! Notice they all start with _, to distinguish them from Unicode chars
	enum AmharicStateType 
	{ 
	  //_NOTHING=0, 
	  _BRq=1,   _BRk, _BRg, _BRl, _BRm, _BRr, _BRn, _BRh, _BRS, _BRs, _BR, _BRsW,
	  _h,       _H,   _HW,  _l,   _lW,  _m,   _mW,  _r,   _rW,  _b,   _bW,  _w,   _x,
	  _xW,      _s,   _sW,  _sc,  _v,   _vW,  _q,   _Q,   _qW,  _QW,  _c,   _cW,  _t,
	  _tW,      _n,   _nW,  _pc,  _N,   _NW,  _hW,  _k,   _kW,  _K,   _KW,  _e,   _z,   
	  _zW,      _y,   _yW,  _d,   _dW,  _j,   _ps,  _jW,  _g,   _f,   _fW,  _p,   _pW,
	  _Z,       _ZW,  _D,   _DW,  _gW,  _G,   _GW,  _T,   _TW,  _C,   _CW,  _X,   
	  _P,       _PW,  _S,   _SW,  _mY,  _rY,  _fY };

	//! whether or not we will be backspacing to erase the previous character
	bool ForceBackspace;

	//! private utility functions

	//! THIS IS THE MAIN FUNCTION, setting ToSend based on incoming
	void IdentifyChar(WCHAR incoming);	


	//! Why is this a member?  Because it's selectively converting to
	//! lower case only those chars that may be in the SERA scheme
	//! for Amharic text
	void toLowercase(WCHAR& ch);

	//! converts ASCII numbers to Amharic  & appends to ToSend
	void numberConvertor();

	//! specifically identifies the Geez meto (100) character
    bool isHundred(PWCHAR str);

	//! specifically identifies the Geez 10,000 character
    bool isTenThousand(PWCHAR str);

	//! returns appropiate Amharic one's character
	const WCHAR ones(const WCHAR& num);

	//! returns appropiate Amharic ten's character
	const WCHAR tens(const WCHAR& num);
};

#endif