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

#include "session.h"
#include "utils.h"
#include "config.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <unistd.h>

#define CFG_READ_BOOL(V, DEF)      V=entries.contains(#V) ? QLatin1String("true")==entries[#V] : DEF
#define CFG_READ_INT(V, DEF)       V=entries.contains(#V) ? entries[#V].toInt() : DEF
#define CFG_READ_STR(V, DEF)       V=entries.contains(#V) ? entries[#V] : DEF
#define CFG_READ_STRING(V, DEF)    V=entries.contains(#V) ? entries[#V] : DEF

#define CFG_WRITE_INT(V)       out << #V << "=" << V << endl
#define CFG_WRITE_BOOL(V)      out << #V << "=" << (V ? "true" : "false") << endl
#define CFG_WRITE_STR(V)       out << #V << "=" << V << endl

static QString getName(const QString &f) {
    return QFileInfo(f).fileName().remove(CARBON_EXTENSION);
}

Session::Session(const QString &file, bool def)
    : isDef(def)
    , exclude(0) {
    if (isDef) {
        dirName = Utils::configDir(QString(), true);
        sessionName = QLatin1String("default");
    } else {
        dirName = Utils::getDir(file);
        sessionName = getName(file);
    }

    QString fName = fileName();

    if (isDef) {
        if (!QFile::exists(fName)) {
            fName = QLatin1String(CARBON_SYSTEM_DEF_FILE);
        }
        QString exFileName = excludeFileName();
        if (!QFile::exists(exFileName)) {
            exFileName = QLatin1String(CARBON_SYSTEM_DEF_FILE CARBON_EXCLUDE_EXTENSION);
        }
        exclude = new ExcludeFile(exFileName);
    } else {
        exclude = new ExcludeFile(excludeFileName());
    }

    QFile f(fName);
    QMap<QString, QString> entries;
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (!line.startsWith("#") && !line.startsWith("[") && line.contains('=')) {
                QStringList parts = line.split("=");
                if (parts.length() > 1) {
                    entries[parts.at(0)] = parts.at(1);
                }
            }
        }
    }

    CFG_READ_STR(src, QDir::homePath());
    CFG_READ_STR(dest, QLatin1String("/tmp"));
    CFG_READ_BOOL(archive, true);
    CFG_READ_BOOL(recursive, true);
    CFG_READ_BOOL(skipFilesOnSizeMatch, false);
    CFG_READ_BOOL(skipReceiverNewerFiles, true);
    CFG_READ_BOOL(keepPartial, false);
    CFG_READ_BOOL(onlyUpdate, false);
    CFG_READ_BOOL(useCompression, false);
    CFG_READ_BOOL(checksum, false);
    CFG_READ_BOOL(windowsCompat, false);
    CFG_READ_BOOL(ignoreExisting, false);
    CFG_READ_BOOL(makeBackups, false);
    CFG_READ_BOOL(deleteExtraFilesOnReceiver, true);
    CFG_READ_BOOL(copySymlinksAsSymlinks, true);
    CFG_READ_BOOL(preservePermissions, true);
    CFG_READ_BOOL(preserveSpecialFiles, true);
    CFG_READ_BOOL(preserveOwner, true);
    CFG_READ_BOOL(dontLeaveFileSystem, false);
    CFG_READ_BOOL(preserveGroup, true);
    CFG_READ_BOOL(modificationTimes, true);
    CFG_READ_BOOL(cvsExclude, true);
    CFG_READ_INT(maxBackupAge, 7);
    CFG_READ_INT(maxFileSize, 0);

    if (archive) {
        copySymlinksAsSymlinks = preservePermissions = preserveSpecialFiles = preserveOwner =
                                     preserveGroup = true;
    }
    if (maxBackupAge > 365) {
        maxBackupAge = 365;
    } else if (maxBackupAge < 0) {
        maxBackupAge = 0;
    }

    if (maxFileSize > 65535) {
        maxFileSize = 65535;
    } else if (maxFileSize < 0) {
        maxFileSize = 0;
    }

    CFG_READ_STRING(customOptions, QString());

    if (!customOptions.isEmpty()) {
        if (QLatin1Char('\"') == customOptions[0]) {
            customOptions = customOptions.mid(1);
        }
        if (!customOptions.isEmpty() && QChar('\"') == customOptions[customOptions.size() - 1]) {
            customOptions = customOptions.left(customOptions.size() - 1);
        }
        if (!customOptions.isEmpty()) {
            customOptions.replace("\\\"", "\"").replace("\\\'", "\'");
        }
    }

    updateLast();
}

