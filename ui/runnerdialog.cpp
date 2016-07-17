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

#include "runnerdialog.h"
#include "session.h"
#include "utils.h"
#include "messagebox.h"
#include <QProcess>
#include <QTimer>
#include <QTextStream>
#ifdef QT_QTDBUS_FOUND
#include <QDBusConnection>
#include <unistd.h>
#include <sys/types.h>
#endif

static inline QString removePrefixes(const QString &str) {
    return QString(str).replace(CARBON_PREFIX, QString()).replace(CARBON_MSG_PREFIX, QString()).replace(CARBON_ERROR_PREFIX, QString());
}

static QString updatedFiles(int v) {
    return 0 == v ? QObject::tr("Checking for updated files...") : QObject::tr("Checking for updated files...%1").arg(v);
}

static QString toStr(int exitCode) {
    switch (exitCode) {
    case 101:
        return QObject::tr("Usage error.");
    case 102:
        return QObject::tr("Could not read session file.");
    case 103:
        return QObject::tr("Destination folder does not exist, and could not be created.");
    case 104:
        return QObject::tr("Destination exists as a file.");
    case 105:
        return QObject::tr("Failed to mount destination folder.");
    case 106:
        return QObject::tr("Failed to unmount destination folder.");
    case 107:
        return QObject::tr("Mountpoint already exists.");
    case 108:
        return QObject::tr("Destination requires user@host");
    case 109:
        return QObject::tr("No source supplied.");
    case 110:
        return QObject::tr("No destination supplied.");
    case 111:
        return QObject::tr("Destination parent folder does not exist.");
    case 112:
        return QObject::tr("Source does not exist.");
    case 113:
        return QObject::tr("Session is already running.");
    case 1:
        return QObject::tr("Syntax or usage error.");
    case 2:
        return QObject::tr("Protocol incompatibility.");
    case 3:
        return QObject::tr("Errors selecting input/output files/folders.");
    case 4:
        return QObject::tr("Requested action not supported: an attempt "
                           "was made to manipulate 64-bit files on a "
                           "platform that cannot support them; or an "
                           "option was specified that is supported by "
                           "the client and not by the server.");
    case 5:
        return QObject::tr("Error starting client-server protocol.");
    case 6:
        return QObject::tr("Daemon unable to append to log-file.");
    case 10:
        return QObject::tr("Error in socket I/O.");
    case 11:
        return QObject::tr("Error in file I/O.");
    case 12:
        return QObject::tr("Error in rsync protocol data stream.");
    case 13:
        return QObject::tr("Errors with program diagnostics.");
    case 14:
        return QObject::tr("Error in IPC code.");
    case 20:
        return QObject::tr("Received SIGUSR1 or SIGINT.");
    case 21:
        return QObject::tr("Some error returned by waitpid().");
    case 22:
        return QObject::tr("Error allocating core memory buffers.");
    case 23:
        return QObject::tr("Partial transfer due to error.");
    case 24:
        return QObject::tr("Partial transfer due to vanished source files.");
    case 25:
        return QObject::tr("The --max-delete limit stopped deletions.");
    case 30:
        return QObject::tr("Timeout in data send/receive.");
    case 35:
        return QObject::tr("Timeout waiting for daemon connection.");
    default:
        return QObject::tr("Unknown error code %1.").arg(exitCode);
    }

    return QString();
}

RunnerDialog::RunnerDialog(QWidget *parent)
    : Dialog(parent)
    , process(0) {
    QWidget *mainWidget = new QWidget(this);

    setupUi(mainWidget);
    setMainWidget(mainWidget);
    setButtons(Cancel);
    output->setReadOnly(true);
    output->setVisible(false);
    setMinimumWidth(500);
    connect(detailsButton, SIGNAL(toggled(bool)), this, SLOT(showDetails(bool)));
    #ifdef QT_QTDBUS_FOUND
    unityMessage = QDBusMessage::createSignal("/Carbon", "com.canonical.Unity.LauncherEntry", "Update");
    #endif
}

RunnerDialog::~RunnerDialog() {
    disconnectProcess();
}

