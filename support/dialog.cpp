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

#include "dialog.h"
#ifdef Q_OS_MAC
#include "osxstyle.h"
#endif
#include <QIcon>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QSettings>
#include <QStyle>

Dialog::Dialog(QWidget *parent, const QString &name, const QSize &defSize)
    : QDialog(parent)
    , defButton(0)
    , buttonTypes(0)
    , mw(0)
    , buttonBox(0)
    , shown(false) {
    if (!name.isEmpty()) {
        setObjectName(name);
        QSettings cfg;
        cfg.beginGroup(metaObject()->className());
        cfgSize = cfg.value(name + "/size", QSize()).toSize();
        if (!cfgSize.isEmpty()) {
            QDialog::resize(cfgSize);
        } else if (!defSize.isEmpty()) {
            QDialog::resize(defSize);
        }
        cfg.endGroup();
    }

    #ifdef Q_OS_MAC
    setWindowIcon(QIcon());
    #endif
}

Dialog::~Dialog() {
    if (!objectName().isEmpty() && size() != cfgSize) {
        QSettings cfg;
        cfg.beginGroup(metaObject()->className());
        cfg.setValue(objectName() + "/size", size());
        cfg.endGroup();
    }
    #ifdef Q_OS_MAC
    OSXStyle::self()->removeWindow(this);
    #endif
}

void Dialog::resize(const QSize &sz) {
    if (cfgSize.isEmpty()) {
        QDialog::resize(sz);
        cfgSize = sz;
    }
    QDialog::resize(sz);
}

#ifdef Q_OS_MAC
Dialog::ButtonProxyStyle::ButtonProxyStyle()
    : QProxyStyle() {
    setBaseStyle(qApp->style());
}

int Dialog::ButtonProxyStyle::styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget, QStyleHintReturn *returnData) const {
    if (QStyle::SH_DialogButtonLayout == stylehint) {
        return QDialogButtonBox::GnomeLayout;
    } else {
        return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
    }
}

Dialog::ButtonProxyStyle * Dialog::buttonProxyStyle() {
    static ButtonProxyStyle *style = 0;
    if (!style) {
        style = new ButtonProxyStyle();
    }
    return style;
}
#endif

static QDialogButtonBox::StandardButton mapType(int btn) {
    switch (btn) {
    case Dialog::Help:   return QDialogButtonBox::Help;
    case Dialog::Ok:     return QDialogButtonBox::Ok;
    case Dialog::Apply:  return QDialogButtonBox::Apply;
    case Dialog::Cancel: return QDialogButtonBox::Cancel;
    case Dialog::Close:  return QDialogButtonBox::Close;
    case Dialog::No:     return QDialogButtonBox::No;
    case Dialog::Yes:    return QDialogButtonBox::Yes;
    case Dialog::Reset:  return QDialogButtonBox::Reset;
    default:             return QDialogButtonBox::NoButton;
    }
}

void Dialog::setButtons(ButtonCodes buttons) {
    if (buttonBox && buttons == buttonTypes) {
        return;
    }

    QFlags<QDialogButtonBox::StandardButton> btns;
    if (buttons & Help) {
        btns |= QDialogButtonBox::Help;
    }
    if (buttons & Ok) {
        btns |= QDialogButtonBox::Ok;
    }
    if (buttons & Apply) {
        btns |= QDialogButtonBox::Apply;
    }
    if (buttons & Cancel) {
        btns |= QDialogButtonBox::Cancel;
    }
    if (buttons & Close) {
        btns |= QDialogButtonBox::Close;
    }
    if (buttons & No) {
        btns |= QDialogButtonBox::No;
    }
    if (buttons & Yes) {
        btns |= QDialogButtonBox::Yes;
    }
    if (buttons & Reset) {
        btns |= QDialogButtonBox::Reset;
    }

    buttonTypes = (int)btns;
    bool needToCreate = true;
    if (buttonBox) {
        needToCreate = false;
        buttonBox->clear();
        buttonBox->setStandardButtons(btns);
        userButtons.clear();
    } else {
        buttonBox = new QDialogButtonBox(btns, Qt::Horizontal, this);
        #ifdef Q_OS_MAC
        buttonBox->setStyle(buttonProxyStyle());
        #endif
    }

    if (buttons & Help) {
        setButtonText(Help, tr("Help"));
    }
    if (buttons & Ok) {
        setButtonText(Ok, tr("OK"));
    }
    if (buttons & Apply) {
        setButtonText(Apply, tr("Apply"));
    }
    if (buttons & Cancel) {
        setButtonText(Cancel, tr("Cancel"));
    }
    if (buttons & Close) {
        setButtonText(Close, tr("Close"));
    }
    if (buttons & No) {
        setButtonText(No, tr("No"));
    }
    if (buttons & Yes) {
        setButtonText(Yes, tr("Yes"));
    }
    if (buttons & Reset) {
        setButtonText(Reset, tr("Reset"));
    }

    if (buttons & User3) {
        QPushButton *button = new QPushButton(buttonBox);
        userButtons.insert(User3, button);
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);
    }
    if (buttons & User2) {
        QPushButton *button = new QPushButton(buttonBox);
        userButtons.insert(User2, button);
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);
    }
    if (buttons & User1) {
        QPushButton *button = new QPushButton(buttonBox);
        userButtons.insert(User1, button);
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);
    }

    if (needToCreate && mw && buttonBox) {
        create();
    }
}

