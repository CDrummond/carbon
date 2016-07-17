/*
  Carbon (C) Craig Drummond, 2013 craig.p.drummond@gmail.com

  ----

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "utils.h"
#include "mainwindow.h"
#include "messagebox.h"
#include "config.h"
#include <QApplication>
#include <QIcon>
#ifdef QT_QTDBUS_FOUND
#include <QDBusConnection>
#endif
#include <iostream>

static QString fixArg(QString a, const QString &param) {
    a = a.mid(param.length());
    if (a[0] == '\'' || a[0] == '\"') {
        a = a.mid(1);
    }
    if (a.endsWith('\'') || a.endsWith('\"')) {
        a = a.left(a.length() - 1);
    }
    return a;
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    #ifdef QT_QTDBUS_FOUND
    if (!QDBusConnection::sessionBus().registerService("org.craigd.carbon")) {
        return -1;
    }
    #endif

    QString rsync(Utils::findExe("rsync"));

    if (rsync.isEmpty()) {
        MessageBox::error(0L, QObject::tr("Carbon is a GUI front-end for \'rsync\'\nThis tool could not be found on your system."
                                          " Please install this and restart Carbon"));
        return -1;
    } else {
        QCoreApplication::setApplicationName(CARBON_PACKAGE_NAME);
        QCoreApplication::setOrganizationName(CARBON_PACKAGE_NAME);

        MainWindow mw(rsync);
        mw.show();
        return app.exec();
    }
}
