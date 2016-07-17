/*
 * Cantata
 *
 * Copyright (c) 2011-2014 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <sys/types.h>
#include <qglobal.h>
#include <stdlib.h>
#include <QString>
#include <QLatin1Char>
#include <QDir>
#include <QPainterPath>

class QString;
class QWidget;

namespace Utils {
    extern QLatin1Char constDirSep;
    extern QLatin1String constDirSepStr;
    extern const char * constDirSepCharStr;

    inline bool equal(double d1, double d2, double precision = 0.0001) {
        return (fabs(d1 - d2) < precision);
    }
    inline int random(int max) {
        return ::random() % max;
    }

    extern QString strippedText(QString s);
    extern QString stripAcceleratorMarkers(QString label);
    extern QString fixPath(const QString &d);
    // Convert directory to a format suitable fo rUI - e.g. use native separators, and remove any traling separator
    extern QString convertDirForDisplay(const QString &dir);
    // Convert directory from a UI field - convert to / separators, and add a trailing separator
    extern QString convertDirFromDisplay(const QString &dir);
    extern QString getDir(const QString &file);
    extern QString getFile(const QString &file);
    extern QString changeExtension(const QString &file, const QString &extension);
    extern bool makeDir(const QString &dir, int mode);
    extern void msleep(int msecs);
    inline void sleep() {
        msleep(100);
    }

    extern QString findExe(const QString &appname, const QString &pathstr = QString());
    extern QString formatByteSize(double size);
    extern QString formatDuration(const quint32 totalseconds);
    extern QString cleanPath(const QString &p);
    extern QString configDir(const QString &sub = QString(), bool create = false);
    extern QString dataDir(const QString &sub = QString(), bool create = false);
    extern QString cacheDir(const QString &sub = QString(), bool create = true);
    extern QString systemDir(const QString &sub);
    extern bool moveFile(const QString &from, const QString &to);
    extern void moveDir(const QString &from, const QString &to);
    extern void clearOldCache(const QString &sub, int maxAge);
    extern void touchFile(const QString &fileName);
    extern bool isDirReadable(const QString &dir);

    extern int layoutSpacing(QWidget *w);
    extern bool isHighDpi();
    extern QPainterPath buildPath(const QRectF &r, double radius);

    enum Desktop {
        KDE,
        Gnome,
        Unity,
        Other
    };
    extern Desktop currentDe();
}

#endif
