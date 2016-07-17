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

#include "generaloptionswidget.h"
#include "session.h"

GeneralOptionsWidget::GeneralOptionsWidget(QWidget *parent, bool showName)
    : QWidget(parent) {
    setupUi(this);
    if (!showName) {
        nameEdit->setVisible(false);
        nameLabel->setVisible(false);
    }
    srcPath->setDirMode(true);
    destPath->setDirMode(true);
    type->insertItem(0, QObject::tr("Synchronisation"));
    type->insertItem(1, QObject::tr("Backup"));
    connect(type, SIGNAL(activated(int)), SLOT(typeChanged(int)));
}

void GeneralOptionsWidget::set(const Session &session, bool edit) {
    type->setEnabled(!edit);
    nameEdit->setText(session.isDefault() ? QObject::tr("New Session") : session.name());
    srcPath->setText(Utils::convertDirForDisplay(session.source()));
    destPath->setText(Utils::convertDirForDisplay(session.destination()));
    type->setCurrentIndex(session.makeBackupsFlag() ? 1 : 0);
    typeChanged(type->currentIndex());
    if (session.maxBackupDays()) {
        maxDays->setValue(session.maxBackupDays());
        deleteOld->setChecked(true);
    } else {
        maxDays->setValue(7);
        dontDeleteOld->setChecked(true);
    }
}

void GeneralOptionsWidget::get(Session &session) {
    session.setSource(src());
    session.setDestination(dest());
    session.setMakeBackupsFlag(type->currentIndex() ? true : false);
    session.setMaxBackupDays(deleteOld->isChecked() ? maxDays->value() : 0);
    session.setName(name());
}

void GeneralOptionsWidget::typeChanged(int idx) {
    ageWidget->setVisible(1 == idx);
}
