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
#include "sessiondialog.h"
#include "sessionwidget.h"
#include "generaloptionswidget.h"
#include "excludewidget.h"
#include "rsyncoptionswidget.h"
#include "mainwindow.h"
#include "messagebox.h"
#include "pagewidget.h"
#include "utils.h"
#include <QProcess>

SessionDialog::SessionDialog(QWidget *parent)
    : Dialog(parent, "SessionDialog", QSize(800, 600)) {
    pageWidget = new PageWidget(this);
    setButtons(Ok | Cancel | User1 | User2);
    setButtonText(User1, tr("Set Defaults"));
    setButtonText(User2, tr("Show RSync Manual"));

    generalOptions = new GeneralOptionsWidget(0);
    excludeWidget = new ExcludeWidget(0);
    rSyncOptionsWidget = new RSyncOptionsWidget(0);

    generalPage = pageWidget->addPage(generalOptions, tr("General Options"), QIcon::fromTheme("folder"), tr("Basic Synchronisation Session Options"));
    excludePage = pageWidget->addPage(excludeWidget, tr("Exclusions"), QIcon::fromTheme("edit-delete"), tr("Exclude Files And Folders From Synchronisation"));
    rSyncOptionsPage = pageWidget->addPage(rSyncOptionsWidget, tr("Backend Options"), MainWindow::appIcon, tr("RSync Backend Options"));
    setMainWidget(pageWidget);
}

bool SessionDialog::run(Session &session, bool edit) {
    origName = edit ? session.name() : QString();
    origSrcPath = edit ? session.source() : QString();
    setCaption(edit ? tr("Edit Session") : tr("Create New Session"));
    generalOptions->set(session, edit);
    excludeWidget->set(session);
    rSyncOptionsWidget->set(session);
    pageWidget->setCurrentPage(generalPage);

    return QDialog::Accepted == exec();
}

void SessionDialog::slotButtonClicked(int btn) {
    switch (btn) {
    case Ok: {
        QString newName(generalOptions->name());
        QString newSrc(generalOptions->src());

        if (newName.isEmpty()) {
            pageWidget->setCurrentPage(generalPage);
            MessageBox::error(this, tr("You must specify a name!"));
        } else if (generalOptions->src().isEmpty()) {
            pageWidget->setCurrentPage(generalPage);
            MessageBox::error(this, tr("You must specify a source!"));
        } /*else if (generalOptions->src().protocol()!="file") {
pageWidget->setCurrentPage(generalPage);
MessageBox::error(this, tr("Source must be on the local filesystem."));
}*/ else if (generalOptions->dest().isEmpty()) {
            pageWidget->setCurrentPage(generalPage);
            MessageBox::error(this, tr("You must specify a destination!"));
        } /*else if (generalOptions->dest().protocol()!="file" &&
generalOptions->dest().protocol()!="sftp" &&
generalOptions->dest().protocol()!="fish") {
pageWidget->setCurrentPage(generalPage);
MessageBox::error(this, tr("Only file:/, fish:/, and sftp:/ protocols are allowed."));
} else if ( (generalOptions->dest().protocol()=="sftp" ||
  generalOptions->dest().protocol()=="fish") &&
 (!generalOptions->dest().hasUser() ||
  !generalOptions->dest().hasHost() ) ) {
setCurrentPage(generalPage);
MessageBox::error(this, tr("Remote URLs require user and host."));
}*/ else if (generalOptions->dest().startsWith(generalOptions->src())) {
            pageWidget->setCurrentPage(generalPage);
            MessageBox::error(this, tr("Destination must not be located within source."));
        } else if (newName != origName && ((SessionWidget *)parent())->exists(newName)) {
            pageWidget->setCurrentPage(generalPage);
            MessageBox::error(this, tr("A session named <b>%1</b> already exists.<br/>"
                                       "Please choose a different name.").arg(newName));
        } else if (newSrc != origSrcPath && ((SessionWidget *)parent())->srcExists(newSrc)) {
            pageWidget->setCurrentPage(generalPage);
            MessageBox::error(this, tr("A session with source <b>%1</b> already exists.<br/>"
                                       "Please choose a different path.").arg(newSrc));
        } else if (-1 != rSyncOptionsWidget->getCustom().indexOf("--out-format") ||
                   -1 != rSyncOptionsWidget->getCustom().indexOf("--backup-dir")) {
            pageWidget->setCurrentPage(rSyncOptionsPage);
            MessageBox::error(this, tr("<p>Sorry, custom backend options cannot contain <i>--out-format</i>"
                                       "or <i>--backup-dir</i></p>"));
        } else {
            QDialog::accept();
        }
        break;
    }
    case User1:
        if (QMessageBox::Yes == MessageBox::warningYesNo(this, tr("Use the current session settings as the defaults for new sessions?"))) {
            emit setAsDefaults();
        }
        break;
    case User2:
        if (QLatin1String("KDE") == QLatin1String(qgetenv("XDG_CURRENT_DESKTOP")) && !Utils::findExe("khelpcenter").isEmpty()) {
            QProcess::startDetached("khelpcenter", QStringList() << "man:/rsync");
        } else if (!Utils::findExe("yelp").isEmpty()) {
            QProcess::startDetached("yelp", QStringList() << "info:rsync");
        }
        break;
    default:
        Dialog::slotButtonClicked(btn);
    }
}

void SessionDialog::get(Session &session) {
    excludeWidget->get(session);
    rSyncOptionsWidget->get(session);
    generalOptions->get(session);
}
