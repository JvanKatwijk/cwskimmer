#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the cwSkimmer
 *
 *    cwSkimmer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    cwSkimmer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cwSkimmer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__ELEMENT_HANDLER__
#define	__ELEMENT_HANDLER__

#include	<QObject>
#include	<stdint.h>
#include	<stdio.h>
#include	"averager.h"
#include	<cstring>
#include	<string>

#define	MODE_IDLE		0100
#define	MODE_TONE		0200
#define	MODE_SPACE		0500

#define	CW_DOT_REPRESENTATION	'.'
#define	CW_DASH_REPRESENTATION	'_'
#define	CW_IF			0
/*
 * 	The standard morse wordlength (i.e.
 * 	PARIS) is 50 bits, then for a wpm of 1,
 * 	one bit takes 1.2 second or 1.2 x 10^6	microseconds
 */
#define	DOT_MAGIC		1200000

#define	CWError		1000
#define	CWNewLetter	1001
#define	CWCarrierUp	1002
#define	CWCarrierDown	1005
#define	CWStroke	1003
#define	CWDot		1004
#define	CWendofword	1006
#define	CWFailing	1007

#define	CW_RECEIVE_CAPACITY	040
class	RadioInterface;

class	elementHandler: public QObject{
Q_OBJECT
public:
		elementHandler	(RadioInterface *, int);
		~elementHandler	();
	void	reset		(int Frequency);
	void	process		(float value, float signal);
	void	add		(int, int);
private:
	average		smoothenSamples;
	float		decayingAverage (float old,
	                                 float input, float weight);

	int		identity;
	float		agc_peak;
	float		avg;
	float		noiseLevel;
	float		value;
	int		cwState;
	int		currentTime;
	int		cwCurrent;
	int		cwStartTime;
	int		starter;
	int		cwFrequency;
	std::string	cwText;
	int		wordCount;
	int		*buffer;
	int		fillerP;
	int		emptyP;

	void		cw_addText	(char c);
	void		cw_clrText	();
	void		lookupToken	(char *, char *);
	void		printChar	(char a, char err);
signals:
	void		updateText	(int, int, const QString &);
};
#endif

