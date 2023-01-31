#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the  cwSkimmer
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__OUTPUT_LIST_H
#define	__OUTPUT_LIST_H

#include	<QWidget>
#include	<QScrollArea>
#include	<QTableWidget>
#include	<QStringList>
#include	<QTableWidgetItem>
#include	<QObject>
#include	<QString>

class	RadioInterface;

class outputList:public QObject {
Q_OBJECT
public:
		outputList	(RadioInterface *, QString);
		~outputList	(void);
	void	addRow		();
	void	setFrequency	(int, int);
	void	updateText	(int, int, const QString &);
	void	show		(void);
	void	hide		(void);
	void	saveTable	(void);
	QScrollArea		*myWidget;
	QTableWidget	*tableWidget;
	QString		saveName;
};

#endif

