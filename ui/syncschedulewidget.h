#ifndef __SYNCSCHEDULE_WIDGET_H__
#define __SYNCSCHEDULE_WIDGET_H__

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

#include <QComboBox>
#include "ui_syncschedulewidget.h"
#include "cronwidget.h"
#include "session.h"

class CTTask;

class SyncScheduleWidget : public QWidget, Ui::SyncScheduleWidget {
    Q_OBJECT

public:
    SyncScheduleWidget(QWidget *parent);
    void set(const Session &session);
    void get(Session &session, CTTask *sched);
    bool cronScheduled() const {
        return Session::CronSchedule == scheduleType->currentIndex();
    }
    bool cronValid() const {
        return cronWidget->valid();
    }

private Q_SLOTS:
    void typeChanged(int idx);

private:
    CronWidget *cronWidget;
};

#endif
