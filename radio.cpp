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
#include	<unistd.h>
#include	<QSettings>
#include	<QDebug>
#include	<QDateTime>
#include	<QObject>
#include	<QDir>
#include	<QColor>
#include	<QMessageBox>
#include	<QFileDialog>
#include	"radio.h"
#include	"fft-scope.h"
#include	"scope.h"
#include	"fft-complex.h"
#include	"popup-keypad.h"
//
//      devices
#include        "device-handler.h"
#include	"deviceselect.h"
#include        "filereader.h"
#ifdef	HAVE_SDRPLAY
#include        "sdrplay-handler.h"
#endif
#ifdef	HAVE_SDRPLAY_V2
#include        "sdrplay-handler-v2.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
//

#define	D_SDRPLAY	"sdrplay"
#define	D_SDRPLAY_V3	"sdrplay-v3"
#define	D_HACKRF	"hackrf"
#define	D_RTLSDR	"dabstick"
#define	D_FILEREADER	"filereader"
static 
const char *deviceTable [] = {
#ifdef	HAVE_SDRPLAY
	D_SDRPLAY_V3,
#endif
#ifdef	HAVE_SDRPLAY_V2
	D_SDRPLAY,
#endif
#ifdef	HAVE_HACKRF
	D_HACKRF,
#endif
#ifdef	HAVE_LIME
	D_LIME,
#endif
#ifdef	HAVE_DABSTICK
	D_RTLSDR,
#endif
	D_FILEREADER,
	nullptr
};

static int startKnop;
static	QTimer	*starter;
#include	"element-handler.h"
#define	NR_ELEMENTS	48
elementHandler *workVector [NR_ELEMENTS];

#define	FFT_SIZE	2048
static inline
int twoPower (int a) {
int	res	= 1;
	while (--a >= 0)
	   res <<= 1;
	return res;
}

QString	FrequencytoString (int32_t freq) {
	if (freq < 10)
	   return QString ('0' + (uint8_t)(freq % 10));
	return 
	   FrequencytoString (freq / 10). append (QString ('0' + (uint8_t)(freq % 10)));
}

	RadioInterface::RadioInterface (QSettings	*sI,
	                                QWidget		*parent):
	                                    QMainWindow (parent),
	                                    inputBuffer (128 * 1024),
	                                    theOutput (this, "dummy") {
	this	-> settings	= sI;
	this	-> inputRate	= 192000;
	setupUi (this);
//      and some buffers
//

//	and the decoders
	displaySize		= 1024;
	scopeWidth		= inputRate;

	signalValue		= 0;

	mouseIncrement		= 5;
//	scope and settings
	hfScopeSlider	-> setValue (50);
        hfScope		= new fftScope (hfSpectrum,
                                        displaySize,
                                        kHz (1),        // scale
                                        inputRate,
                                        hfScopeSlider -> value (),
                                        8);
	hfScope		-> set_bitDepth (12);	// default
	hfScope	-> setScope (centerFrequency, selectedFrequency - centerFrequency);

        connect (hfScope,
                 SIGNAL (clickedwithLeft (int)),
                 this,
                 SLOT (adjustFrequency_khz (int)));
	connect (hfScope,
	         SIGNAL (clickedwithRight (int)),
	         this, SLOT (switch_hfViewMode (int)));
        connect (hfScopeSlider, SIGNAL (valueChanged (int)),
                 this, SLOT (set_hfscopeLevel (int)));
	LFScope		= new SpectrumViewer (lfPlot, NR_ELEMENTS);

        mykeyPad                = new keyPad (this);
        connect (freqButton, SIGNAL (clicked (void)),
                 this, SLOT (handle_freqButton (void)));

	connect (mouse_Inc, SIGNAL (valueChanged (int)),
	         this, SLOT (set_mouseIncrement (int)));

	theDevice		= nullptr;
	delayCount		= 0;

	secondsTimer. setInterval (1000);
	connect (&secondsTimer, SIGNAL (timeout ()),
                 this, SLOT (updateTime ()));
        secondsTimer. start (1000);
//
//	create the decoders
	for (int i = 0; i < NR_ELEMENTS; i ++) {
	   workVector [i] = new elementHandler (this, i);
	   theOutput. addRow ();
	}
//	If we had a device previously, we try that,
//	if it does not work, or this is none, the menu appears
//	The menu is activated by a timer that is started, such that
//	first the creation function can terminate
//	Actual starting of the process is therefore delayed until
//	we have an input device
	QString device =
	             settings -> value ("device", "no device").toString ();

	int k = -1;
	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   if (deviceTable [i] == device) {
	      k = i;
	      break;
	}
	if (k != -1) {		// we have a device?
	   starter	= new QTimer;
	   startKnop	= k;
	   starter -> setSingleShot (true);
	   starter -> setInterval (500);
	   connect (starter, SIGNAL (timeout()), 
	            this, SLOT (quickStart ()));
	   starter -> start (500);
	}
	else {
//	   deviceSelector	-> setCurrentIndex (0);
	   startKnop	= 0;
	   if (setDevice (settings) == nullptr)
	      exit (2);
	}
}

