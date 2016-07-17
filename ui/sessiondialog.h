#ifndef __SESSION_DIALOG_H__
#define __SESSION_DIALOG_H__

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

class Session;
class GeneralOptionsWidget;
class SyncScheduleWidget;
class ExcludeWidget;
class RSyncOptionsWidget;
class CTTask;
class PageWidget;
class PageWidgetItem;

class SessionDialog : public Dialog {
    Q_OBJECT

public:
    SessionDialog(QWidget *parent);

    bool run(Session &session, bool edit);
    void slotButtonClicked(int btn);
    void get(Session &session, CTTask *sched);
    void getRSyncDefaults(Session &session);
    void getExcludeDefaults(Session &session);

Q_SIGNALS:
    void setAsDefaults();

private:
    PageWidget *pageWidget;
    QString origName;
    QString origSrcPath;
    GeneralOptionsWidget *generalOptions;
    SyncScheduleWidget *syncScheduleWidget;
    ExcludeWidget *excludeWidget;
    RSyncOptionsWidget *rSyncOptionsWidget;
    PageWidgetItem *generalPage;
    PageWidgetItem *syncSchedulePage;
    PageWidgetItem *excludePage;
    PageWidgetItem *rSyncOptionsPage;
};

#endif
