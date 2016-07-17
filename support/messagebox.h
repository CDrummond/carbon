/*
 * Cantata
 *
 * Copyright (c) 2011-2016 Craig Drummond <craig.p.drummond@gmail.com>
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

#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include <QMessageBox>

namespace MessageBox {

    extern QMessageBox::StandardButton questionYesNoCancel(QWidget *parent, const QString &message, const QString &title = QString(),
            const QString &yesText = QString(), const QString &noText = QString(), bool showCancel = true, bool isWarning = false);
    inline QMessageBox::StandardButton questionYesNo(QWidget *parent, const QString &message, const QString &title = QString(), const QString &yesText = QString(),
            const QString &noText = QString()) {
        return questionYesNoCancel(parent, message, title, yesText, noText, false);
    }
    inline QMessageBox::StandardButton warningYesNoCancel(QWidget *parent, const QString &message, const QString &title = QString(),
            const QString &yesText = QString(), const QString &noText = QString()) {
        return questionYesNoCancel(parent, message, title, yesText, noText, true, true);
    }
    inline QMessageBox::StandardButton warningYesNo(QWidget *parent, const QString &message, const QString &title = QString(), const QString &yesText = QString(), const QString &noText = QString()) {
        return questionYesNoCancel(parent, message, title, yesText, noText, false, true);
    }
    #ifdef Q_OS_MAC
    extern void error(QWidget *parent, const QString &message, const QString &title = QString());
    extern void information(QWidget *parent, const QString &message, const QString &title = QString());
    #else
    inline void error(QWidget *parent, const QString &message, const QString &title = QString()) {
        QMessageBox::critical(parent, title.isEmpty() ? QObject::tr("Error") : title, message);
    }
    inline void information(QWidget *parent, const QString &message, const QString &title = QString()) {
        QMessageBox::information(parent, title.isEmpty() ? QObject::tr("Information") : title, message);
    }
    #endif
    extern QMessageBox::StandardButton msgListEx(QWidget *parent, QMessageBox::Icon type, const QString &message, const QStringList &strlist, const QString &title = QString());
    inline void errorListEx(QWidget *parent, const QString &message, const QStringList &strlist, const QString &title = QString()) {
        msgListEx(parent, QMessageBox::Critical, message, strlist, title);
    }
    inline void errorList(QWidget *parent, const QString &message, const QStringList &strlist, const QString &title = QString()) {
        msgListEx(parent, QMessageBox::Critical, message, strlist, title);
    }
    inline QMessageBox::StandardButton questionYesNoList(QWidget *parent, const QString &message, const QStringList &strlist, const QString &title = QString()) {
        return msgListEx(parent, QMessageBox::Question, message, strlist, title);
    }
    inline QMessageBox::StandardButton warningYesNoList(QWidget *parent, const QString &message, const QStringList &strlist, const QString &title = QString()) {
        return msgListEx(parent, QMessageBox::Warning, message, strlist, title);
    }
    inline void informationList(QWidget *parent, const QString &message, const QStringList &strlist, const QString &title = QString()) {
        msgListEx(parent, QMessageBox::Information, message, strlist, title);
    }

}

#endif