void	RadioInterface::setStart () {
	hfScope		-> set_bitDepth (theDevice -> bitDepth ());
	setFrequency (14070000);
	connect (theDevice, SIGNAL (dataAvailable (int)),
	         this, SLOT (sampleHandler (int)));
	centerFrequency		= KHz (14070);
	selectedFrequency	= KHz (14070);

	centerBin		= settings -> value ("centerBin", 
	                                            NR_ELEMENTS / 2). toInt ();
	nrBins			= settings -> value ("nrBins", 5). toInt ();

	centerChannel	-> setMinimum (3);
	centerChannel	-> setMaximum (NR_ELEMENTS - 4);
	decodeWidth	-> setMaximum (NR_ELEMENTS / 2);
	if (centerBin > centerChannel -> maximum ())
	   centerBin = centerChannel -> maximum ();;
	if (centerBin < centerChannel -> minimum ())
	   centerBin = centerChannel -> minimum ();

	
	if (nrBins & 01) {
	   lowEnd	= centerBin - nrBins / 2;
	   highEnd	= centerBin + nrBins / 2;
	}
	else {
	   lowEnd 	= centerBin - nrBins / 2;
	   highEnd	= centerBin + nrBins / 2 - 1;
	   fprintf (stderr, "Bins = %d, low %d high %d  center %d\n",
	                           nrBins, lowEnd, highEnd, centerBin);
	}

	if (lowEnd < 0)
	   lowEnd	= 0;
	if (highEnd >= NR_ELEMENTS)
	   highEnd	= NR_ELEMENTS - 1;
	
	centerChannel		-> setValue (centerBin);
	decodeWidth		-> setValue (nrBins);
	connect (decodeWidth, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_decodeWidth (int)));
	connect (centerChannel, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_centerChannel (int)));
//	and off we go
	theOutput. show ();
	theDevice	-> restartReader ();
}

//      The end of all
        RadioInterface::~RadioInterface () {
	theOutput. hide ();
}
//
//	If the user quits before selecting a device ....
void	RadioInterface::handle_quitButton	() {
	if (theDevice != NULL) {
	   theDevice	-> stopReader ();
	   disconnect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	   delete  theDevice;
	}
	sleep (1);
	fprintf (stderr, "device is deleted\n");
	secondsTimer. stop ();
	delete		hfScope;
        delete		mykeyPad;
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   delete workVector [i];
}
//
//	
void    RadioInterface::handle_freqButton (void) {
        if (mykeyPad -> isVisible ())
           mykeyPad	-> hidePad ();
        else
           mykeyPad     -> showPad ();
}

//	setFrequency is called from the frequency panel
//	as well as from inside to change VFO and offset
void	RadioInterface::setFrequency (int32_t freq) {
	centerFrequency		= freq;
	selectedFrequency	= freq;
	theDevice	-> setVFOFrequency (centerFrequency);
	hfScope		-> setScope  (centerFrequency,
	                              centerFrequency - selectedFrequency);
	QString ff	= FrequencytoString (selectedFrequency);
	frequencyDisplay	-> display (ff);
	for (int i = lowEnd; i < highEnd; i ++) {
	   int freqOffset = (-NR_ELEMENTS / 2 + i) * 192000.0 / FFT_SIZE;
	   workVector [i] -> reset ((freq + freqOffset));
	   theOutput. setFrequency (i, (freq + freqOffset));
	}
}

