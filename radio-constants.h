#
/*
 *    Copyright (C)  2011 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the cw skimmer
 *
 *    cw skimmer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    cw skimmer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cw skimmer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#ifndef __RADIO_CONSTANTS_H
#define	__RADIO_CONSTANTS_H

#include	<math.h>
#include	<complex>
#include	<stdint.h>
#include	<limits>
#include	<stdlib.h>
#include	<unistd.h>
#ifdef __MINGW32__
#include        "windows.h"
#else
#include        "alloca.h"
#include        "dlfcn.h"
typedef void    *HINSTANCE;
#endif
#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif

//
//
typedef	std::complex<float>	DSPCOMPLEX;

using namespace std;

#define	Hz(x)		(x)
#define	Khz(x)		(x * 1000)
#define	KHz(x)		(x * 1000)
#define	kHz(x)		(x * 1000)
#define	Mhz(x)		(Khz (x) * 1000)
#define	MHz(x)		(KHz (x) * 1000)
#define	mHz(x)		(KHz (x) * 1000)
/*
 */
#define	MINIMUM(x, y)	((x) < (y) ? x : y)
#define	MAXIMUM(x, y)	((x) > (y) ? x : y)

#define	IandQ		0100
#define	QandI		0101
#define I_Only		0102
#define	Q_Only		0104

#define	CURRENT_VERSION	"8.1"

//
//	common functions
static inline
bool	isIndeterminate (float x) {
	return x != x;
}

static inline
bool	isInfinite (double x) {
	return x == numeric_limits<float>::infinity ();
}
//
static inline
DSPCOMPLEX cmul (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) * y, imag (x) * y);
}


static inline
DSPCOMPLEX cdiv (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) / y, imag (x) / y);
}


static inline
float	get_db (float x) {
	return 20 * log10 ((x + 1) / (float)(2048));
}


static	inline
float	PI_Constrain (float val) {
	if (0 <= val && val < 2 * M_PI)
	   return val;
	if (val >= 2 * M_PI)
	   return fmod (val, 2 * M_PI);
//	apparently val < 0
	if (val > - 2 * M_PI)
	   return val + 2 * M_PI;
	return 2 * M_PI - fmod (- val, 2 * M_PI);
}
#endif

