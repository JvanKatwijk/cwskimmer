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
#include	"element-handler.h"
#include	<stdio.h>
#include	<algorithm>
#include	<cstring>
#include	<vector>
#include	"radio.h"

	elementHandler::elementHandler (RadioInterface *mr,
	                                int ident): 
	                             smoothenSamples (5) {
	this	-> identity	= ident;
	connect (this, SIGNAL (updateText (int, int, const QString &)),
	         mr, SLOT (updateText (int, int, const QString &)));
	reset	 (140700000);	// just a default
}

	elementHandler::~elementHandler () {}

void	elementHandler::reset	(int frequency) {
	cwFrequency		= frequency;
	cwCurrent		= 0;
	agc_peak		= 0;
	noiseLevel		= 0;
	avg			= 0;
	starter			= 0;
	cwState			= MODE_IDLE;
	cwText			= "";

//
//	a PARIS bit is 1200 milliseconds, we use a 2 msec period
//	so a PARIS bit takes here 600 samples
//	All length are expressed in samples
	currentTime		= 0;
	fillerP			= 0;
	emptyP			= 0;
}

void	elementHandler::process	(float value, float signalValue) {
int	lengthOfSilence;
int	lengthOfTone;

	if (value > agc_peak)
	   agc_peak = decayingAverage (agc_peak, value, 50.0);
	else
	   agc_peak = decayingAverage (agc_peak, value, 500.0);

	value	= smoothenSamples. filter (value);
	avg	= 0.99 * avg + 0.01 * value;
	if (starter < 100) {
	   starter ++;
	   return;
	}
//
//	the incoming samplerate is 1000 / 2 samples/second,
//	i.e. app 500 
	currentTime	+= 1;

	switch (cwState) {
	   case MODE_IDLE:
	      if (value > 0.77 * agc_peak) {
	         cwStartTime	= currentTime;
	         cwState	= MODE_TONE;
	      }
	      break;

	   case MODE_TONE:
	      if (value < 0.43 * agc_peak) {	// end of tone?
	         int duration	= currentTime - cwStartTime;
	         if (duration < 3)	// see it as a spike
	            break;
	         cwState	= MODE_SPACE;
	         cwStartTime	= currentTime;
	         add (MODE_TONE, duration);
	         break;
	      }
	      break;

	   case MODE_SPACE:
	      if (value > 0.67 * agc_peak) {
	         int duration = currentTime - cwStartTime;
	         if (duration < 4)	// see it as noise
	            break;
	         cwState	= MODE_TONE;
	         cwStartTime	= currentTime;
	         add (MODE_SPACE, duration);
	         break;
	      }
	      break;

	   default: ;
	}
}

bool	sorter (int i, int j) {
	return i < j;
}

#define	SEARCH_LENGTH	14
bool	isDot (int value, int norm) {
	if (value < 0)
	   fprintf (stderr, "Help 1");
	return value < 1.5 * norm;
}