void	RadioInterface::updateText	(int index, int wc, const QString &s) {
	theOutput. updateText (index, wc, s);
}
//
//	adjustFrequency is called whenever clicking the mouse
void	RadioInterface::adjustFrequency_khz (int32_t n) {
	adjustFrequency_hz (1000 * n);
}

void	RadioInterface::adjustFrequency_hz (int32_t n) {
int h = inputRate / 2 - KHz (5);
	if ((selectedFrequency + n >= centerFrequency + h) ||
	    (selectedFrequency + n <= centerFrequency - h)) {
	   setFrequency (centerFrequency + n);	
	   centerFrequency = theDevice -> getVFOFrequency ();
	}
	else {
	   selectedFrequency += n;
	}

	hfScope		-> setScope (centerFrequency,
	                             selectedFrequency - centerFrequency);
	QString ff 		= FrequencytoString (selectedFrequency);
	frequencyDisplay	-> display (selectedFrequency);
	for (int i = lowEnd; i <= highEnd; i ++) {
	   int freqOffset = (-NR_ELEMENTS / 2.0 + i) * 192000.0 / FFT_SIZE;
	   workVector [i] -> reset (selectedFrequency + freqOffset);
	   theOutput. setFrequency (i, selectedFrequency + freqOffset);
	}
}

int32_t	RadioInterface::get_selectedFrequency	() {
	return selectedFrequency;
}

int32_t	RadioInterface::get_centerFrequency	() {
	return centerFrequency;
}
//
void	RadioInterface::set_mouseIncrement (int inc) {
	mouseIncrement = inc;
}

void	RadioInterface::wheelEvent (QWheelEvent *e) {
	adjustFrequency_hz ((e -> delta () > 0) ?
	                        mouseIncrement : -mouseIncrement);
}

//////////////////////////////////////////////////////////////////
//
void	RadioInterface::sampleHandler (int amount) {
std::complex<float>   buffer [FFT_SIZE];
static int cnt	= 0;
	(void)amount;
	while (inputBuffer. GetRingBufferReadAvailable () > 2 * 192) {
	   inputBuffer. getDataFromBuffer (buffer, 2 * 192);
	   for (int i = 2 * 192; i < FFT_SIZE; i ++) 
	      buffer [i] = std::complex<float> (0, 0);
	   processBuffer (buffer, FFT_SIZE);
	   cnt ++;
	   if (cnt % 10 == 0) {	// 100 milliseconds
	      hfScope	-> addElements (buffer, FFT_SIZE);
	      cnt = 0;
	   }
	}
}

static
int	cnt	= 0;
void	RadioInterface::processBuffer	(std::complex<float> *b, int s) {
float sum	= 0;
double	LFVector [NR_ELEMENTS];
std::complex<float> res [FFT_SIZE];

	for (int i = 0; i < FFT_SIZE; i ++)
	   res [i] = b [i];
	Fft_transform (res, FFT_SIZE, false);

	float	avg	= 0;
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   avg += abs (res [(FFT_SIZE - NR_ELEMENTS / 2 + i) % FFT_SIZE]);

	avg	/= NR_ELEMENTS;
	signalValue	= 0.99 * signalValue + 0.01 * get_db (avg);
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   LFVector [i] = 0.1;		// 
	for (int k = lowEnd; k <= highEnd; k ++) {
	   int index = (FFT_SIZE - NR_ELEMENTS / 2 + k) % FFT_SIZE;
	   workVector [k] -> process (abs (res [index]), signalValue);
	   LFVector  [k] = abs (res [(FFT_SIZE - NR_ELEMENTS / 2 + k) % FFT_SIZE]);
	}
	if (++cnt > 50) { 
	   float max = 0;
	   double X_axis [NR_ELEMENTS];
	   float avg	= 0;
	   for (int i = - NR_ELEMENTS / 2; i < NR_ELEMENTS / 2;  i ++) 
	      X_axis [NR_ELEMENTS / 2 + i] = i;
	   for (int i = 0; i < NR_ELEMENTS; i ++) {
	      if (max < LFVector [i])
	         max = LFVector [i];
	   }
	   LFScope	-> ViewSpectrum (X_axis, LFVector, max, 0,  0);
	   cnt = 0;
	}
}

