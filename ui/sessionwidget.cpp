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

#include "sessionwidget.h"
#include "sessiondialog.h"
#include "session.h"
#include "runnerdialog.h"
#include "config.h"
#include "messagebox.h"
#include "utils.h"
#include <QTextEdit>
#include <QMenu>
#include <QTimer>
#include <QContextMenuEvent>
#include <QDir>
#include <QSettings>
#include <QHeaderView>
#include <QFileInfo>
#include "cthost.h"
#include "ctcron.h"
#include "ctInitializationError.h"

#define CFG_GROUP     "SessionWidget/"
#define CFG_COL_SIZES CFG_GROUP "List"

class CLogViewer : public Dialog {
public:

    CLogViewer(QWidget *parent)
        : Dialog(parent) {
        edit = new QTextEdit(this);
        edit->setReadOnly(true);
        setMainWidget(edit);
        setButtons(Close);
    }

    void show(const Session &session) {
        QFile file(session.logFileName());

        edit->setText(file.open(QIODevice::ReadOnly)
                      ? file.readAll()
                      : tr("<b>Could not open log file.</b>"));
        file.close();
        setCaption(tr("%1 Log").arg(session.name()));
        resize(800, 500);
        QDialog::show();
    }

private:

    QTextEdit *edit;
};

enum EColumns {
    COL_NAME,
    COL_TYPE,
    COL_LAST_RUN,
    COL_SCHED,

    NUM_COLS
};

static QString typeStr(bool backup) {
    return backup ? QObject::tr("Backup") : QObject::tr("Synchronisation");
}

class SessionWidgetItem : public QTreeWidgetItem {
public:
    SessionWidgetItem(QTreeWidget *parent, const QString &s)
        : QTreeWidgetItem(parent)
        , session(new Session(s)) {
        update();
    }

    SessionWidgetItem(QTreeWidget *parent, Session *s)
        : QTreeWidgetItem(parent)
        , session(s) {
        update();
        session->save();
    }

    void update() {
        setText(COL_NAME, session->name());
        setText(COL_TYPE, typeStr(session->makeBackupsFlag()));
        setText(COL_LAST_RUN, session->last().isEmpty() ? QObject::tr("Never") : session->last());
        setText(COL_SCHED, session->scheduleStr());
        setToolTip();
    }

    void setSchedule(const QString &str) {
        setText(COL_SCHED, str);
        setToolTip();
    }

    void setLastRun(const QString &str) {
        setText(COL_LAST_RUN, str);
        setToolTip();
    }

    operator bool() {
        return session;
    }

    Session & sessionData() {
        return *session;
    }

    void remove() {
        if (session->erase()) {
            delete session;
            delete this;
        }
    }

private:
    void setToolTip() {
        QString tip(QObject::tr("<p><h3>%1</h3></p><p>"
                                "<table>"
                                "<tr><td>Source:</td><td>%2</td></tr>"
                                "<tr><td>Destination:</td><td>%3</td></tr>")
                    .arg(text(COL_NAME))
                    .arg(session->source())
                    .arg(session->destination()));

        for (int i = 0; i < NUM_COLS; ++i) {
            QTreeWidgetItem::setToolTip(i, tip);
        }
    }

private:
    Session *session;
};

static QStringList toNames(QList<QTreeWidgetItem *> &items) {
    QStringList                             names;
    QList<QTreeWidgetItem *>::ConstIterator it(items.constBegin());
    QList<QTreeWidgetItem *>::ConstIterator end(items.constEnd());

    for (; it != end; ++it)
        names.append(((SessionWidgetItem *)(*it))->sessionData().name());

    return names;
}

SessionWidget::SessionWidget(QWidget *parent)
    : QWidget(parent)
    , defSession(tr("New Session"), true)
    , sessionDialog(0L)
    , runnerDialog(0L)
    , logViewer(0L)
    , menu(0L) {
    setupUi(this);
    setupWidgets();
    initCronTab();

    QTimer::singleShot(0, this, SLOT(loadSessions()));
}

