/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#include "windows.h"
#include "AmharicEngine.h"
#include <assert.h>

//! the typed character will now be stored and usually immediately outputted
void AmharicEngine::IdentifyChar(WCHAR incoming)
{
  bool unknownState = false;

  //to convert appropiate equivalences according to the SERA definitions
  toLowercase(incoming);
  
  //clear the buffer that stores the characters to output
  if(!isExcel)  clear (ToSend);

  ActualChar = incoming;	//remember what was entered...

  //variables to track numbers
  static bool lastIncoming_BR = false;

  //numbers must be preceded by ` in order for them to be treated as numbers
  if(lastIncoming_BR && isNumeric(incoming)) numberState = true; 

  //reset BR if next character is not a number
  if(!isNumeric(incoming))						{ lastIncoming_BR = false; }

  //buffer incoming numbers while in the numberState
  if(numberState && isNumeric(incoming))
  {
  	 append(numberBuffer, incoming);
  }


  //all numbers have been entered so convert them
  if(numberState && !isNumeric(incoming))		
  {
	  //2 special cases: 10,000 & 100
	  if(isHundred(numberBuffer))			{ append(ToSend, HUNDRED);     }
	  else if(isTenThousand(numberBuffer))	{ append(ToSend, TEN_THOUSAND);}
	  else numberConvertor();

	  reset();
	  lastIncoming_BR	 = false;
  }
  
  //do not perform this routine for numbers!
  while(!numberState)
  {
	//if data is in the buffer and the incoming char is an apostrophe
	//then do not paste the apostrophe & reset the buffer
	if(stateSomething() && incoming == APOSTROPHE)
	{
		reset();
		incoming = 0;
	}

	//performed on the current state of the buffer
	switch(State)
	{
	case _NOTHING:
		switch(incoming)
		{
		case 'a': complete_syllable (a);				return;
		case 'b': make_syllable		(_b, b);			return;
		case 'C': make_syllable		(_C, C);			return;
		case 'c': make_syllable		(_c, c);			return;
		case 'D': make_syllable		(_D, D);			return;
		case 'd': make_syllable		(_d, d);			return;
		case 'E': complete_syllable (E);				return;
		case 'e': make_syllable		(_e, e);			return;
		case 'f': make_syllable		(_f, f);			return;
		case 'G': make_syllable		(_G, G);			return;
		case 'g': make_syllable		(_g, g);			return;
		case 'H': make_syllable		(_H, H);			return;
		case 'h': make_syllable		(_h, h);			return;
		case 'I': complete_syllable (I);				return;
		case 'i': complete_syllable (i);				return;
		case 'j': make_syllable		(_j, j);			return;
		case 'k': make_syllable		(_k, k);			return;
		case 'K': make_syllable		(_K, K);			return;
		case 'l': make_syllable		(_l, l);			return;
		case 'm': make_syllable		(_m, m);			return;
		case 'N': make_syllable		(_N, N);			return;
		case 'o': complete_syllable (o);				return;
		case 'n': make_syllable		(_n, n);			return;
		case 'P': make_syllable		(_P, P);			return;
		case 'p': make_syllable		(_p, p);			return;
		case 'Q': make_syllable		(_Q, Q);			return;
		case 'q': make_syllable		(_q, q);			return;
		case 'r': make_syllable		(_r, r);			return;
		case 'S': make_syllable		(_S, S);			return;
		case 's': make_syllable		(_s, s);			return;
		case 'T': make_syllable		(_T, T);			return;
		case 't': make_syllable		(_t, t);			return;

		case '`': develop_state     (_BR);
				  lastIncoming_BR = true;				return;
	
		case 'u': complete_syllable (u);				return;
		case 'v': make_syllable		(_v, v);			return;
		case 'W': reset();								return;
		case 'w': make_syllable		(_w, w);			return;
		case 'x': make_syllable		(_x, x);			return;
		case 'X': make_syllable		(_X, X);			return;
		case 'y': make_syllable		(_y, y);			return;
		case 'z': make_syllable		(_z, z);			return;
		case 'Z': make_syllable		(_Z, Z);			return;
		case ';': complete_syllable (SEMICOLON);		return;
		case ':': make_syllable		(_sc, WORDSPACE);	return;
		case ',': complete_syllable (COMMA);			return;
		case '-': make_syllable		(_pc, COLON);		return;

		default:  append (ToSend, incoming);			return;
		}
		break;

	case _e:
		switch (incoming)
		{
		case 'a': complete_syllable (ea);				ForceBackspace = true;	return;
		case '2': complete_syllable (b_e);				ForceBackspace = true;	return;
		default:  reset();
		}
		break;

	case _pc:
		switch (incoming)
		{
		case ':': reset();				return;
		default:  reset();				
		}
		break;

	case _sc:
		switch (incoming)
		{
		case ':': complete_syllable (FULL_STOP);				
			      ForceBackspace = true;	return;
		case '|': make_syllable		(_ps, PARAGRAPH_SEPARATOR);	
				  ForceBackspace = true;	return;
		case '-': complete_syllable (PREFACE_COLON);			
			      ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _ps:
		switch (incoming)
		{
		case ':': reset();	return;
		default:  reset();			
		}
		break;


	case _h:
		switch (incoming)
		{
		case 'a': complete_syllable (ha);		ForceBackspace = true;	return;
		case 'e': complete_syllable (he);		ForceBackspace = true;	return;
		case 'E': complete_syllable (hE);		ForceBackspace = true;	return;
		case 'i': complete_syllable (hi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (ho);		ForceBackspace = true;	return;
		case 'u': complete_syllable (hu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_hW, hW);	ForceBackspace = true;	return;
		case '2': complete_syllable (b_h);		ForceBackspace = true;	return;
		default:  reset();
		}
		break;

	case _H:
		switch (incoming)
		{
		case 'a': complete_syllable (Ha);		ForceBackspace = true;	return;
		case 'e': complete_syllable (He);		ForceBackspace = true;	return;
		case 'E': complete_syllable (HE);		ForceBackspace = true;	return;
		case 'i': complete_syllable (Hi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (Ho);		ForceBackspace = true;	return;
		case 'u': complete_syllable (Hu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_HW, HWa);	ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _HW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);
		}
		break;

	case _hW:
		switch (incoming)
		{
		case 'a': reset();		return;
		case 'e': complete_syllable (hWe);		ForceBackspace = true;	return;
		case 'E': complete_syllable (hWE);		ForceBackspace = true;	return;
		case 'i': complete_syllable (hWi);		ForceBackspace = true;	return;
		case 'u': complete_syllable (hW);		ForceBackspace = true;	return;
		default:  reset();  		

		}
		break;

	case _l:
		switch (incoming)
		{
		case 'a': complete_syllable (la);		ForceBackspace = true;	return;
		case 'E': complete_syllable (lE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (le);		ForceBackspace = true;	return;
		case 'i': complete_syllable (li);		ForceBackspace = true;	return;
		case 'o': complete_syllable (lo);		ForceBackspace = true;	return;
		case 'u': complete_syllable (lu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_lW, lWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _lW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _m:
		switch (incoming)
		{
		case 'a': complete_syllable (ma);		ForceBackspace = true;	return;
		case 'E': complete_syllable (mE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (me);		ForceBackspace = true;	return;
		case 'i': complete_syllable (mi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (mo);		ForceBackspace = true;	return;
		case 'u': complete_syllable (mu);		ForceBackspace = true;	return;
		case 'Y': make_syllable     (_mY, mYa);	ForceBackspace = true;	return;
		case 'W': make_syllable     (_mW, mWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _mW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		case 'e': complete_syllable  (mWe);		ForceBackspace = true;	return;
		case 'i': complete_syllable  (mWi);		ForceBackspace = true;	return;
		case 'E': complete_syllable  (mWE);		ForceBackspace = true;	return;
		case 'u': complete_syllable  (mWu);		ForceBackspace = true;	return;
		default:  complete_syllable  (0);						
		}
		break;

	case _r:
		switch (incoming)
		{
		case 'a': complete_syllable (ra);		ForceBackspace = true;	return;
		case 'E': complete_syllable (rE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (re);		ForceBackspace = true;	return;
		case 'i': complete_syllable (ri);		ForceBackspace = true;	return;
		case 'o': complete_syllable (ro);		ForceBackspace = true;	return;
		case 'u': complete_syllable (ru);		ForceBackspace = true;	return;
		case 'Y': make_syllable     (_rY, rYa);	ForceBackspace = true;	return;
		case 'W': make_syllable     (_rW, rWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _rW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _b:
		switch (incoming)
		{
		case 'a': complete_syllable (ba);		ForceBackspace = true;	return;
		case 'E': complete_syllable (bE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (be);		ForceBackspace = true;	return;
		case 'i': complete_syllable (bi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (bo);		ForceBackspace = true;	return;
		case 'u': complete_syllable (bu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_bW, bWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _bW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		case 'e': complete_syllable  (bWe);		ForceBackspace = true;	return;
		case 'i': complete_syllable  (bWi);		ForceBackspace = true;	return;
		case 'E': complete_syllable  (bWE);		ForceBackspace = true;	return;
		case 'u': complete_syllable  (bWu);		ForceBackspace = true;	return;
		default:  complete_syllable  (0);	
		}
		break;

	case _w:
		switch (incoming)
		{
		case 'a': complete_syllable (wa);		ForceBackspace = true;	return;
		case 'E': complete_syllable (wE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (we);		ForceBackspace = true;	return;
		case 'i': complete_syllable (wi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (wo);		ForceBackspace = true;	return;
		case 'u': complete_syllable (wu);		ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _x:
		switch (incoming)
		{
		case 'a': complete_syllable (xa);		ForceBackspace = true;	return;
		case 'E': complete_syllable (xE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (xe);		ForceBackspace = true;	return;
		case 'i': complete_syllable (xi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (xo);		ForceBackspace = true;	return;
		case 'u': complete_syllable (xu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_xW, xWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _xW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _s:
		switch (incoming)
		{
		case 'a': complete_syllable (sa);		ForceBackspace = true;	return;
		case 'E': complete_syllable (sE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (se);		ForceBackspace = true;	return;
		case 'i': complete_syllable (si);		ForceBackspace = true;	return;
		case 'o': complete_syllable (so);		ForceBackspace = true;	return;
		case 'u': complete_syllable (su);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_sW, sWa);	ForceBackspace = true;	return;
		case '2': complete_syllable (b_s);		return;
		default:  reset();		
		}
		break;

	case _sW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _v:
		switch (incoming)
		{
		case 'a': complete_syllable (va);		ForceBackspace = true;	return;
		case 'E': complete_syllable (vE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (ve);		ForceBackspace = true;	return;
		case 'i': complete_syllable (vi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (vo);		ForceBackspace = true;	return;
		case 'u': complete_syllable (vu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_vW, vWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _vW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _BR:
		switch (incoming)
		{
		case 'a': complete_syllable (b_a);			return;
		case 'e': complete_syllable (b_e);			return;
		case 'E': complete_syllable (b_E);			return;
		case 'i': complete_syllable (b_i);			return;
		case 'I': complete_syllable (b_I);			return;
		case 'o': complete_syllable (b_o);			return;
		case 'h': make_syllable		(_BRh, b_h);	return;
		case 'q': make_syllable		(_BRq, b_q);	return;
		case 'k': make_syllable		(_BRk, b_k);	return;
		case 'g': make_syllable		(_BRg, b_g);	return;
		case 'l': make_syllable		(_BRl, b_l);	return;
		case 'm': make_syllable		(_BRm, b_m);	return;
		case 'r': make_syllable		(_BRr, b_r);	return;
		case 'n': make_syllable		(_BRn, b_n);	return;
		case 'S': make_syllable		(_BRS, b_S);	return;
		case 's': make_syllable		(_BRs, b_s);	return;
		case 'u': complete_syllable (b_u);			return;
		case '?': complete_syllable (QUESTION);		return;
		default:  reset();						
		}
		break;

	case _BRs:
		switch (incoming)
		{
		case 'a': complete_syllable (b_sa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_sE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_se);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_si);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_so);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_su);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_BRsW, b_sWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRq:
		switch (incoming)
		{
		case 'a': complete_syllable (b_qa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_qE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_qe);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_qi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_qo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_qu);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRk:
		switch (incoming)
		{
		case 'a': complete_syllable (b_ka);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_kE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_ke);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_ki);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_ko);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_ku);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRg:
		switch (incoming)
		{
		case 'a': complete_syllable (b_ga);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_gE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_ge);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_gi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_go);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_gu);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRl:
		switch (incoming)
		{
		case 'a': complete_syllable (b_la);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_lE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_le);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_li);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_lo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_lu);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRm:
		switch (incoming)
		{
		case 'a': complete_syllable (b_ma);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_mE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_me);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_mi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_mo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_mu);			ForceBackspace = true;	return;;
		default:  reset();		
		}
		break;

	case _BRr:
		switch (incoming)
		{
		case 'a': complete_syllable (b_ra);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_rE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_re);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_ri);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_ro);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_ru);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRn:
		switch (incoming)
		{
		case 'a': complete_syllable (b_na);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_nE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_ne);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_ni);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_no);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_nu);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _X:
		switch (incoming)
		{
		case 'a': complete_syllable (Xa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (XE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (Xe);			ForceBackspace = true;	return;
		case 'i': complete_syllable (Xi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (Xo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (Xu);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRS:
		switch (incoming)
		{
		case 'a': complete_syllable (b_Sa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (b_SE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (b_Se);			ForceBackspace = true;	return;
		case 'i': complete_syllable (b_Si);			ForceBackspace = true;	return;
		case 'o': complete_syllable (b_So);			ForceBackspace = true;	return;
		case 'u': complete_syllable (b_Su);			ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _BRsW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _q:
		switch (incoming)
		{
		case '2': complete_syllable (b_q);			ForceBackspace = true;	return;
		case 'a': complete_syllable (qa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (qE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (qe);			ForceBackspace = true;	return;
		case 'i': complete_syllable (qi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (qo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (qu);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_qW, qW);	    ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _qW:
		switch (incoming)
		{
		case 'a': complete_syllable (qWa);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (qWe);			ForceBackspace = true;	return;
		case 'E': complete_syllable (qWE);			ForceBackspace = true;	return;
		case 'i': complete_syllable (qWi);			ForceBackspace = true;	return;
		case 'u': reset();									return;
		default:  reset();	
		}
		break;

	case _Q:
		switch (incoming)
		{
		case 'a': complete_syllable (Qa);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (QE);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (Qe);		    ForceBackspace = true;	return;
		case 'i': complete_syllable (Qi);		    ForceBackspace = true;	return;
		case 'o': complete_syllable (Qo);		    ForceBackspace = true;	return;
		case 'u': complete_syllable (Qu);		    ForceBackspace = true;	return;
		case 'W': make_syllable     (_QW, QW);	    ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _QW:
		switch (incoming)
		{
		case 'a': complete_syllable (QWa);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (QWe);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (QWE);          ForceBackspace = true;	return;
		case 'i': complete_syllable (QWi);			ForceBackspace = true;	return;
		case 'u': reset();									return;
		default:  reset();	
		}
		break;

	case _t:
		switch (incoming)
		{
		case 'a': complete_syllable (ta);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (tE);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (te);		    ForceBackspace = true;	return;
		case 'i': complete_syllable (ti);		    ForceBackspace = true;	return;
		case 'o': complete_syllable (to);		    ForceBackspace = true;	return;
		case 'u': complete_syllable (tu);		    ForceBackspace = true;	return;
		case 'W': make_syllable     (_tW, tWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _tW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _c:
		switch (incoming)
		{
		case 'a': complete_syllable (ca);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (cE);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (ce);		    ForceBackspace = true;	return;
		case 'i': complete_syllable (ci);		    ForceBackspace = true;	return;
		case 'o': complete_syllable (co);		    ForceBackspace = true;	return;
		case 'u': complete_syllable (cu);		    ForceBackspace = true;	return;
		case 'W': make_syllable     (_cW, cWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _cW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);					
		}
		break;

	case _n:
		switch (incoming)
		{
		case 'a': complete_syllable (na);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (nE);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (ne);		    ForceBackspace = true;	return;
		case 'i': complete_syllable (ni);		    ForceBackspace = true;	return;
		case 'o': complete_syllable (no);		    ForceBackspace = true;	return;
		case 'u': complete_syllable (nu);		    ForceBackspace = true;	return;
		case 'W': make_syllable     (_nW, nWa);		ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _nW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);					
		}
		break;

	case _N:
		switch (incoming)
		{
		case 'a': complete_syllable (Na);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (NE);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (Ne);		    ForceBackspace = true;	return;
		case 'i': complete_syllable (Ni);		    ForceBackspace = true;	return;
		case 'o': complete_syllable (No);		    ForceBackspace = true;	return;
		case 'u': complete_syllable (Nu);		    ForceBackspace = true;	return;
		case 'W': make_syllable     (_NW, NWa);		ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _NW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _BRh:
		switch (incoming)
		{
		case 'a': complete_syllable (b_ha);		    ForceBackspace = true;	return;
		case 'E': complete_syllable (b_hE);		    ForceBackspace = true;	return;
		case 'e': complete_syllable (b_he);		    ForceBackspace = true;	return;
		case 'i': complete_syllable (b_hi);		    ForceBackspace = true;	return;
		case 'o': complete_syllable (b_ho);		    ForceBackspace = true;	return;
		case 'u': complete_syllable (b_hu);		    ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _k:
		switch (incoming)
		{
		case 'a': complete_syllable (ka);			ForceBackspace = true;	return;
		case 'E': complete_syllable (kE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (ke);			ForceBackspace = true;	return;
		case 'i': complete_syllable (ki);			ForceBackspace = true;	return;
		case 'o': complete_syllable (ko);			ForceBackspace = true;	return;
		case 'u': complete_syllable (ku);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_kW, kW);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _kW:
		switch (incoming)
		{
		case 'a': complete_syllable (kWa);			ForceBackspace = true;	return;
		case 'e': complete_syllable (kWe);			ForceBackspace = true;	return;
		case 'E': complete_syllable (kWE);			ForceBackspace = true;	return;
		case 'i': complete_syllable (kWi);			ForceBackspace = true;	return;
		case 'u': reset();									return;
		default:  reset();		
		}
		break;

	case _K:
		switch (incoming)
		{
		case 'a': complete_syllable (Ka);			ForceBackspace = true;	return;
		case 'E': complete_syllable (KE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (Ke);			ForceBackspace = true;	return;
		case 'i': complete_syllable (Ki);			ForceBackspace = true;	return;
		case 'o': complete_syllable (Ko);			ForceBackspace = true;	return;
		case 'u': complete_syllable (Ku);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_KW, KW);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _KW:
		switch (incoming)
		{
		case 'a': complete_syllable (KWa);			ForceBackspace = true;	return;
		case 'e': complete_syllable (KWe);			ForceBackspace = true;	return;
		case 'E': complete_syllable (KWE);			ForceBackspace = true;	return;
		case 'i': complete_syllable (KWi);			ForceBackspace = true;	return;
		case 'u': reset();									return;
		default:  reset();		
		}
		break;

	case _z:
		switch (incoming)
		{
		case 'a': complete_syllable (za);			ForceBackspace = true;	return;
		case 'E': complete_syllable (zE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (ze);			ForceBackspace = true;	return;
		case 'i': complete_syllable (zi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (zo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (zu);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_zW, zWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _zW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _y:
		switch (incoming)
		{
		case 'a': complete_syllable (ya);			ForceBackspace = true;	return;
		case 'E': complete_syllable (yE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (ye);			ForceBackspace = true;	return;
		case 'i': complete_syllable (yi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (yo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (yu);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_yW, yWa);		ForceBackspace = true;	return;
		default:  reset();
		}
		break;

	case _yW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _d:
		switch (incoming)
		{
		case 'a': complete_syllable (da);			ForceBackspace = true;	return;
		case 'E': complete_syllable (dE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (de);			ForceBackspace = true;	return;
		case 'i': complete_syllable (di);			ForceBackspace = true;	return;
		case 'o': complete_syllable (ddo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (du);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_dW, dWa);		ForceBackspace = true;	return;
		default:  reset();	
		}
		break;

	case _dW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);				
		}
		break;

	case _j:
		switch (incoming)
		{
		case 'a': complete_syllable (ja);			ForceBackspace = true;	return;
		case 'E': complete_syllable (jE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (je);			ForceBackspace = true;	return;
		case 'i': complete_syllable (ji);			ForceBackspace = true;	return;
		case 'o': complete_syllable (jo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (ju);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_jW, jWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _jW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _g:
		switch (incoming)
		{
		case '2': complete_syllable (b_g);			ForceBackspace = true;	return;
		case 'a': complete_syllable (ga);			ForceBackspace = true;	return;
		case 'E': complete_syllable (gE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (ge);			ForceBackspace = true;	return;
		case 'i': complete_syllable (gi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (go);			ForceBackspace = true;	return;
		case 'u': complete_syllable (gu);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_gW, gW);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _gW:
		switch (incoming)
		{
		case 'a': complete_syllable (gWa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (gWE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (gWe);			ForceBackspace = true;	return;
		case 'i': complete_syllable (gWi);			ForceBackspace = true;	return;
		case 'u': reset();									return;
		default:  reset();		
		}
		break;

	case _f:
		switch (incoming)
		{
		case 'a': complete_syllable (fa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (fE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (fe);			ForceBackspace = true;	return;
		case 'i': complete_syllable (fi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (fo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (fu);			ForceBackspace = true;	return;
		case 'Y': make_syllable     (_fY, fYa);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_fW, fWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _fW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);									return;
		case 'e': complete_syllable  (fWe);			ForceBackspace = true;	return;
		case 'i': complete_syllable  (fWi);			ForceBackspace = true;	return;
		case 'E': complete_syllable  (fWE);			ForceBackspace = true;	return;
		case 'u': complete_syllable  (fWu);			ForceBackspace = true;	return;
		default:  complete_syllable  (0);	
		}
		break;

	case _p:
		switch (incoming)
		{
		case 'a': complete_syllable (pa);			ForceBackspace = true;	return;
		case 'E': complete_syllable (pE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (pe);			ForceBackspace = true;	return;
		case 'i': complete_syllable (pi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (po);			ForceBackspace = true;	return;
		case 'u': complete_syllable (pu);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_pW, pWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _pW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);								return;
		case 'e': complete_syllable  (pWe);			ForceBackspace = true;	return;
		case 'i': complete_syllable  (pWi);			ForceBackspace = true;	return;
		case 'E': complete_syllable  (pWE);			ForceBackspace = true;	return;
		case 'u': complete_syllable  (pWu);			ForceBackspace = true;	return;
		default:  complete_syllable  (0);	
		}
		break;

	case _Z:
		switch (incoming)
		{
		case 'a': complete_syllable (Za);			ForceBackspace = true;	return;
		case 'E': complete_syllable (ZE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (Ze);			ForceBackspace = true;	return;
		case 'i': complete_syllable (Zi);			ForceBackspace = true;	return;
		case 'o': complete_syllable (Zo);			ForceBackspace = true;	return;
		case 'u': complete_syllable (Zu);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_ZW, ZWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _ZW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);			return;
		default:  complete_syllable  (0);						
		}
		break;

	case _D:
		switch (incoming)
		{
		case 'a': complete_syllable (Da);			ForceBackspace = true;	return;
		case 'E': complete_syllable (DE);			ForceBackspace = true;	return;
		case 'e': complete_syllable (De);			ForceBackspace = true;	return;
		case 'i': complete_syllable (Di);			ForceBackspace = true;	return;
		case 'o': complete_syllable (Do);			ForceBackspace = true;	return;
		case 'u': complete_syllable (Du);			ForceBackspace = true;	return;
		case 'W': make_syllable     (_DW, DWa);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _DW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _G:
		switch (incoming)
		{
		case 'a': complete_syllable (Ga);		ForceBackspace = true;	return;
		case 'E': complete_syllable (GE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (Ge);		ForceBackspace = true;	return;
		case 'i': complete_syllable (Gi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (Go);		ForceBackspace = true;	return;
		case 'u': complete_syllable (Gu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_GW, GWu);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _GW:
		switch (incoming)
		{
		case 'a': complete_syllable  (GWa);		ForceBackspace = true;	return;
		case 'e': complete_syllable  (GWe);		ForceBackspace = true;	return;
		case 'i': complete_syllable  (GWi);		ForceBackspace = true;	return;
		case 'E': complete_syllable  (GWE);		ForceBackspace = true;	return;
		case 'u': complete_syllable  (0);								return;
		default:  complete_syllable  (0);		
		}
		break;

	case _T:
		switch (incoming)
		{
		case 'a': complete_syllable (Ta);		ForceBackspace = true;	return;
		case 'E': complete_syllable (TE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (Te);		ForceBackspace = true;	return;
		case 'i': complete_syllable (Ti);		ForceBackspace = true;	return;
		case 'o': complete_syllable (To);		ForceBackspace = true;	return;
		case 'u': complete_syllable (Tu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_TW, TWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _TW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _C:
		switch (incoming)
		{
		case 'a': complete_syllable (Ca);		ForceBackspace = true;	return;
		case 'E': complete_syllable (CE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (Ce);		ForceBackspace = true;	return;
		case 'i': complete_syllable (Ci);		ForceBackspace = true;	return;
		case 'o': complete_syllable (Co);		ForceBackspace = true;	return;
		case 'u': complete_syllable (Cu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_CW, CWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _CW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _P:
		switch (incoming)
		{
		case 'a': complete_syllable (Pa);		ForceBackspace = true;	return;
		case 'E': complete_syllable (PE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (Pe);		ForceBackspace = true;	return;
		case 'i': complete_syllable (Pi);		ForceBackspace = true;	return;
		case 'o': complete_syllable (Po);		ForceBackspace = true;	return;
		case 'u': complete_syllable (Pu);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_PW, PWa);	ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _PW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);					
		}
		break;

	case _S:
		switch (incoming)
		{
		case 'a': complete_syllable (Sa);		ForceBackspace = true;	return;
		case 'E': complete_syllable (SE);		ForceBackspace = true;	return;
		case 'e': complete_syllable (Se);		ForceBackspace = true;	return;
		case 'i': complete_syllable (Si);		ForceBackspace = true;	return;
		case 'o': complete_syllable (So);		ForceBackspace = true;	return;
		case 'u': complete_syllable (Su);		ForceBackspace = true;	return;
		case 'W': make_syllable     (_SW, SWa);	ForceBackspace = true;	return;
		case '2': complete_syllable (b_S);		ForceBackspace = true;	return;
		default:  reset();		
		}
		break;

	case _SW:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _mY:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _rY:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);						
		}
		break;

	case _fY:
		switch (incoming)
		{
		case 'a': complete_syllable  (0);		return;
		default:  complete_syllable  (0);					
		}
		break;

	default:
		assert (unknownState == true); //Error: unknown state!
	}
  }
}

//! calls IdentifyChar which will actually handle the conversion
const PWCHAR AmharicEngine::convertASCII(const WCHAR& incoming)
{
	IdentifyChar(incoming);

	//for Excel, instead of backspacing on the screen, backspace in the buffer
	if(isExcel && ForceBackspace)
		shift_left();

	return to_send();
}


////////////////////////////////////////////////////////////////////////
//private utility functions


//! converts ASCII numbers to Amharic  & appends to ToSend
void AmharicEngine::numberConvertor()
{
	WCHAR temp[BIG_NUMBER];
	temp[0]       = '\0';
	int length    = 0; 
	int counter   = 0; 
	int inc       = 0; 
	bool hundreds = true;
	int xx        = 0;

	while(numberBuffer[length] != 0)	{ ++length; } 
	int index = length - 1;

	enum {ONES = 1, TENS, HUNDREDS_TENTHOUSANDS};

	while(index >= 0)
	{
		++counter;
		switch (counter)
		{
		case ONES:
			temp[inc++] = ones(numberBuffer[index]); 
			--index;
			break;

		case TENS:
			temp[inc] = tens(numberBuffer[index]); 

			//omitt the meto (100) if the two proceeding digits are both zero
			xx = inc;
			if(temp[xx--] == 0 && temp[xx--] == 0 && temp[xx] == HUNDRED)	
			{ 
				inc -= 2; 
			}
			else		{ ++ inc;   }

			--index;
			break;

		case HUNDREDS_TENTHOUSANDS:
			if(hundreds)
			{
				temp[inc++] = HUNDRED; 
				hundreds = false;
			}
			else
			{
				temp[inc++] = TEN_THOUSAND; 
				hundreds = true;
			}
			counter = 0;
			break;
		}
	}
	--inc;

	//start at end of temp & append it ToSend
	while (inc >= 0)			{ append(ToSend, temp[inc--]); }
}

//! specifically identifies the Geez meto (100) character
bool AmharicEngine::isHundred(PWCHAR str)
{
	return (*str++ == '1' && *str++ == '0' && *str++ == '0' && *str == 0);
}

//! specifically identifies the Geez 10,000 character
bool AmharicEngine::isTenThousand(PWCHAR str)
{
	return (*str++ == '1' && *str++ == '0' && *str++ == '0' && *str++ == '0' 
		&& *str++ == '0' && *str == 0);
}

//! returns appropiate Amharic one's character
const WCHAR AmharicEngine::ones(const WCHAR& num)
{
	switch (num)
	{
	case '0':
		return ZERO;
	case '1':
		return ONE;
	case '2': 
		return TWO;
	case '3':
		return THREE;
	case '4':
		return FOUR;
	case '5':
		return FIVE;
	case '6':
		return SIX;
	case '7': 
		return SEVEN;
	case '8': 
		return EIGHT;
	case '9':
		return NINE;
	}
	return 0;
}

//! returns appropiate Amharic ten's character
const WCHAR AmharicEngine::tens(const WCHAR& num)
{
	switch (num)
	{
	case '0':
		return ZERO;
	case '1':
		return TEN;
	case '2': 
		return TWENTY;
	case '3':
		return THIRTY;
	case '4':
		return FORTY;
	case '5':
		return FIFTY;
	case '6':
		return SIXTY;
	case '7': 
		return SEVENTY;
	case '8': 
		return EIGHTY;
	case '9':
		return NINETY;
	}
	return 0;
}


//! convert certain uppercase characters to lowercase since they are used interchangeably
void AmharicEngine::toLowercase(WCHAR& ch)
{
	switch(ch)
	{
	case 'A':
	case 'B':
	case 'F':
	case 'J':
	case 'L':
	case 'M':
	case 'O':
	case 'R':
	case 'U':
	case 'V':
		ch += 0x20;
		break;
	}

	//usually we need to convert 'Y' to lowercase, but NOT if the previous key was
	//'m', 'r', or 'f'
	if(ch == 'Y')
	{
		switch(State)
		{
		case _r:
		case _m:
		case _f:
			break;
		default:
			ch += 0x20;
		}
	}
}












