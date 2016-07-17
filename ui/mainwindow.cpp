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

#include "mainwindow.h"
#include "sessionwidget.h"
#include "config.h"
#include <QMenu>
#include <QToolBar>
#include <QSettings>
#include <QMessageBox>
#include <unistd.h>
#include <sys/types.h>

#define CFG_GROUP "MainWindow/"
#define CFG_SIZE  CFG_GROUP "Size"

QIcon MainWindow::appIcon;

MainWindow::MainWindow(const QString &rsync)
    : QMainWindow(0L)
    , rsync(rsync) {
    bool root = 0 == getuid();

    QList<int> appSizes = QList<int>() << 16 << 22 << 32 << 48;
    foreach (int s, appSizes) {
        appIcon.addFile(QString(":carbon%1%2.png").arg(root ? "-root" : "").arg(s), QSize(s, s));
    }
    appIcon.addFile(root ? ":carbon-root.svg" : ":carbon.svg");
    setWindowIcon(appIcon);

    setWindowTitle(root ? tr("Carbon (root)") : tr("Carbon"));
    sessionWidget = new SessionWidget(this);
    setCentralWidget(sessionWidget);
    setupActions();
    createGUI();

    QSettings cfg;
    resize(cfg.value(CFG_SIZE, QSize(600, 300)).toSize());
    setMinimumSize(600, 200);

    if (root) {
        sessionWidget->setBackground(appIcon);
    }
}

MainWindow::~MainWindow() {
    QSettings cfg;
    cfg.setValue(CFG_SIZE, size());
}

void MainWindow::setupActions() {
    quitAction = new QAction(QIcon::fromTheme("application-exit"), tr("Quit"), this);
    connect(quitAction, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    quitAction->setToolTip(tr("Quit application."));

    aboutAction = new QAction(QIcon::fromTheme("dialog-information"), tr("About"), this);
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(about()));

    newSessionAction = new QAction(QIcon::fromTheme("document-new"), tr("New..."), this);
    connect(newSessionAction, SIGNAL(triggered(bool)), sessionWidget, SLOT(newSession()));
    newSessionAction->setToolTip(tr("Create a new synchronisation session (either a plain synchronisation, or a backup)."));

    editSessionAction = new QAction(QIcon::fromTheme("document-properties"), tr("Edit..."), this);
    connect(editSessionAction, SIGNAL(triggered(bool)), sessionWidget, SLOT(editSession()));
    editSessionAction->setToolTip(tr("Change synchronisation session properties."));

    deleteSessionAction = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete..."), this);
    connect(deleteSessionAction, SIGNAL(triggered(bool)), sessionWidget, SLOT(removeSession()));

    QIcon icon = QIcon::fromTheme("view-history");
    if (icon.isNull()) {
        icon = QIcon::fromTheme("document");
    }
    showLogAction = new QAction(icon, tr("Show Last Log..."), this);
    connect(showLogAction, SIGNAL(triggered(bool)), sessionWidget, SLOT(showSessionLog()));

    dryRunAction = new QAction(QIcon::fromTheme("system-run"), tr("Dry-run..."), this);
    connect(dryRunAction, SIGNAL(triggered(bool)), sessionWidget, SLOT(dryRunSession()));
    dryRunAction->setToolTip(tr("Show what would happen with a sync, but don't actually do anything."));

    syncAction = new QAction(appIcon, tr("Synchronise..."), this);
    connect(syncAction, SIGNAL(triggered(bool)), sessionWidget, SLOT(syncSession()));
    syncAction->setToolTip(tr("Perform synchronisation."));

    //    restoreAction=ActionCollection::get()->createAction("restore", tr("Restore..."), QIcon::fromTheme("edit-undo"));
    //    restoreAction->setCheckable(true);
    //    connect(restoreAction, SIGNAL(toggled(bool)), SLOT(restoreSession(bool)));

    deleteSessionAction->setEnabled(false);
    editSessionAction->setEnabled(false);
    showLogAction->setEnabled(false);
    dryRunAction->setEnabled(false);
    syncAction->setEnabled(false);

    connect(sessionWidget, SIGNAL(itemsSelected(bool)), deleteSessionAction, SLOT(setEnabled(bool)));
    connect(sessionWidget, SIGNAL(singleItemSelected(bool)), editSessionAction, SLOT(setEnabled(bool)));
    connect(sessionWidget, SIGNAL(haveLog(bool)), showLogAction, SLOT(setEnabled(bool)));
    connect(sessionWidget, SIGNAL(haveSessions(bool)), dryRunAction, SLOT(setEnabled(bool)));
    connect(sessionWidget, SIGNAL(haveSessions(bool)), syncAction, SLOT(setEnabled(bool)));
    //    connect(sessionWidget, SIGNAL(haveSessions(bool)), restoreAction, SLOT(setEnabled(bool)));

    QMenu *menu = new QMenu(sessionWidget);
    menu->addAction(deleteSessionAction);
    menu->addAction(editSessionAction);
    menu->addSeparator();
    menu->addAction(showLogAction);
    menu->addSeparator();
    menu->addAction(dryRunAction);
    menu->addAction(syncAction);
    sessionWidget->setMenu(menu);
}

class Spacer : public QWidget {
public:
    Spacer(QWidget *p)
        : QWidget(p) {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    }
};

void MainWindow::createGUI() {
    QToolBar *tb = new QToolBar(this);
    tb->addAction(newSessionAction);
    tb->addAction(editSessionAction);
    tb->addAction(deleteSessionAction);
    //    tb->addSeparator();
    //    tb->addAction(showLogAction);
    tb->addSeparator();
    tb->addAction(dryRunAction);
    tb->addAction(syncAction);
    //    tb->addAction(restoreAction);
    tb->addWidget(new Spacer(tb));
    tb->addAction(aboutAction);
    tb->addAction(quitAction);
    tb->setMovable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tb->toggleViewAction()->setVisible(false);
    addToolBar(tb);
}

void MainWindow::restoreSession(bool on) {
    newSessionAction->setEnabled(!on);
    if (on) {
        editSessionAction->setEnabled(false);
        deleteSessionAction->setEnabled(false);
        showLogAction->setEnabled(false);
        dryRunAction->setEnabled(false);
        syncAction->setEnabled(false);
    }

    sessionWidget->setEnabled(!on);
    sessionWidget->restoreSession(on);
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About Carbon"),
                       tr("<b>Carbon %1</b><br/><br/>rsync front-end.<br/><br/>"
                          "(C) Craig Drummond 2013.<br/>Released under the <a href=\"http://www.gnu.org/licenses/gpl.html\">GPLv3</a>").arg(CARBON_VERSION));

}