SessionWidget::~SessionWidget() {
    QStringList list;
    for (int i = 0; i < NUM_COLS; ++i) {
        list << QString::number(sessions->header()->sectionSize(i));
    }

    QSettings cfg;
    cfg.setValue(CFG_COL_SIZES, list);
}

bool SessionWidget::exists(const QString &name) const {
    return 0 != sessions->findItems(name, Qt::MatchExactly).count();
}

bool SessionWidget::srcExists(const QString &path) const {
    for (int i = 0; i < sessions->topLevelItemCount(); ++i) {
        SessionWidgetItem *item = static_cast<SessionWidgetItem *>(sessions->topLevelItem(i));
        if (item->sessionData().source() == path) {
            return true;
        }
    }
    return false;
}

void SessionWidget::setBackground(const QIcon &icon) {
    sessions->setBackground(icon);
}

void SessionWidget::setupWidgets() {
    controlSyncButtons();
    controlButtons();
    connect(sessions, SIGNAL(itemSelectionChanged()), SLOT(controlButtons()));
}

void SessionWidget::newSession() {
    createSessionDialog();

    if (sessionDialog->run(defSession, false)) {
        Session *session = new Session;
        CTCron   *cron = ctHost->findCurrentUserCron();
        CTTask   *sched = new CTTask("", "", cron->userLogin(), cron->isMultiUserCron());
        bool     addedSched(false);

        sessionDialog->get(*session, sched);

        SessionWidgetItem *item = new SessionWidgetItem(sessions, session);

        if (*item) {
            if (Session::CronSchedule == session->schedType()) {
                addedSched = true;
                item->sessionData().setSched(sched);
                item->setSchedule(item->sessionData().scheduleStr());
            }
        } else {
            delete item;
        }

        if (addedSched) {
            cron->addTask(sched);

            CTSaveStatus st(ctHost->save());

            if (st.isError()) {
                MessageBox::error(this, tr("<p><b>Failed to save advanced schedule data</b></p><p>%1</p>")
                                  .arg(st.detailErrorMessage()));
                item->sessionData().setSchedType(Session::NoSchedule);
            }
        } else {
            delete sched;
        }

        controlButtons();
        controlSyncButtons();
    }
}

void SessionWidget::editSession() {
    QList<QTreeWidgetItem *> sessionList(sessions->selectedItems());

    if (1 == sessionList.count()) {
        createSessionDialog();

        SessionWidgetItem *session = (SessionWidgetItem *)sessionList.at(0);

        if (sessionDialog->run(session->sessionData(), true)) {
            CTCron *cron = ctHost->findCurrentUserCron();
            CTTask *prevSched = session->sessionData().sched(),
                    *sched = prevSched
                             ? prevSched
                             : new CTTask("", "", cron->userLogin(), cron->isMultiUserCron());

            sessionDialog->get(session->sessionData(), sched);

            bool isCron(Session::CronSchedule == session->sessionData().schedType()),
                 saveCron(false);

            if (prevSched && !isCron) {
                session->sessionData().setSched(0L);
                cron->removeTask(prevSched);
                saveCron = true;
            } else if (!prevSched && isCron) {
                session->sessionData().setSched(sched);
                cron->addTask(sched);
                saveCron = true;

            }

            if (saveCron) {
                CTSaveStatus st(ctHost->save());

                if (st.isError()) {
                    MessageBox::error(this, tr("<p><b>Failed to save advanced schedule data</b></p><p>%1</p>")
                                      .arg(st.detailErrorMessage()));
                    if (isCron)
                        session->sessionData().setSchedType(Session::NoSchedule);
                }
            }
            session->sessionData().save();
            session->update();
        }
    }
}

