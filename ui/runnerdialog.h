#ifndef __RUNNER_DIALOG_H__
#define __RUNNER_DIALOG_H__

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

#include "dialog.h"
#include "config.h"
#include <QList>
#include <QFile>
#ifdef QT_QTDBUS_FOUND
#include <QDBusMessage>
#endif
#include "ui_runnerwidget.h"

class Session;
class QProcess;


class RunnerDialog : public Dialog, Ui::RunnerWidget {
    Q_OBJECT

public:
    enum EStatus {
        STARTUP,
        SYNCING
    };

    RunnerDialog(QWidget *parent);
    virtual ~RunnerDialog();

    void go(const QList<Session *> &sessions, bool dry);

public Q_SLOTS:
    void doNext();
    void processFinished(int exitCode);
    void readStdOut();
    void readStdErr();
    void showDetails(bool show = false);

private:
    void processLine(QString &line);
    void slotButtonClicked(int btn);
    void disconnectProcess();
    void updateUnity(bool finished);

private:
    QList<Session *>::ConstIterator currentSession;
    QList<Session *>::ConstIterator endSession;
    QString prevStout;
    QString stdErr;
    bool dryRun;
    QProcess *process;
    QFile logFile;
    EStatus syncStatus;
    int sessionCount;
    int completedSessions;
    #ifdef QT_QTDBUS_FOUND
    QDBusMessage unityMessage;
    #endif
};

#endif
