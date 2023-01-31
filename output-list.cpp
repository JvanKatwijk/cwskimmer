#
/*
 *    Copyright (C) 2014
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
 *    cw skimmerr is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cw skimmer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"output-list.h"
#include	"radio.h"
#include	<QFile>
#include	<QDataStream>
#include	<QHeaderView>

	outputList::outputList (RadioInterface *mr,
	                          QString saveName) {
	this	-> saveName	= saveName;
	myWidget	= new QScrollArea (NULL);
	myWidget	-> resize (400, 200);
	myWidget	-> setWidgetResizable(true);

	tableWidget 	= new QTableWidget (0, 3);
	myWidget	-> setWidget (tableWidget);
	tableWidget 	-> setHorizontalHeaderLabels (
	            QStringList () << tr ("frequency") << tr ("word count") << tr ("up to 73 characters of received text"));
}

	outputList::~outputList () {
int16_t	i;
int16_t	rows	= tableWidget -> rowCount ();

	for (i = rows; i > 0; i --)
	   tableWidget -> removeRow (i);
	delete	tableWidget;
	delete	myWidget;
}

void	outputList::show	() {
	myWidget	-> show ();
}

void	outputList::hide	() {
	myWidget	-> hide ();
}

void	outputList::addRow () {
int16_t	row	= tableWidget -> rowCount ();

	tableWidget	-> insertRow (row);
	QTableWidgetItem *item0	= new QTableWidgetItem;
	item0		-> setTextAlignment (Qt::AlignRight |Qt::AlignVCenter);
	item0		-> setText (" ");
	tableWidget	-> setItem (row, 0, item0);
	QTableWidgetItem *item1 = new QTableWidgetItem;
	item1		-> setTextAlignment(Qt::AlignRight);
	item1		-> setText (" ");
	tableWidget	-> setItem (row, 1, item1);
	QTableWidgetItem *item2 = new QTableWidgetItem;
	item2		-> setTextAlignment(Qt::AlignLeft);
	item2		-> setText (" ");
	tableWidget	-> setItem (row, 2, item2);
	tableWidget	-> setColumnWidth (2, 300);
	tableWidget	-> horizontalHeader () -> setStretchLastSection (true);
}
//

void	outputList::setFrequency (int index, int frequency) {
QString s = QString::number (frequency);
	if (index > 0)
	   index --;
	else
	   return;
	tableWidget	-> item (index, 0) -> setText (s);
}

void	outputList::updateText (int index, int count, const QString &s) {
	if (index > 0)
	   index --;
	else
	   return;
	tableWidget	-> item (index, 1) -> setText (QString::number (count));
	tableWidget	-> item (index, 2) -> setText (s);
}

void	outputList::saveTable (void) {
QFile	file (saveName);

	if (file. open (QIODevice::WriteOnly)) {
	   QDataStream stream (&file);
	   int32_t	n = tableWidget -> rowCount ();
	   int32_t	m = tableWidget -> columnCount ();
	   stream << n << m;

	   for (int i = 0; i < n; i ++) 
	      for (int j = 0; j < m; j ++) 
	         tableWidget -> item (i, j) -> write (stream);
	   file. close ();
	}
}

