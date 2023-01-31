#
/*
 *    Copyright (C)  2018
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the cwskimmer
 *
 *    cwskimmer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    cwskimmer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cwskimmer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__CW_SKIMMER_H
#define	__CW_SKIMMER_H

#include        <QMainWindow>
#include        <QTimer>
#include        <QWheelEvent>
#include        <QLineEdit>
#include	<QTimer>
#include	<sndfile.h>
#include        "ui_newradio.h"
#include        "radio-constants.h"
#include        "ringbuffer.h"
//#include	"shifter.h"
#include	"output-list.h"

class		deviceHandler;
class           QSettings;
class           fftScope;
class		SpectrumViewer;
class           fft_scope;
class		keyPad;

class	RadioInterface:public QMainWindow,
	               private Ui_MainWindow {
Q_OBJECT
public:
		RadioInterface (QSettings	*sI,
	                        QWidget		*parent = NULL);
		~RadioInterface	();

	int32_t	get_selectedFrequency	();
	int32_t	get_centerFrequency	();
private:
        RingBuffer<std::complex<float> > inputBuffer;
	outputList	theOutput;
	std::complex<float> *fftVector;
	int32_t		centerFrequency;
	int32_t		selectedFrequency;
	QSettings       *settings;
        int32_t         inputRate;
	int32_t		scopeWidth;
        int32_t         audioRate;
        int16_t         displaySize;
        int16_t         spectrumSize;
        double          *displayBuffer;
        fftScope        *hfScope;
	int16_t		mouseIncrement;
	int16_t		delayCount;

	SpectrumViewer	*LFScope;
	void		processBuffer	(std::complex<float> *, int);
	void		update_work	(int, float, float);

	float		signalValue;

	int		centerBin;
	int		nrBins;
	int		lowEnd;
	int		highEnd;
	QTimer		secondsTimer;
	keyPad		*mykeyPad;
	deviceHandler	*theDevice;
	void		setStart	();
	deviceHandler   *getDevice      (const QString &);
        deviceHandler   *setDevice      (QSettings *);

private slots:
        void            adjustFrequency_hz	(int);
        void            adjustFrequency_khz	(int);
        void            set_hfscopeLevel	(int);
	void		setFrequency		(int32_t);
        void            handle_freqButton       ();
        void            wheelEvent              (QWheelEvent *);
	void		set_mouseIncrement	(int);
	void		handle_quitButton	();
	void		switch_hfViewMode	(int);
	void		updateTime		();
	void		closeEvent		(QCloseEvent *event);
	void		handle_resetButton	();
	void		handle_centerChannel	(int);
	void		handle_decodeWidth	(int);
public slots:
	void		quickStart		();
	void		sampleHandler		(int amount);
	void		updateText		(int, int, const QString &);
};

#endif
