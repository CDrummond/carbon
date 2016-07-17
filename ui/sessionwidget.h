#ifndef __SESSION_WIDGET_H__
#define __SESSION_WIDGET_H__

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

#include "ui_sessionwidget.h"
#include "session.h"

class QContextMenuEvent;
class QMenu;
class SessionDialog;
class RunnerDialog;
class CLogViewer;
class CTHost;
class QIcon;

class SessionWidget : public QWidget, Ui::SessionWidget {
    Q_OBJECT

public:
    SessionWidget(QWidget *parent);
    virtual ~SessionWidget();

    const Session & defaultSession() const {
        return defSession;
    }

    bool exists(const QString &name) const;
    bool srcExists(const QString &path) const;
    void setMenu(QMenu *m) {
        menu = m;
    }
    void setBackground(const QIcon &icon);

Q_SIGNALS:
    void singleItemSelected(bool);
    void itemsSelected(bool);
    void haveSessions(bool);
    void haveLog(bool);

public Q_SLOTS:
    void newSession();
    void editSession();
    void removeSession();
    void showSessionLog();
    void dryRunSession();
    void syncSession();
    void restoreSession(bool on);
    void controlButtons();
    void setAsDefaults();
    void loadSessions();

private:
    void contextMenuEvent(QContextMenuEvent *e);
    void controlSyncButtons();
    void setupWidgets();
    void doSessions(bool dryRun);
    void initCronTab();
    void createSessionDialog();
    QList<QTreeWidgetItem *> selectedSessions();

private:
    Session       defSession;
    SessionDialog *sessionDialog;
    RunnerDialog  *runnerDialog;
    CLogViewer    *logViewer;
    CTHost        *ctHost;
    QMenu         *menu;
};

#endif
