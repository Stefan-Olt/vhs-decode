/************************************************************************

    main.cpp

    ld-analyse - TBC output analysis
    Copyright (C) 2018-2020 Simon Inns

    This file is part of ld-decode-tools.

    ld-analyse is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QtGlobal>
#include <QCommandLineParser>

#include "logging.h"

int main(int argc, char *argv[])
{
    // Install the local debug message handler
    setDebug(true);
    qInstallMessageHandler(debugOutputHandler);

    QApplication a(argc, argv);

    // Set application name and version
    QCoreApplication::setApplicationName("ld-analyse");
    QCoreApplication::setApplicationVersion("1.1");
    QCoreApplication::setOrganizationDomain("domesday86.com");

    // Set up the command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "ld-analyse - TBC output analysis\n"
        "\n"
        "(c)2018-2020 Simon Inns\n"
        "GPLv3 Open-Source - github: https://github.com/happycube/ld-decode");
    parser.addHelpOption();
    parser.addVersionOption();

    // Option to show debug (-d)
    QCommandLineOption showDebugOption(QStringList() << "d" << "debug",
                                       QCoreApplication::translate("main", "Show debug"));
    parser.addOption(showDebugOption);

    // Positional argument to specify input video file
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Specify input TBC file"));

    // Process the command line arguments given by the user
    parser.process(a);

    // Get the configured settings from the parser
    bool isDebugOn = parser.isSet(showDebugOption);

    // Process the command line options
    if (isDebugOn) setDebug(true); else setDebug(false);

    // Get the arguments from the parser
    QString inputFileName;
    QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.count() == 1) {
        inputFileName = positionalArguments.at(0);
    } else {
        inputFileName.clear();
    }

    // Start the GUI application
    MainWindow w(inputFileName);
    w.show();

    return a.exec();
}
