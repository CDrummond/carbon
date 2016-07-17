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

#include "messagebox.h"
#include "dialog.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>

#ifdef Q_OS_MAC
static void splitMessage(const QString &orig, QString &msg, QString &sub) {
    static QStringList constSeps = QStringList() << QLatin1String("\n\n") << QLatin1String("<br/><br/>");
    msg = orig;

    foreach (const QString &sep, constSeps) {
        QStringList parts = orig.split(sep);
        if (parts.count() > 1) {
            msg = parts.takeAt(0);
            sub = parts.join(sep);
            return;
        }
    }
}

void MessageBox::error(QWidget *parent, const QString &message, const QString &title) {
    QString msg;
    QString sub;
    splitMessage(message, msg, sub);
    QMessageBox box(QMessageBox::Critical, title.isEmpty() ? i18n("Error") : title, msg, QMessageBox::Ok, parent, Qt::Sheet);
    box.setInformativeText(sub);
    box.exec();
}

void MessageBox::information(QWidget *parent, const QString &message, const QString &title) {
    QString msg;
    QString sub;
    splitMessage(message, msg, sub);
    QMessageBox box(QMessageBox::Information, title.isEmpty() ? i18n("Information") : title, msg, QMessageBox::Ok, parent, Qt::Sheet);
    box.setInformativeText(sub);
    box.exec();
}
#endif

QMessageBox::StandardButton MessageBox::questionYesNoCancel(QWidget *parent, const QString &message, const QString &title,
        const QString &yesText, const QString &noText, bool showCancel, bool isWarning) {
    QMessageBox msg(parent);
    #ifdef Q_OS_MAC
    QString mainText;
    QString subText;
    splitMessage(message, mainText, subText);
    msg.setText(mainText);
    msg.setInformativeText(subText);
    #else
    msg.setText(message);
    #endif
    msg.setIcon(isWarning ? QMessageBox::Warning : QMessageBox::Question);
    msg.setWindowTitle(title.isEmpty() ? (isWarning ? QObject::tr("Warning") : QObject::tr("Question")) : title);
    QPushButton *yesBtn = msg.addButton(yesText.isEmpty() ? QObject::tr("Yes") : yesText, QMessageBox::YesRole);
    QPushButton *noBtn = msg.addButton(noText.isEmpty() ? QObject::tr("No") : noText, QMessageBox::NoRole);
    if (showCancel) {
        msg.addButton(QMessageBox::Cancel);
    }
    msg.exec();

    QAbstractButton *clicked = msg.clickedButton();
    return yesBtn == clicked
           ? QMessageBox::Yes
           : noBtn == clicked
           ? QMessageBox::No
           : QMessageBox::Cancel;
}

namespace MessageBox {
    class YesNoListDialog : public Dialog {
    public:
        YesNoListDialog(QWidget *p) : Dialog(p) { }
        void slotButtonClicked(int b) {
            switch (b) {
            case Dialog::Ok:
            case Dialog::Yes:
                accept();
                break;
            case Dialog::No:
                reject();
                break;
            }
        }
    };
}

QMessageBox::StandardButton MessageBox::msgListEx(QWidget *parent, QMessageBox::Icon type, const QString &message, const QStringList &strlist, const QString &title) {
    MessageBox::YesNoListDialog *dlg = new MessageBox::YesNoListDialog(parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    QWidget *wid = new QWidget(dlg);
    QGridLayout *lay = new QGridLayout(wid);
    QLabel *iconLabel = new QLabel(wid);
    int iconSize = 64; // Icon::dlgIconSize();
    iconLabel->setFixedSize(iconSize, iconSize);
    switch (type) {
    case QMessageBox::Critical:
        dlg->setCaption(title.isEmpty() ? QObject::tr("Error") : title);
        dlg->setButtons(Dialog::Ok);
        iconLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(iconSize, iconSize));
        break;
    case QMessageBox::Question:
        dlg->setCaption(title.isEmpty() ? QObject::tr("Question") : title);
        dlg->setButtons(Dialog::Yes | Dialog::No);
        iconLabel->setPixmap(QIcon::fromTheme("dialog-question").pixmap(iconSize, iconSize));
        break;
    case QMessageBox::Warning:
        dlg->setCaption(title.isEmpty() ? QObject::tr("Warning") : title);
        dlg->setButtons(Dialog::Yes | Dialog::No);
        iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(iconSize, iconSize));
        break;
    case QMessageBox::Information:
        dlg->setCaption(title.isEmpty() ? QObject::tr("Information") : title);
        dlg->setButtons(Dialog::Ok);
        iconLabel->setPixmap(QIcon::fromTheme("dialog-information").pixmap(iconSize, iconSize));
        break;
    }
    lay->addWidget(iconLabel, 0, 0, 1, 1);
    QLabel *msgLabel = new QLabel(message, wid);
    msgLabel->setWordWrap(true);
    msgLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    lay->addWidget(msgLabel, 0, 1, 1, 1);
    QListWidget *list = new QListWidget(wid);
    lay->addWidget(list, 1, 0, 1, 2);
    lay->setMargin(0);
    list->insertItems(0, strlist);
    dlg->setMainWidget(wid);
    #ifdef Q_OS_MAC
    dlg->setWindowFlags((dlg->windowFlags() & (~Qt::WindowType_Mask)) | Qt::Sheet);
    #endif
    return QDialog::Accepted == dlg->exec() ? QMessageBox::Yes : QMessageBox::No;
}
