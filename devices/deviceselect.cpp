#
/*
 *    Copyright (C) 2014 .. 2017
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
 *
 */
#include	<cstdio>
#include	<QVBoxLayout>
#include	"deviceselect.h"
//
//	Whenever there are two or more sdrplay devices connected
//	to the computer, the user is asked to make a choice.

	deviceSelect::deviceSelect	():
	                        selectorDisplay () {
	toptext		= new QLabel (this);
	toptext		-> setText ("Select a device");
	QVBoxLayout	*layOut = new QVBoxLayout;
	layOut		-> addWidget (toptext);
	layOut		-> addWidget (&selectorDisplay);
	setWindowTitle (tr("device select"));
	setLayout (layOut);

	Devices		= QStringList ();
	deviceList. setStringList (Devices);
	selectorDisplay. setModel (&deviceList);
	connect (&selectorDisplay, SIGNAL (clicked (QModelIndex)),
	         this, SLOT (select_device (QModelIndex)));
	selectedItem	= -1;
}

	deviceSelect::~deviceSelect () {
}

void	deviceSelect::addtoList (const char *v) {
QString s (v);

	Devices << s;
	deviceList. setStringList (Devices);
	selectorDisplay. setModel (&deviceList);
	selectorDisplay. adjustSize();
	adjustSize();
}

void	deviceSelect::addList (const QStringList &s) {

	deviceList. setStringList (s);
	selectorDisplay. setModel (&deviceList);
	selectorDisplay. adjustSize ();
	adjustSize();
}

void	deviceSelect::select_device (QModelIndex ind) {
	QDialog::done (ind. row());
}

