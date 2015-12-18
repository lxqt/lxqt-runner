/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include <LXQt/SingleApplication>
#include <QCommandLineParser>
#include "dialog.h"


int main(int argc, char *argv[])
{
    LXQt::SingleApplication a(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("lxqt-runner"));
    QCoreApplication::setApplicationVersion(LXQT_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("LXQt runner"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption one_shot_run{QStringList() << QLatin1String("o") << QLatin1String("one-shot")
            , QCoreApplication::translate("main", "Show the runner window for one shot (don't run as a daemon).")};

    parser.addOption(one_shot_run);
    parser.process(a);


    QWidget hiddenPreviewParent{0, Qt::Tool};
    const bool one_shot = parser.isSet(one_shot_run);
    Dialog d(one_shot, &hiddenPreviewParent);
    if (!one_shot)
        a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
