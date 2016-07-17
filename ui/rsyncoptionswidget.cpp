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

#include "rsyncoptionswidget.h"
#include "session.h"
#include <QCheckBox>
#include <unistd.h>
#include <sys/types.h>

RSyncOptionsWidget::RSyncOptionsWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi(this);

    //    if (0!=getuid()) {
    //        preserveSpecialFiles->setEnabled(false);
    //        preserveOwner->setEnabled(false);
    //        preserveGroup->setEnabled(false);
    //    }

    connect(archive, SIGNAL(toggled(bool)), SLOT(archiveToggled(bool)));
    connect(copySymlinksAsSymlinks, SIGNAL(toggled(bool)), SLOT(preserveToggled(bool)));
    connect(preservePermissions, SIGNAL(toggled(bool)), SLOT(preserveToggled(bool)));
    connect(preserveSpecialFiles, SIGNAL(toggled(bool)), SLOT(preserveToggled(bool)));
    connect(preserveOwner, SIGNAL(toggled(bool)), SLOT(preserveToggled(bool)));
    connect(preserveGroup, SIGNAL(toggled(bool)), SLOT(preserveToggled(bool)));
}

void RSyncOptionsWidget::set(const Session &session) {
    archive->setChecked(session.archiveFlag());
    recursive->setChecked(session.recursiveFlag());
    skipFilesOnSizeMatch->setChecked(session.skipFilesOnSizeMatchFlag());
    skipReceiverNewerFiles->setChecked(session.skipReceiverNewerFilesFlag());
    keepPartial->setChecked(session.keepPartialFlag());
    onlyUpdate->setChecked(session.onlyUpdateFlag());
    useCompression->setChecked(session.useCompressionFlag());
    checksum->setChecked(session.checksumFlag());
    windowsCompatability->setChecked(session.windowsFlag());
    ignoreExisting->setChecked(session.ignoreExistingFlag());
    deleteExtraFilesOnReceiver->setChecked(session.deleteExtraFilesOnReceiverFlag());
    copySymlinksAsSymlinks->setChecked(session.copySymlinksAsSymlinksFlag());
    preservePermissions->setChecked(session.preservePermissionsFlag());
    preserveSpecialFiles->setChecked(session.preserveSpecialFilesFlag());
    preserveOwner->setChecked(session.preserveOwnerFlag());
    dontLeaveFileSystem->setChecked(session.dontLeaveFileSystemFlag());
    preserveGroup->setChecked(session.preserveGroupFlag());
    modificationTimes->setChecked(session.modificationTimesFlag());
    customOptions->setText(session.customOpts());
}

void RSyncOptionsWidget::get(Session &session) {
    session.setArchiveFlag(archive->isChecked());
    session.setRecursiveFlag(recursive->isChecked());
    session.setSkipFilesOnSizeMatchFlag(skipFilesOnSizeMatch->isChecked());
    session.setSkipReceiverNewerFilesFlag(skipReceiverNewerFiles->isChecked());
    session.setKeepPartialFlag(keepPartial->isChecked());
    session.setOnlyUpdateFlag(onlyUpdate->isChecked());
    session.setUseCompressionFlag(useCompression->isChecked());
    session.setChecksumFlag(checksum->isChecked());
    session.setWindowsFlag(windowsCompatability->isChecked());
    session.setIgnoreExistingFlag(ignoreExisting->isChecked());
    session.setDeleteExtraFilesOnReceiverFlag(deleteExtraFilesOnReceiver->isChecked());
    session.setCopySymlinksAsSymlinksFlag(copySymlinksAsSymlinks->isChecked());
    session.setPreservePermissionsFlag(preservePermissions->isChecked());
    session.setPreserveSpecialFilesFlag(preserveSpecialFiles->isChecked());
    session.setPreserveOwnerFlag(preserveOwner->isChecked());
    session.setDontLeaveFileSystemFlag(dontLeaveFileSystem->isChecked());
    session.setPreserveGroupFlag(preserveGroup->isChecked());
    session.setModificationTimesFlag(modificationTimes->isChecked());
    session.setCustomOpts(customOptions->text().trimmed());
}

void RSyncOptionsWidget::preserveToggled(bool on) {
    if (!on) {
        archive->setChecked(false);
    }
}

void RSyncOptionsWidget::archiveToggled(bool on) {
    if (on) {
        recursive->setChecked(true);
        copySymlinksAsSymlinks->setChecked(true);
        preservePermissions->setChecked(true);
        preserveSpecialFiles->setChecked(true);
        preserveOwner->setChecked(true);
        preserveGroup->setChecked(true);
    }
}