void Dialog::setDefaultButton(ButtonCode button) {
    QAbstractButton *b = getButton(button);
    if (b) {
        qobject_cast<QPushButton *>(b)->setDefault(true);
    }
    defButton = button;
}

void Dialog::setButtonText(ButtonCode button, const QString &text) {
    QAbstractButton *b = getButton(button);
    if (b) {
        b->setText(text);
    }
}

void Dialog::setButtonIcon(ButtonCode button, const QIcon &icon) {
    QAbstractButton *b = getButton(button);
    if (b) {
        b->setIcon(icon);
    }
}

void Dialog::setButtonMenu(ButtonCode button, QMenu *menu) {
    QAbstractButton *b = getButton(button);
    if (b) {
        qobject_cast<QPushButton *>(b)->setMenu(menu);
    }
}

void Dialog::enableButton(ButtonCode button, bool enable) {
    QAbstractButton *b = getButton(button);
    if (b) {
        b->setEnabled(enable);
    }
}

bool Dialog::isButtonEnabled(ButtonCode button) {
    QAbstractButton *b = getButton(button);
    return b ? b->isEnabled() : false;
}

void Dialog::setMainWidget(QWidget *widget) {
    if (mw) {
        return;
    }
    mw = widget;
    if (mw && buttonBox) {
        create();
    }
}

void Dialog::slotButtonClicked(int button) {
    switch (button) {
    case Ok: accept(); break;
    case Cancel: reject(); break;
    case Close: reject(); break;
    default: break;
    }
}

void Dialog::buttonPressed(QAbstractButton *button) {
    if (buttonTypes & QDialogButtonBox::Help && button == buttonBox->button(QDialogButtonBox::Help)) {
        slotButtonClicked(Help);
    } else if (buttonTypes & QDialogButtonBox::Ok && button == buttonBox->button(QDialogButtonBox::Ok)) {
        slotButtonClicked(Ok);
    } else if (buttonTypes & QDialogButtonBox::Apply && button == buttonBox->button(QDialogButtonBox::Apply)) {
        slotButtonClicked(Apply);
    } else if (buttonTypes & QDialogButtonBox::Cancel && button == buttonBox->button(QDialogButtonBox::Cancel)) {
        slotButtonClicked(Cancel);
    } else if (buttonTypes & QDialogButtonBox::Close && button == buttonBox->button(QDialogButtonBox::Close)) {
        slotButtonClicked(Close);
    } else if (buttonTypes & QDialogButtonBox::No && button == buttonBox->button(QDialogButtonBox::No)) {
        slotButtonClicked(No);
    } else if (buttonTypes & QDialogButtonBox::Yes && button == buttonBox->button(QDialogButtonBox::Yes)) {
        slotButtonClicked(Yes);
    } else if (buttonTypes & QDialogButtonBox::Reset && button == buttonBox->button(QDialogButtonBox::Reset)) {
        slotButtonClicked(Reset);
    } else if (userButtons.contains(User1) && userButtons[User1] == button) {
        slotButtonClicked(User1);
    } else if (userButtons.contains(User2) && userButtons[User2] == button) {
        slotButtonClicked(User2);
    } else if (userButtons.contains(User3) && userButtons[User3] == button) {
        slotButtonClicked(User3);
    }
}

void Dialog::create() {
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->addWidget(mw);
    layout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonPressed(QAbstractButton *)));
}

QAbstractButton *Dialog::getButton(ButtonCode button) {
    QDialogButtonBox::StandardButton mapped = mapType(button);
    QAbstractButton *b = QDialogButtonBox::NoButton == mapped ? 0 : buttonBox->button(mapped);
    if (!b && userButtons.contains(button)) {
        b = userButtons[button];
    }
    return b;
}

void Dialog::showEvent(QShowEvent *e) {
    if (!shown) {
        shown = true;
        if (defButton) {
            setDefaultButton((ButtonCode)defButton);
        }
        if (buttonBox && mw) {
            QSize mwSize = mw->minimumSize();
            if (mwSize.width() < 16 || mwSize.height() < 16) {
                mwSize = mw->minimumSizeHint();
            }
            if (mwSize.width() > 15 && mwSize.height() > 15) {
                setMinimumHeight(qMax(minimumHeight(), buttonBox->height() + layout()->spacing() + mwSize.height() + (2 * layout()->margin())));
                setMinimumWidth(qMax(minimumWidth(), mwSize.width() + (2 * layout()->margin())));
            }
        }
    }
    #ifdef Q_OS_MAC
    if (!isModal()) {
        OSXStyle::self()->addWindow(this);
    }
    #endif
    QDialog::showEvent(e);
}

#ifdef Q_OS_MAC
void Dialog::hideEvent(QHideEvent *e) {
    OSXStyle::self()->removeWindow(this);
    QDialog::hideEvent(e);
}

void Dialog::closeEvent(QCloseEvent *e) {
    OSXStyle::self()->removeWindow(this);
    QDialog::closeEvent(e);
}
#endif