//
//from https://flylib.com/books/en/2.729.1/single_tone_detection.html
//
//
//std::complex<float> goertzel (std::complex<float> *input, int size,
//	                                        int rate, int freq) {
//std::complex<float> f2 = std::complex<float> (cos (2 * M_PI * freq / size),
//	                                      -sin (2 * M_PI * freq / size));
//float t2	= 2 * cos (2 * M_PI * freq / size);
//std::complex<float> w_min1	= std::complex<float> (0, 0);
//std::complex<float> w_min2 	= std::complex<float> (0, 0);
//std::complex<float> w		= std::complex<float> (0, 0);
//
//	for (int i =  0; i < size; i ++) {
//	   w_min2	= w_min1;
//	   w_min1	= w;
//	   w = t2 * w_min1 - w_min2 + input [i];
//	}
//	return w + f2 * w_min1;
//}
////
////
////	We only need about 32 elements, so we ask the system
////	to compute the 32 elements from 3 msec input
//void	RadioInterface::processBuffer	(std::complex<float> *b, int s) {
//std::complex<float> fftBuffer [3 * 192];
//double	Qr, Qi;
//double	LFVector [64];
//double	max	= 0;
//
//	for (int i = 0; i < 3 * 192; i ++)
//	   fftBuffer [i] = b [i];
//
//	for (int i = 0; i < 32; i ++) {
//	   int freq	=  (3 * 192 - 32 + 2 * i);
//	   std::complex<float> res =
//	          goertzel (fftBuffer, 3 * 192, 192000, freq);
//	   LFVector [2 * i] = LFVector [2 * i + 1] = abs (res);
//	   workVector [i]	-> process (abs (res), 0);
//	   if (abs (res) > max)
//	      max = abs (res);
//	}
//	cnt ++;
//	if (cnt > 10) {
//	   double X_axis [64];
//	   for (int i = 0; i < 64; i ++)
//	      X_axis [i] = -32 + i;
//	   LFScope	-> ViewSpectrum (X_axis, LFVector, max, 0,  0);
//	   cnt = 0;
//	}
//}

void    RadioInterface::set_hfscopeLevel (int level) {
        hfScope -> setLevel (level);
}

void	RadioInterface::switch_hfViewMode	(int d) {
	(void)d;
	hfScope	-> switch_viewMode ();
}

