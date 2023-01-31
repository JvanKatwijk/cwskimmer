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
 */
#
#ifndef	__SDRPLAY_SELECT_H
#define	__SDRPLAY_SELECT_H
#
#include	<QDialog>
#include	<QLabel>
#include	<QListView>
#include	<QStringListModel>
#include	<QStringList>
#include	<stdint.h>

class	sdrplaySelect: public QDialog {
Q_OBJECT
public:
			sdrplaySelect	();
			~sdrplaySelect	();
	void		addtoList	(const char *);
private:
	QLabel		*toptext;
	QListView	*selectorDisplay;
	QStringListModel deviceList;
	QStringList	Devices;
	int16_t		selectedItem;
private slots:
	void		select_rsp	(QModelIndex);
};

#endif

