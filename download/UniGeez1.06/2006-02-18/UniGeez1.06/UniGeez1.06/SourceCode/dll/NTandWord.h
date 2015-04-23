/* 	Copyright(c) October2000 by Marcus Wright and Will Briggs (Lynchburg College, VA).
	This program is free software; redistribution or modification is permitted under 
	the terms of the GNU General Public License version 2 as published by the Free 
	Software Foundation. */

#ifndef NTANDWORD_H
#define NTANDWORD_H

//! deals specifically with using MSWord in on the NT platform
struct NTandWord
{
	bool state;
	bool bkspForNT_Char;
	bool bkspForNT_Num;
};

#endif