void SessionWidget::removeSession() {
    QList<QTreeWidgetItem *> sessionList(sessions->selectedItems());
    QStringList              names(toNames(sessionList));

    if ((sessionList.count() == 1 && QMessageBox::Yes == MessageBox::warningYesNo(this, tr("Delete <b>%1</b>?").arg(*(names.begin())))) ||
            (sessionList.count() > 1 && QMessageBox::Yes == MessageBox::warningYesNoList(this, tr("Delete the following sessions?"), names))) {
        QList<QTreeWidgetItem *>::Iterator it(sessionList.begin());
        QList<QTreeWidgetItem *>::Iterator end(sessionList.end());
        CTCron *cron = ctHost->findCurrentUserCron();
        bool modifiedCron(false);

        for (; it != end; ++it) {
            SessionWidgetItem *item = (SessionWidgetItem *)(*it);

            if (item->sessionData().sched()) {
                cron->removeTask(item->sessionData().sched());
                delete item->sessionData().sched();
                item->sessionData().setSchedType(Session::NoSchedule);
                modifiedCron = true;
            }
            item->remove();
        }

        if (modifiedCron) {
            ctHost->save();
        }
        controlButtons();
        controlSyncButtons();
    }
}

void SessionWidget::showSessionLog() {
    QList<QTreeWidgetItem *> sessionList(sessions->selectedItems());

    if (1 == sessionList.count()) {
        if (!logViewer) {
            logViewer = new CLogViewer(this);
        }

        logViewer->show(((SessionWidgetItem *)(*(sessionList.begin())))->sessionData());
    }
}

void SessionWidget::dryRunSession() {
    doSessions(true);
}

void SessionWidget::syncSession() {
    doSessions(false);
}

void SessionWidget::restoreSession(bool on) {
    if (!on) {
        controlSyncButtons();
        controlButtons();
    }
}

void SessionWidget::controlButtons() {
    QList<QTreeWidgetItem *> sessionList(sessions->selectedItems());

    emit singleItemSelected(1 == sessionList.count());
    emit itemsSelected(sessionList.count() > 0);
    emit haveLog(1 == sessionList.count() &&
                 !((SessionWidgetItem *)(*(sessionList.begin())))->sessionData().last().isEmpty());
}

void SessionWidget::setAsDefaults() {
    CTTask *prevSched = defSession.sched(),
            *sched = prevSched
                     ? prevSched
                     : new CTTask("", "", "", false);

    sessionDialog->get(defSession, sched);

    if (prevSched || (!prevSched && Session::CronSchedule == defSession.schedType())) {
        defSession.setSched(sched);
    }

    defSession.save(QString());
}

void SessionWidget::controlSyncButtons() {
    emit haveSessions(sessions->topLevelItemCount() > 0);
}

void SessionWidget::loadSessions() {
    QFileInfoList sessionList = QDir(Utils::dataDir(QString(), true)).entryInfoList(QStringList() << "*"CARBON_EXTENSION, QDir::NoDotAndDotDot | QDir::Files);
    QMap<QString, CTTask *> taskMap;
    QList<CTTask *> tasks(ctHost->findCurrentUserCron()->tasks());
    int pos;
    int srlen(strlen(CARBON_RUNNER));
    bool haveSessions(false);

    foreach (CTTask *task, tasks) {
        if (-1 != (pos = task->command.indexOf(QLatin1String(CARBON_RUNNER)))) {
            QString name(task->command.mid(pos + srlen + 1).trimmed());

            if (name.length() && name[0] == QChar('\'') && name[name.length() - 1] == QChar('\'')) {
                name = name.mid(1, name.length() - 2).replace(QLatin1String("'\\''"), QLatin1String("\'")).trimmed();
            }

            taskMap[name] = task;
        }
    }

    QMap<QString, CTTask *>::Iterator mapEnd(taskMap.end());

    foreach (const QFileInfo &session, sessionList) {
        SessionWidgetItem *item = new SessionWidgetItem(sessions, session.absoluteFilePath());

        if (*item) {
            QMap<QString, CTTask *>::Iterator mapIt = taskMap.find(item->text(COL_NAME));

            if (mapIt != mapEnd) {
                item->sessionData().setSched(*mapIt);
                item->sessionData().setSchedType(Session::CronSchedule);
                item->setSchedule(item->sessionData().scheduleStr());
            }

            haveSessions = true;
        } else {
            delete item;
        }
    }

    controlSyncButtons();

    QStringList list;
    QSettings cfg;
    list = cfg.value(CFG_COL_SIZES, list).toStringList();

    if (NUM_COLS == list.count()) {
        for (int i = 0; i < NUM_COLS; ++i) {
            sessions->header()->resizeSection(i, list[i].toInt());
        }
    }

    sessions->sortItems(0, Qt::AscendingOrder);
}