Session::Session()
    : isDef(false)
    , exclude(0L) {
}

Session::~Session() {
    delete exclude;
}

void Session::updateLast() {
    QFile log(logFileName());

    lastSyncDate = !log.exists()
                   ? QString()
                   : QFileInfo(log).created().toString(Qt::SystemLocaleShortDate);
}

bool Session::save(const QString &name) {
    QString fName = dirName + (isDef ? sessionName : name) + QLatin1String(CARBON_EXTENSION);
    QFile f(fName);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    if (!isDef && name != sessionName) {
        removeFiles();
        sessionName = name;
    }

    QTextStream out(&f);

    out << "[Settings]" << endl;
    CFG_WRITE_STR(src);
    CFG_WRITE_STR(dest);
    CFG_WRITE_BOOL(archive);
    CFG_WRITE_BOOL(recursive);
    CFG_WRITE_BOOL(skipFilesOnSizeMatch);
    CFG_WRITE_BOOL(skipReceiverNewerFiles);
    CFG_WRITE_BOOL(keepPartial);
    CFG_WRITE_BOOL(onlyUpdate);
    CFG_WRITE_BOOL(useCompression);
    CFG_WRITE_BOOL(checksum);
    CFG_WRITE_BOOL(windowsCompat);
    CFG_WRITE_BOOL(ignoreExisting);
    CFG_WRITE_BOOL(makeBackups);
    CFG_WRITE_BOOL(deleteExtraFilesOnReceiver);
    CFG_WRITE_BOOL(copySymlinksAsSymlinks);
    CFG_WRITE_BOOL(preservePermissions);
    CFG_WRITE_BOOL(preserveSpecialFiles);
    CFG_WRITE_BOOL(preserveOwner);
    CFG_WRITE_BOOL(dontLeaveFileSystem);
    CFG_WRITE_BOOL(preserveGroup);
    CFG_WRITE_BOOL(modificationTimes);
    CFG_WRITE_BOOL(cvsExclude);
    CFG_WRITE_INT(maxBackupAge);
    CFG_WRITE_INT(maxFileSize);

    QString temp = customOptions;
    if (!temp.isEmpty()) {
        temp = QChar('\"') + temp.replace('\"', "\\\"").replace('\'', "\\\'") + QChar('\"');
    }
    out << "customOptions=" << temp << endl;

    if (exclude) {
        exclude->save(excludeFileName());
    }

    return true;
}

bool Session::erase() {
    bool rv(removeFiles());

    if (rv) {
        src = QString(); // Stop destructor from saving file!
    }

    return rv;
}

bool Session::sync(bool dryRun) {
    if (dryRun) {
        return true; // "add -d"
    }
    return false;
}

void Session::setName(const QString &v) {
    if (v != sessionName && removeFiles()) {
        QString oldName(sessionName),
                oldDir(dirName);

        dirName = Utils::dataDir(QString(), true);
        if (save(v)) {
            lastSyncDate = QString();
            sessionName = v;
        } else {
            sessionName = oldName;
            dirName = oldDir;
            save();
        }
    }
}

void Session::setExcludePatterns(const ExcludeFile::PatternList &list) {
    if (!exclude) {
        exclude = new ExcludeFile(excludeFileName());
    }

    exclude->setPatterns(list);
}

static bool removeFile(const QString &file) {
    return file.isEmpty() || !QFile::exists(file) || QFile::remove(file);
}

bool Session::removeLockFile() {
    return removeFile(lockFileName());
}

bool Session::removeFiles() {
    if (removeFile(logFileName()) && removeFile(infoFileName()) && removeFile(lockFileName()) &&
            (!exclude || exclude->erase()) && removeFile(fileName())) {
        return true;
    }

    return false;
}