void RunnerDialog::go(const QList<Session *> &sessions, bool dry) {
    setWindowTitle(dry ? tr("Performing Dry-Run") : tr("Performing Synchronisation"));

    completedSessions = 0;
    overallProgress->setVisible(sessions.count() > 1);
    overallProgressLabel->setVisible(sessions.count() > 1);
    sessionLabel->setText(QString());
    status->setText(QString());
    fileProgress->setValue(0);
    overallProgress->setValue(0);
    overallProgress->setMaximum(sessions.count() * 1000);
    sessionProgress->setValue(0);
    sessionProgress->setMaximum(0);
    detailsButton->setChecked(false);
    dryRun = dry;

    stdErr = prevStout = QString();
    output->setText(QString());
    currentSession = sessions.begin();
    endSession = sessions.end();
    sessionCount = sessions.count();
    if (!process) {
        process = new QProcess(this);
        QStringList env(QProcess::systemEnvironment());
        env.append(CARBON_GUI_PARENT"=true");
        process->setEnvironment(env);
        connect(process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
        connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
        connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    }
    setButtons(Cancel);
    QTimer::singleShot(0, this, SLOT(doNext()));
    QTimer::singleShot(0, this, SLOT(showDetails()));
    exec();
}

void RunnerDialog::doNext() {
    syncStatus = STARTUP;
    if (currentSession != endSession) {
        sessionLabel->setText((*currentSession)->name());

        QStringList arguments;

        (*currentSession)->save();
        arguments << (*currentSession)->fileName();

        if (dryRun) {
            arguments << "-d";
        }

        stdErr = QString();
        prevStout = QString();
        sessionProgress->setValue(0);
        sessionProgress->setMaximum(0);
        updateUnity(false);
        status->setText(updatedFiles(0));
        fileProgress->setValue(0);
        logFile.close();
        logFile.setFileName((*currentSession)->fileName() + CARBON_LOG_EXTENSION);
        logFile.open(QIODevice::WriteOnly);
        process->start(CARBON_RUNNER, arguments, QIODevice::ReadOnly);
    } else {
        fileProgress->setValue(fileProgress->maximum());
        setButtons(Close);
    }
}

void RunnerDialog::processFinished(int exitCode) {
    logFile.close();

    if (0 != exitCode) {
        QString errorMsg = tr("<p>The <i>rsync</i> backend returned the following error:</p><p><i>%1</i></p>").arg(toStr(exitCode));
        status->setText(tr("An error ocurred"));
        if (currentSession == endSession) {
            MessageBox::error(this, errorMsg);
            return;
        } else if (QMessageBox::No == MessageBox::warningYesNo(this, errorMsg + "<p>" + tr("Continue with next session?") + "</p>")) {
            return;
        }
    }

    completedSessions++;
    sessionProgress->setMaximum(1000);
    sessionProgress->setValue(sessionProgress->maximum());
    overallProgress->setValue(completedSessions * 1000);
    updateUnity(currentSession == endSession);
    if (currentSession == endSession) {
        status->setText(tr("Cancelled"));
    } else {
        if (0 == exitCode) {
            status->setText(tr("Finished"));
        }
        currentSession++;
        doNext();
    }
}

void RunnerDialog::readStdOut() {
    if (!process) {
        return;
    }

    QByteArray all(process->readAllStandardOutput());
    QString raw(prevStout + all);
    QString str(raw.replace('\r', '\n'));
    bool doLast(str.size() && '\n' == str[str.size() - 1]);
    QStringList lines(str.split('\n', QString::SkipEmptyParts));
    QStringList::Iterator it(lines.begin());
    QStringList::Iterator end(lines.end());
    int numLines(lines.count());

    for (int l = 1; it != end; ++it, ++l) {
        if (l == numLines) {
            if (doLast) {
                processLine(*it);
            } else {
                prevStout = *it;
            }
        } else {
            processLine(*it);
        }
    }
    str = removePrefixes(all);
    output->append(str);
    QTextStream(&logFile) << str;
}

void RunnerDialog::readStdErr() {
    if (!process) {
        return;
    }

    QString str(removePrefixes(process->readAllStandardError()));

    stdErr += str;
    output->append("<b>" + str + "</b>");
    QTextStream(&logFile) << str;
}

void RunnerDialog::showDetails(bool show) {
    int w = width();
    if (show) {
        setMaximumHeight(4096);
    }

    detailsButton->setText(show ? tr("Hide Details <<") : tr("Show Details >>"));
    output->setVisible(show);
    adjustSize();
    resize(width()<w ? w : width(), show ? height() : minimumHeight());
    if (!show) {
        setMaximumHeight(minimumHeight());
    }
}

void RunnerDialog::processLine(QString &line) {
    bool isSynk(0 == line.indexOf(CARBON_PREFIX)),
         isMsg(!isSynk && 0 == line.indexOf(CARBON_MSG_PREFIX)),
         isError(!isSynk && !isMsg && 0 == line.indexOf(CARBON_ERROR_PREFIX));

    if (isSynk) {
        syncStatus = SYNCING;
        line.replace(CARBON_PREFIX, QString());
        status->setText(line);
        fileProgress->setValue(0);
    } else if (isMsg) {
        line.replace(CARBON_MSG_PREFIX, QString())
        .replace("Cleaning old backups", tr("Cleaning old backups"))
        .replace("Erasing", tr("Erasing"));

        status->setText(line);
        syncStatus = SYNCING;
    } else if (isError) {
        line.replace(CARBON_ERROR_PREFIX, QString())
        .replace("Cleaning old backups", tr("Cleaning old backups"))
        .replace("Erasing", tr("Erasing"));

        status->setText(QString("<b>") + line + QString("</b>"));
        syncStatus = SYNCING;
    } /*else if (STARTUP==syncStatus && -1!=line.indexOf("files to consider")) {
        QStringList lst(line.split(' ', QString::SkipEmptyParts));

        if (lst.size())
        {
            bool ok;
            int total(lst[0].toInt(&ok));

            if (ok)
            {
                sessionProgress->setMaximum(total);
                syncStatus=SYNCING;
            }
        }
    }*/ else if (STARTUP == syncStatus && -1 != line.indexOf(" files...")) {
        QStringList lst(line.split(' ', QString::SkipEmptyParts));

        if (lst.size()) {
            bool ok;
            int  total(lst[0].toInt(&ok));

            if (ok) {
                status->setText(updatedFiles(total));
            }
        }
    } else if (-1 != line.indexOf("%")) {
        QStringList lst(line.split(' ', QString::SkipEmptyParts));

        if (lst.size() > 2) {
            int percent;

            if (1 == sscanf(lst[1].toLatin1().constData(), "%d%%", &percent)) {
                fileProgress->setValue(percent);
                syncStatus = SYNCING;
            }
            static const QString constGlobalCheck = QLatin1String("to-check=");
            foreach (const QString &str, lst) {
                if (str.startsWith(constGlobalCheck)) {
                    lst = str.mid(constGlobalCheck.length()).split('/');
                    if (lst.size() >= 2) {
                        QString totStr = lst.at(1);
                        totStr = totStr.left(totStr.length() - 1);
                        int total = totStr.toInt();
                        int left = lst.at(0).toInt();
                        if (0 == sessionProgress->maximum()) {
                            sessionProgress->setMaximum(1000);
                        }

                        if (total > 0 && left <= total) {
                            if (0 == left) {
                                sessionProgress->setValue(sessionProgress->maximum());
                            } else {
                                sessionProgress->setValue((((total - left) * 1000.0) / total) + 0.5);
                            }
                            overallProgress->setValue((1000 * completedSessions) + (sessionProgress->value()));
                            updateUnity(false);
                        }
                    }
                    break;
                }
            }
        }
    }
}

void RunnerDialog::slotButtonClicked(int btn) {
    if (Dialog::Cancel == btn && process && QProcess::NotRunning != process->state()) {
        switch (MessageBox::warningYesNoCancel(this, tr("Abort the current synchronisation?"),
                                               tr("Abort"), tr("Abort Now"),
                                               tr("Abort After Current Sync"))) {
        case QMessageBox::Yes:
            QProcess::startDetached(CARBON_TERMINATE, QStringList() << QString::number(process->pid()));
            for (int i = 0; i < 50 && QProcess::Running == process->state(); ++i) {
                Utils::msleep(10);
            }
            process->kill();
            disconnectProcess();
            if (currentSession != endSession) {
                (*currentSession)->removeLockFile();
            }
            updateUnity(true);
            QDialog::reject();
            break;
        case QMessageBox::No:
            currentSession = endSession;
        default:
            break;
        }
    } else {
        Dialog::slotButtonClicked(btn);
    }
}

void RunnerDialog::disconnectProcess() {
    if (process) {
        disconnect(process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
        disconnect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
        disconnect(process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
        process->deleteLater();
        process = 0;
    }
}

void RunnerDialog::updateUnity(bool finished) {
    #ifdef QT_QTDBUS_FOUND
    QList<QVariant> args;
    double progress = finished || overallProgress->maximum() < 1 ? 0.0 : (overallProgress->value() / (overallProgress->maximum() * 1.0));
    bool showProgress = progress > -0.1 && progress < 100 && !finished;
    QMap<QString, QVariant> props;
    props["count-visible"] = !finished && sessionCount > 1;
    props["count"] = (long long)(sessionCount - completedSessions);
    props["progress-visible"] = showProgress;
    props["progress"] = showProgress ? progress : 0.0;
    args.append(0 == getuid() ? "application://carbon-root.desktop" : "application://carbon.desktop");
    args.append(props);
    unityMessage.setArguments(args);
    QDBusConnection::sessionBus().send(unityMessage);
    #else
    Q_UNUSED(finished)
    #endif
}
