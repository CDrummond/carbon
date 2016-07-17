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

#include "syncschedulewidget.h"
#include "cttask.h"
#include <QVBoxLayout>

SyncScheduleWidget::SyncScheduleWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi(this);

    cronWidget = new CronWidget(cronOptions);
    QVBoxLayout *layout = new QVBoxLayout(cronOptions);
    layout->addWidget(cronWidget);

    scheduleType->insertItem(Session::NoSchedule, Session::scheduleStr(Session::NoSchedule));
    scheduleType->insertItem(Session::DailySchedule, Session::scheduleStr(Session::DailySchedule));
    scheduleType->insertItem(Session::WeeklySchedule, Session::scheduleStr(Session::WeeklySchedule));
    scheduleType->insertItem(Session::MonthlySchedule, Session::scheduleStr(Session::MonthlySchedule));
    scheduleType->insertItem(Session::CronSchedule, Session::scheduleStr(Session::CronSchedule));

    connect(scheduleType, SIGNAL(activated(int)), SLOT(typeChanged(int)));
    typeChanged(Session::DailySchedule);
}

void SyncScheduleWidget::set(const Session &session) {
    scheduleType->setCurrentIndex((int)session.schedType());
    typeChanged((int)session.schedType());

    if (session.sched() || session.crontabStr().isEmpty()) {
        cronWidget->load(session.sched());
    } else {
        CTTask sched(session.crontabStr(), "", "", false);
        cronWidget->load(&sched);
    }
}

void SyncScheduleWidget::get(Session &session, CTTask *sched) {
    session.setSchedType((Session::ESchedule)(scheduleType->currentIndex()));
    if (Session::CronSchedule == scheduleType->currentIndex()) {
        cronWidget->save(sched);
    } else if (sched) {
        sched->enabled = false;
    }
}

void SyncScheduleWidget::typeChanged(int idx) {
    cronOptions->setVisible(Session::CronSchedule == idx);
}
