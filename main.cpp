#
/*
 *    Copyright (C) 2020
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
 *	Main program
 */
#include	<QApplication>
#include	<QSettings>
#include	<QDir>
#include	<unistd.h>
#include	"radio.h"
#include	"radio-constants.h"

QString fullPathfor (QString v) {
QString fileName;

	if (v == QString (""))
	   return QString ("/tmp/xxx");

	if (v. at (0) == QChar ('/'))           // full path specified
	   return v;

	fileName = QDir::homePath ();
	fileName. append ("/");
	fileName. append (v);
	fileName = QDir::toNativeSeparators (fileName);

	if (!fileName. endsWith (".ini"))
	   fileName. append (".ini");

	return fileName;
}

#define	DEFAULT_INI	".cw_skimmer.ini"

int	main (int argc, char **argv) {
int32_t		opt;
/*
 *	The default values
 */
RadioInterface	*MyRadioInterface;
QString iniFile		= QDir::homePath ();

	QCoreApplication::setOrganizationName ("Lazy Chair Computing");
        QCoreApplication::setOrganizationDomain ("Lazy Chair Computing");
        QCoreApplication::setApplicationName ("sw-radio");
        QCoreApplication::setApplicationVersion (QString (CURRENT_VERSION) + " Git: " + GITHASH);

        iniFile. append ("/");
        iniFile. append (DEFAULT_INI);
        iniFile = QDir::toNativeSeparators (iniFile);

        while ((opt = getopt (argc, argv, "i:B:")) != -1) {
           switch (opt) {
              case 'i':
                 iniFile	= fullPathfor (QString (optarg));
                 break;

             default:
                 break;
           }
        }

#if QT_VERSION >= 0x050600
        QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);
#endif
	QApplication a (argc, argv);

	QSettings ISettings (iniFile, QSettings::IniFormat);
/*
 *	Before we connect control to the gui, we have to
 *	instantiate
 */
        MyRadioInterface = new RadioInterface (&ISettings);
	MyRadioInterface -> show ();
        a. exec ();
/*
 *	done:
 */
	fflush (stdout);
	fflush (stderr);
	qDebug ("It is done\n");
	ISettings. sync ();
	fprintf (stderr, "we gaan deleten\n");
	delete MyRadioInterface;
}
