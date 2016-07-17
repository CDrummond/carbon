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

//------------------------

/***************************************************************************
 *   KT task editor window header                                          *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __CRON_WIDGET_H__
#define __CRON_WIDGET_H__

//#include <kmimetype.h>
#include <QWidget>
#include <QCheckBox>

class QLabel;
class QLineEdit;
class QGridLayout;
class QHBoxLayout;
class QComboBox;
class QGroupBox;
class QPushButton;
class SqueezedTextLabel;
class CTTask;
class SetOrClearAllButton;

class CronWidget : public QWidget {
    Q_OBJECT

    enum EStatus {
        NOT_SCHEDULED,
        SCHEDULE_OK,
        SCHEDULE_ERROR
    };

public:
    CronWidget(QWidget *parent);
    ~CronWidget() { }

    void          setupTitleWidget(const QString &comment, EStatus status);
    void          load(const CTTask *ctTask);
    bool          valid() const {
        return ok;
    }
    void          save(CTTask *ctTask);

private Q_SLOTS:
    void          slotRebootChanged();
    void          slotDailyChanged();
    void          slotWizard();
    void          slotAllMonths();
    void          slotMonthChanged();
    void          slotAllDaysOfMonth();
    void          slotDayOfMonthChanged();
    void          slotAllDaysOfWeek();
    void          slotDayOfWeekChanged();
    void          slotAllHours();
    void          slotHourChanged();
    void          slotMinutesPreselection(int index);
    void          slotMinuteChanged();

private:
    QPushButton * createHourButton(QGroupBox *hourGroupBox, int hour);
    QGroupBox *   createHoursGroup(QWidget *mainWidget);
    QPushButton * createMinuteButton(int minuteIndex);
    void          createMinutesGroup(QWidget *mainWidget);

    // Returns true if there is no checked minute in the hidden minuteButton
    bool          canReduceMinutesGroup();
    void          emptyMinutesGroup();
    void          reduceMinutesGroup();
    void          increaseMinutesGroup();
    QGroupBox *   createMonthsGroup(QWidget *mainWidget);
    QGroupBox *   createDaysOfMonthGroup(QWidget *mainWidget);
    QGroupBox *   createDaysOfWeekGroup(QWidget *mainWidget);
    bool          isEveryDay(const CTTask *ctTask);

private:
    SqueezedTextLabel *titleWidget;
    QLabel *statusWidget;
    QCheckBox *onReboot;
    QCheckBox *everyDay;
    QGroupBox *monthGroupBox;
    QPushButton *monthButtons[13]; // The index 0 is not used
    SetOrClearAllButton *monthClearButton;
    QGroupBox *dayOfMonthGroupBox;
    QPushButton *dayOfMonthButtons[32]; // The index 0 is not used
    SetOrClearAllButton *dayOfMonthClearButton;
    QGroupBox *dayOfWeekGroupBox;
    QPushButton *dayOfWeekButtons[8]; // The index 0 is not used
    SetOrClearAllButton *dayOfWeekClearButton;
    QGroupBox *hourGroupBox;
    QPushButton *hourButtons[24];
    SetOrClearAllButton *hourClearButton;
    QGroupBox *minuteGroupBox;
    QGridLayout *minuteLayout;
    QPushButton *minuteButtons[60];
    QHBoxLayout *minutePreSelectionLayout;
    QComboBox *minutePreSelection;
    bool ok;
};

#endif