void SessionWidget::contextMenuEvent(QContextMenuEvent *e) {
    if (menu) {
        menu->popup(e->globalPos());
    }
}

void SessionWidget::doSessions(bool dryRun) {
    QList<QTreeWidgetItem *> sessionList(selectedSessions());

    if (sessionList.count()) {
        QStringList names(toNames(sessionList));

        if ((1 == names.count() &&
                QMessageBox::Yes == MessageBox::questionYesNo(this, dryRun
                        ? tr("Perform a dry run of <b>%1</b>?").arg(*(names.begin()))
                        : tr("Synchronise <b>%1</b>").arg(*(names.begin())))) ||
                (names.count() > 1 &&
                 QMessageBox::Yes == MessageBox::questionYesNoList(this, dryRun
                         ? tr("Perform a dry run of the following sessions?")
                         : tr("Synchronise the following sessions?"),
                         names))) {
            QList<Session *> sessionDataList;

            foreach (QTreeWidgetItem *i, sessionList) {
                sessionDataList.append(&(((SessionWidgetItem *)(i))->sessionData()));
            }

            if (sessionDataList.count()) {
                if (!runnerDialog) {
                    runnerDialog = new RunnerDialog(this);
                }

                runnerDialog->go(sessionDataList, dryRun);

                for (int i = 0; i < sessions->topLevelItemCount(); ++i) {
                    SessionWidgetItem *item = (SessionWidgetItem *)(sessions->topLevelItem(i));

                    item->sessionData().updateLast();
                    item->setLastRun(item->sessionData().last());
                }
            }
        }
    }
}

void SessionWidget::initCronTab() {
    QString exe(Utils::findExe("crontab"));

    if (exe.isEmpty()) {
        MessageBox::error(this, tr("<p><b>Synchronisation Scheduling Disabled</b></p>"
                                   "<p>Synkronise uses <i>cron</i> to schedule synchronisations. The"
                                   " <i>crontab</i> program (which is used to edit <i>cron</i> files) "
                                   " has not been found on your system. Please install the relevant packages</p>"));
        ctHost = 0;
    } else {
        CTInitializationError ctInitializationError;

        ctHost = new CTHost(exe, ctInitializationError);
        if (ctInitializationError.hasErrorMessage()) {
            MessageBox::error(this, tr("<p><b>Synchronisation Scheduling Disabled</b></p><p>%1</p>").arg(ctInitializationError.errorMessage()));
            delete ctHost;
            ctHost = 0;
        }
    }

    if (0 == ctHost) {
        sessions->hideColumn(2);
    }
}

void SessionWidget::createSessionDialog() {
    if (!sessionDialog) {
        sessionDialog = new SessionDialog(this);
        connect(sessionDialog, SIGNAL(setAsDefaults()), SLOT(setAsDefaults()));
    }
}

QList<QTreeWidgetItem *> SessionWidget::selectedSessions() {
    QList<QTreeWidgetItem *> sessionList(sessions->selectedItems());

    if (0 == sessionList.count()) {
        for (int i = 0; i < sessions->topLevelItemCount(); ++i) {
            sessionList.append(sessions->topLevelItem(i));
        }
    }
    return sessionList;
}