void	RadioInterface::updateTime		(void) {
QDateTime currentTime = QDateTime::currentDateTime ();

	timeDisplay     -> setText (currentTime.
                                    toString (QString ("dd.MM.yy:hh:mm:ss")));
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

        QMessageBox::StandardButton resultButton =
                        QMessageBox::question (this, "dabRadio",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
        if (resultButton != QMessageBox::Yes) {
           event -> ignore();
        } else {
           handle_quitButton ();
           event -> accept ();
        }
}

void	RadioInterface::handle_resetButton	() {
	for (int i = 0; i < NR_ELEMENTS; i ++)	{
	   theOutput. updateText (i, i, "");
	}
}

void	RadioInterface::handle_decodeWidth	(int n) {
int lowEnd_new, highEnd_new;

	if (centerBin < n / 2)
	   return;
	if (centerBin >= NR_ELEMENTS - n / 2)
	   return;

	if ((n & 01) != 0) {	// odd
	   lowEnd_new	= centerBin - n / 2;
	   highEnd_new	= centerBin + n / 2;
	}
	else {
	   lowEnd_new	= centerBin - n / 2;
	   highEnd_new	= centerBin + n / 2 - 1;
	}

	if (lowEnd_new > this -> lowEnd)
	   for (int i = this -> lowEnd; i < lowEnd_new; i++)
	      workVector [i] -> reset (0);

	if (highEnd_new < this -> highEnd)
	   for (int i = highEnd_new + 1; i <= this -> highEnd; i ++)
	      workVector [i] -> reset (0);
	this	-> nrBins	= n;
	this	-> lowEnd	= lowEnd_new;
	this	-> highEnd	= highEnd_new;
	settings	-> setValue ("nrBins", nrBins);
}

void	RadioInterface::handle_centerChannel (int n) { 
int	lowEnd_new, highEnd_new;

	if (n < nrBins / 2)
	   return;
	if (n >= NR_ELEMENTS - nrBins / 2)
	   return;

	if ((this -> nrBins & 01) != 0) {	// odd
	   lowEnd_new	= n - this -> nrBins / 2;
	   highEnd_new	= n + this -> nrBins / 2;
	}
	else {
	   lowEnd_new	= n - this -> nrBins / 2;
	   highEnd_new	= n + this -> nrBins / 2 - 1;
	}

	if (lowEnd_new > this -> lowEnd)
	   for (int i = this -> lowEnd; i < lowEnd_new; i++)
	      workVector [i] -> reset (0);

	if (highEnd_new < this -> highEnd)
	   for (int i = highEnd_new + 1; i <= this -> highEnd; i ++)
	      workVector [i] -> reset (0);
	this	-> centerBin	= n;
	this	-> lowEnd	= lowEnd_new;
	this	-> highEnd	= highEnd_new;
	settings	-> setValue ("centerBin", centerBin);
}

deviceHandler	*RadioInterface::getDevice (const QString &s) {
QString file;
bool    success	= true;		// we hope it stays that way

#ifdef HAVE_SDRPLAY
	if (s == D_SDRPLAY) {
	   try {
	      theDevice = new sdrplayHandler_v2 (this,
	                                         &inputBuffer,
	                                         settings, 192000);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_SDRPLAY_V3
	if (s == D_SDRPLAY_V3) {
	   try {
	      theDevice = new sdrplayHandler_v3 (this, 
	                                         &inputBuffer,
	                                         settings, 192000);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_AIRSPY
	if (s == D_AIRSPY) {
	   try {
	      theDevice = new airspyHandler (this, 
	                                     &inputBuffer,
	                                     fmSettings, 192000);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_HACKRF
	if (s == D_HACKRF) {
	   try {
	      theDevice = new hackrfHandler (this,
	                                     &inputBuffer,
	                                     settings, 192000);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
#ifdef HAVE_DABSTICK
	if (s == D_RTLSDR) {
	   try {
	      theDevice = new rtlsdrHandler (this,
	                                     &inputBuffer,
	                                     settings, 192000);
	   } catch (int e) {
	      success = false;
	   }
	}
	else
#endif
	if (s == "filereader") {
	   try {
	      theDevice	= new fileReader (this,
	                                  &inputBuffer,
	                                  settings);
	   } catch (int e) {
	      success = false;
	   }
	}
	else 
	   theDevice	= new deviceHandler ();

	if (!success) {
	   QMessageBox::warning (this, tr ("sdr"),
	                               tr ("loading device failed"));
	   if (theDevice == nullptr)
	      theDevice = new deviceHandler ();	// the empty one
	   return nullptr;
	}

	theDevice -> setVFOFrequency (14070000);
	setStart ();
	settings	-> setValue ("device", s);
	return theDevice;
}
//
deviceHandler	*RadioInterface::setDevice (QSettings *settings) {
deviceSelect	deviceSelect;
deviceHandler	*theDevice	= nullptr;
QStringList devices;

	(void)settings;
	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   devices += deviceTable [i];
	devices	+= "quit";
	deviceSelect. addList (devices);
	int theIndex = -1;
	while (theDevice == nullptr) {
	   theIndex = deviceSelect. QDialog::exec ();
	   if (theIndex < 0)
	      continue;
	   QString s = devices. at (theIndex);
	   if (s == "quit")
	      return nullptr;
	   theDevice	= getDevice (s);
	}
	return theDevice;
}

void	RadioInterface::quickStart () {
	disconnect (starter, SIGNAL (timeout ()),
	            this, SLOT (quickStart ()));
	fprintf (stderr, "going for quickStart\n");
	delete starter;
	if (getDevice (deviceTable [startKnop]) ==  nullptr)
	   if (setDevice (settings) == nullptr)
	      exit (1);
}