bool	isLongSpace	(int value, int norm) {
	if (value > 0)
	   fprintf (stderr, "Help 2");
	return -value > norm;
}
//
//	The approach we take is to put the durations of the tones and spaces
//	into a queue. We know that a Morse symbol does not exceed 5
//	dots/dashes, so if the queue is sufficiently filled, we
//	try to figure out what for this combination the DOT
//	length would be.
void	elementHandler::add (int soort, int duration) {
int	bufferElems	= 0;

	buffer [fillerP] = soort == MODE_SPACE ? -duration : duration;
	fillerP		= (fillerP + 1) % QUEUE_LENGTH;
	bufferElems	= (fillerP + QUEUE_LENGTH - emptyP) % QUEUE_LENGTH;
	if (bufferElems < SEARCH_LENGTH)
	   return;
//
//	The first thing to do is to guess the length of the DOT
	std::vector<int> spaceSizes;
	std::vector<int> toneSizes;
	for (int i = 0; i < SEARCH_LENGTH; i ++) {
	   if (buffer [(emptyP + i) % QUEUE_LENGTH] < 0)
	      spaceSizes. push_back (-buffer [(emptyP + i) % QUEUE_LENGTH]);
	   else
	      toneSizes. push_back (buffer [(emptyP + i) % QUEUE_LENGTH]);
	}
	std::sort (spaceSizes. begin (), spaceSizes. end ());
	std::sort (toneSizes. begin (), toneSizes. end ());

	int spaceGuess	= spaceSizes. at (0);
	int dotGuess	= toneSizes. at (0);
//
//	since we know that a symbol takes at most 6 dash/dot combinations,
//	the longest space should be at least 2 times the shortest
//
	if (2 * spaceGuess > spaceSizes.  at (spaceSizes. size () - 1)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
	}
	wordCount	= 600 / spaceGuess;
	if ((wordCount < 5) || (wordCount > 40)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
	}
	   
//	it is known that Morse symbols are at most 5 dash/dot combis
	for (int i = 0; i < 6; i ++) {
	   int space = -buffer [(emptyP + 2 * i + 1) % QUEUE_LENGTH];
	   if (space > 2 * spaceGuess) {
	      char bbb [10];
	      for (int j = 0; j <= i; j ++) {
	         int xx = buffer [(emptyP + 2 * j) % QUEUE_LENGTH];
//	         fprintf (stderr, "%d ", xx);
	         if (xx > 1.5 * spaceGuess)
	            bbb [j] = '_';
	         else
	            bbb [j] = '.';
	      }
	      bbb [i + 1] = 0;
	      char aaa [4];
	      lookupToken (bbb, aaa);
//	      fprintf (stderr, "%s  %c\n", bbb, aaa [0]);
	      cw_addText (aaa [0]);
	      if (space > 3 * spaceGuess) {
	         cw_addText (' ');
	      }
	      emptyP = (emptyP + 2 * i + 2) % QUEUE_LENGTH;
	      return;
	   }
	}
	emptyP = (emptyP + 2) % QUEUE_LENGTH;
}
	
float	elementHandler::decayingAverage (float old, float input, float weight) {
	if (weight <= 1)
	   return input;
	return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}


void 	elementHandler::printChar (char a, char er) {
	if ((a & 0x7f) < 32) {
	   switch (a) {
	      case '\n':		break;
	      case '\r':		return;
	      case 8:			break;
	      case 9:			break;
	      default:			a = ' ';
	   }
	}

	switch (er) {
	   case 0:	printf("%c",a);break;
	   case 1:	printf("\033[01;42m%c\033[m",a); break;
	   case 2:	printf("\033[01;41m%c\033[m",a); break;
	   case 3:	printf("\033[01;43m%c\033[m",a); break;
	   case 4:
	   case 5:
	   case 6:
	   case 7:	printf("\033[2J\033[H<BRK>\n"); break;
	   default:
			break;
	}
}

const char * const codetable[] = {
	"A._",
	"B_...",
	"C_._.",
	"D_..",
	"E.",
	"F.._.",
	"G__.",
	"H....",
	"I..",
	"J.___",
	"K_._",
	"L._..",
	"M__",
	"N_.",
	"O___",
	"P.__.",
	"Q__._",
	"R._.",
	"S...",
	"T_",
	"U.._",
	"V..._",
	"W.__",
	"X_.._",
	"Y_.__",
	"Z__..",
	"0_____",
	"1.____",
	"2..___",
	"3...__",
	"4...._",
	"5.....",
	"6_....",
	"7__...",
	"8___..",
	"9____.",
	".._._._",
	",__..__",
	"?..__..",
	"~"
	};

void	elementHandler::lookupToken (char *in, char *out) {
int	i;

	for (i = 0; codetable [i][0] != '~'; i ++) {
	   if (strcmp (&codetable [i][1], in) == 0) {
	      out [0] = codetable [i][0];
	      return;
	   }
	}
	out [0] = '*';
}

void	elementHandler::cw_clrText () {
	cwText		= std::string ("");
//	cwTextbox	-> setText (cwText);
}

void	elementHandler::cw_addText (char c) {
	if (c < ' ') c = ' ';
	cwText +=  char (c);
	if (cwText. length () > 75)
	   cwText. erase (0, 1);
	updateText (identity, wordCount, QString::fromStdString (cwText));
//	fprintf (stderr, " %d %d %s\n", identity, cwFrequency, cwText. c_str ());
//	cwTextbox	-> setText (cwText);
}

