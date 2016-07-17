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

#include "excludefile.h"
#include <QTextStream>
#include <QFile>
#include <unistd.h>

ExcludeFile::ExcludeFile(const QString &name)
    : fileName(name) {
    load();
}

void ExcludeFile::load() {
    QFile f(fileName);

    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString comment;

        while (!f.atEnd()) {
            QString line(f.readLine().trimmed());

            if (!line.isEmpty()) {
                if (QChar('#') == line[0] || QChar(';') == line[0]) {
                    comment = line.mid(1).trimmed();
                } else {
                    patternList.append(Pattern(line, comment));
                    comment = QString();
                }
            }
        }
    }
}

bool ExcludeFile::save(const QString &n) {
    QString name = n.isEmpty() ? fileName : n;
    if (0 == patternList.count()) {
        bool rv = erase();
        fileName = name;
        return rv;
    }

    QFile f(name);

    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fs(&f);

        foreach (const Pattern &p, patternList) {
            if (!p.comment.isEmpty()) {
                fs << "# " << p.comment << endl;
            }
            fs << p.value << endl;
        }

        fs.flush();
        f.close();

        if (fileName != name) {
            erase();
        }
        fileName = name;
        return true;
    }

    return false;
}

bool ExcludeFile::erase() {
    return fileName.isEmpty() || !QFile::exists(fileName) || 0 ==::unlink(QFile::encodeName(fileName).constData());
}

QString ExcludeFile::Pattern::toStr() {
    return QString(value + QString(" # ") + comment).trimmed();
}
