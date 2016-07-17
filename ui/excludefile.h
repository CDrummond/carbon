#ifndef __EXCLUDE_FILE_H__
#define __EXCLUDE_FILE_H__

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

#include <QtCore/QList>
#include <QtCore/QString>

class ExcludeFile {
public:
    struct Pattern {
        Pattern(const QString &v = QString(), const QString &c = QString())
            : value(v), comment(c) { }

        QString toStr();

        QString value;
        QString comment;
    };

    typedef QList<Pattern> PatternList;

    ExcludeFile(const QString &name);

    void load();
    bool save(const QString &n = QString());
    bool erase();
    const PatternList & patterns() const {
        return patternList;
    }
    void setPatterns(const QList<Pattern> &v) {
        patternList = v;
    }

private:
    QString fileName;
    PatternList patternList;
};

#endif
