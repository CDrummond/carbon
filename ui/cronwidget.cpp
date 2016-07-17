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
 *   KT task editor window implementation                                  *
 *   --------------------------------------------------------------------  *
 *   Copyright(C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 ***************************************************************************/

#include "cronwidget.h"

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QPalette>
#include <QPainter>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFontMetrics>
#include <QStyleOption>
#include <QStylePainter>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include "squeezedtextlabel.h"
#include "logging.h"
#include "cttask.h"
#include "ctcron.h"
#include "cthost.h"

class SetOrClearAllButton : public QPushButton {
public:

    enum Status {
        SET_ALL,
        CLEAR_ALL
    };

    SetOrClearAllButton(QWidget *parent, Status status) : QPushButton(parent) {
        setStatus(status);
    }

    void setStatus(Status status) {
        currentStatus = status;
        setText(SetOrClearAllButton::SET_ALL == currentStatus ? QObject::tr("Set All") : QObject::tr("Clear All"));
    }

    bool isSetAll() const   {
        return SET_ALL == currentStatus;
    }
    bool isClearAll() const {
        return CLEAR_ALL == currentStatus;
    }

private:

    Status currentStatus;
};

static const int constMinuteTotal = 59; // or 55 or 59
static const int constMinutePerColumn = 12; // or 30 or 12
static const int constReducedMinuteStep = 5;

CronWidget::CronWidget(QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->setMargin(0);
    // top title widget
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleWidget = new SqueezedTextLabel(this);
    statusWidget = new QLabel(this);
    titleWidget->setTextElideMode(Qt::ElideRight);
    titleLayout->addWidget(statusWidget);
    titleLayout->addWidget(titleWidget);

    mainLayout->addLayout(titleLayout);

    QHBoxLayout *checkboxesLayout = new QHBoxLayout();

    mainLayout->addLayout(checkboxesLayout);

    // @reboot
    onReboot = new QCheckBox(QObject::tr("Run at system &bootup"), this);
    checkboxesLayout->addWidget(onReboot);

    // Every day
    everyDay = new QCheckBox(QObject::tr("Run &every day"), this);
    checkboxesLayout->addWidget(everyDay);

    QHBoxLayout *schedulingLayout = new QHBoxLayout();
    mainLayout->addLayout(schedulingLayout);

    QVBoxLayout *monthLayout = new QVBoxLayout();
    schedulingLayout->addLayout(monthLayout);

    // months
    monthGroupBox = createMonthsGroup(this);
    monthLayout->addWidget(monthGroupBox);
    monthLayout->addStretch(1);

    QVBoxLayout* v1 = new QVBoxLayout();
    schedulingLayout->addLayout(v1);

    // days of the month
    dayOfMonthGroupBox = createDaysOfMonthGroup(this);
    v1->addWidget(dayOfMonthGroupBox);

    // days of the week
    dayOfWeekGroupBox = createDaysOfWeekGroup(this);
    v1->addWidget(dayOfWeekGroupBox);

    v1->addStretch(1);

    QVBoxLayout *v2 = new QVBoxLayout();
    schedulingLayout->addLayout(v2);

    hourGroupBox = createHoursGroup(this);
    v2->addWidget(hourGroupBox);

    createMinutesGroup(this);
    v2->addWidget(minuteGroupBox);

    v2->addStretch(1);

    connect(onReboot, SIGNAL(clicked()), SLOT(slotRebootChanged()));
    connect(onReboot, SIGNAL(clicked()), SLOT(slotWizard()));
    connect(everyDay, SIGNAL(clicked()), SLOT(slotDailyChanged()));
    connect(everyDay, SIGNAL(clicked()), SLOT(slotWizard()));
}

bool CronWidget::isEveryDay(const CTTask *ctTask) {
    if (!ctTask) {
        return false;
    }

    for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
        if (!ctTask->dayOfWeek.isEnabled(dw)) {
            return false;
        }
    }
    for (int mo = ctTask->month.minimum(); mo <= ctTask->month.maximum(); mo++) {
        if (!ctTask->month.isEnabled(mo)) {
            return false;
        }
    }
    for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
        if (!ctTask->dayOfMonth.isEnabled(dm)) {
            return false;
        }
    }

    return true;
}

QGroupBox * CronWidget::createDaysOfMonthGroup(QWidget *main) {
    QGroupBox   *daysOfMonthGroup = new QGroupBox(QObject::tr("Days of Month"), main);
    QGridLayout *daysOfMonthLayout = new QGridLayout(daysOfMonthGroup);

    int dm = CTDayOfMonth::MINIMUM;
    for (int row = 0; row < 5; ++row) {
        for (int column = 0; column < 7; ++column) {
            QPushButton *day = new QPushButton(daysOfMonthGroup);
            day->setFixedSize(25, 25);
            day->setText(QString::number(dm));
            day->setCheckable(true);
            dayOfMonthButtons[dm] = day;

            connect(dayOfMonthButtons[dm], SIGNAL(clicked()), SLOT(slotDayOfMonthChanged()));
            connect(dayOfMonthButtons[dm], SIGNAL(clicked()), SLOT(slotWizard()));

            daysOfMonthLayout->addWidget(day, row, column);
            if (CTDayOfMonth::MAXIMUM == dm) {
                break;
                break;
            }
            dm++;
        }
    }

    dayOfMonthClearButton = new SetOrClearAllButton(daysOfMonthGroup, SetOrClearAllButton::SET_ALL);
    daysOfMonthLayout->addWidget(dayOfMonthClearButton, 4, 3, 1, 4);

    connect(dayOfMonthClearButton, SIGNAL(clicked()), SLOT(slotAllDaysOfMonth()));
    connect(dayOfMonthClearButton, SIGNAL(clicked()), SLOT(slotWizard()));

    return daysOfMonthGroup;
}

QGroupBox * CronWidget::createMonthsGroup(QWidget *main) {

    QGroupBox   *monthsGroup = new QGroupBox(QObject::tr("Months"), main);
    QGridLayout *monthsLayout = new QGridLayout(monthsGroup);
    int         column = 0,
                row = 0;

    for (int mo = CTMonth::MINIMUM; mo <= CTMonth::MAXIMUM; mo++) {
        monthButtons[mo] = new QPushButton(monthsGroup);
        monthButtons[mo]->setText(CTMonth::getName(mo));
        monthButtons[mo]->setCheckable(true);
        monthsLayout->addWidget(monthButtons[mo], row, column);

        connect(monthButtons[mo], SIGNAL(clicked()), SLOT(slotMonthChanged()));
        connect(monthButtons[mo], SIGNAL(clicked()), SLOT(slotWizard()));

        if (1 == column) {
            column = 0;
            row++;
        } else {
            column = 1;
        }
    }

    monthClearButton = new SetOrClearAllButton(monthsGroup, SetOrClearAllButton::SET_ALL);
    monthsLayout->addWidget(monthClearButton, row, 0, 1, 2);

    connect(monthClearButton, SIGNAL(clicked()), SLOT(slotAllMonths()));
    connect(monthClearButton, SIGNAL(clicked()), SLOT(slotWizard()));

    return monthsGroup;
}

QGroupBox * CronWidget::createDaysOfWeekGroup(QWidget *main) {

    QGroupBox   *daysOfWeekGroup = new QGroupBox(QObject::tr("Days of Week"), main);
    QGridLayout *daysOfWeekLayout = new QGridLayout(daysOfWeekGroup);
    int         column = 0,
                row = 0;
    for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
        dayOfWeekButtons[dw] = new QPushButton(daysOfWeekGroup);
        dayOfWeekButtons[dw]->setText(CTDayOfWeek::getName(dw));
        dayOfWeekButtons[dw]->setCheckable(true);
        daysOfWeekLayout->addWidget(dayOfWeekButtons[dw], row, column);

        connect(dayOfWeekButtons[dw], SIGNAL(clicked()), SLOT(slotDayOfWeekChanged()));
        connect(dayOfWeekButtons[dw], SIGNAL(clicked()), SLOT(slotWizard()));

        if (1 == column) {
            column = 0;
            row++;
        } else {
            column = 1;
        }
    }

    dayOfWeekClearButton = new SetOrClearAllButton(daysOfWeekGroup, SetOrClearAllButton::SET_ALL);
    daysOfWeekLayout->addWidget(dayOfWeekClearButton);

    connect(dayOfWeekClearButton, SIGNAL(clicked()), SLOT(slotAllDaysOfWeek()));
    connect(dayOfWeekClearButton, SIGNAL(clicked()), SLOT(slotWizard()));

    return daysOfWeekGroup;
}

bool CronWidget::canReduceMinutesGroup() {
    for (int minuteIndex = 0; minuteIndex <= constMinuteTotal; ++minuteIndex) {
        if ((minuteIndex % constReducedMinuteStep) && minuteButtons[minuteIndex]->isChecked()) {
            return false;
        }
    }

    return true;
}

void CronWidget::emptyMinutesGroup() {
    for (int minuteIndex = 0; minuteIndex <= constMinuteTotal; ++minuteIndex) {
        minuteLayout->removeWidget(minuteButtons[minuteIndex]);
        minuteButtons[minuteIndex]->hide();
    }
}

void CronWidget::increaseMinutesGroup() {
    emptyMinutesGroup();

    int minuteIndex = 0;
    for (int row = 0; row < (constMinuteTotal + 1) / constMinutePerColumn; ++row) {
        for (int column = 0; column < constMinutePerColumn; ++column) {
            minuteLayout->addWidget(minuteButtons[minuteIndex], row, column);
            minuteButtons[minuteIndex]->show();
            minuteIndex++;
        }
    }
}

void CronWidget::reduceMinutesGroup() {
    emptyMinutesGroup();

    int nextRow = 0;
    int nextColumn = 0;

    for (int minuteIndex = 0; minuteIndex <= constMinuteTotal; ++minuteIndex) {
        if (minuteIndex % constReducedMinuteStep == 0) {
            minuteLayout->addWidget(minuteButtons[minuteIndex], nextRow, nextColumn);
            minuteButtons[minuteIndex]->show();

            nextColumn++;
            if (6 == nextColumn) {
                nextColumn = 0;
                nextRow = 1;
            }
        } else {
            minuteButtons[minuteIndex]->setChecked(false);
        }
    }

    resize(sizeHint());
}

QPushButton * CronWidget::createMinuteButton(int minuteIndex) {
    QPushButton * minuteButton = new QPushButton(minuteGroupBox);
    minuteButton->setText(QString::number(minuteIndex));
    minuteButton->setCheckable(true);
    minuteButton->setFixedSize(25, 25);

    connect(minuteButton, SIGNAL(clicked()), SLOT(slotMinuteChanged()));
    connect(minuteButton, SIGNAL(clicked()), SLOT(slotWizard()));

    return minuteButton;
}

void CronWidget::createMinutesGroup(QWidget *main) {
    minuteGroupBox = new QGroupBox(QObject::tr("Minutes"), main);
    minuteLayout = new QGridLayout(minuteGroupBox);

    for (int minuteIndex = 0; minuteIndex <= constMinuteTotal; ++minuteIndex) {
        minuteButtons[minuteIndex] = createMinuteButton(minuteIndex);
    }

    minutePreSelectionLayout = new QHBoxLayout();

    QLabel *minutePreSelectionLabel = new QLabel(QObject::tr("Preselection:"));
    minutePreSelectionLayout->addWidget(minutePreSelectionLabel);
    minutePreSelection = new QComboBox(this);
    minutePreSelectionLabel->setBuddy(minutePreSelection);
    minutePreSelection->addItem(/*SmallQIcon::fromTheme("edit-clear-locationbar-ltr"), TODO */QObject::tr("Clear selection"), -1);
    minutePreSelection->addItem(QObject::tr("Custom selection"), 0);
    minutePreSelection->addItem(QObject::tr("On the hour"), -1);
    minutePreSelection->addItem(QObject::tr("Every 15 minutes"), 15);
    minutePreSelection->addItem(QObject::tr("Every 30 minutes"), 30);
    minutePreSelectionLayout->addWidget(minutePreSelection);
    minuteLayout->addLayout(minutePreSelectionLayout, ((constMinuteTotal + 1) / constMinutePerColumn), 0, 1, constMinutePerColumn);

    connect(minutePreSelection, SIGNAL(activated(int)), SLOT(slotMinutesPreselection(int)));
    connect(minutePreSelection, SIGNAL(activated(int)), SLOT(slotWizard()));

    //First mandatory increase
    increaseMinutesGroup();

    if (canReduceMinutesGroup()) {
        reduceMinutesGroup();
    }
}

QPushButton * CronWidget::createHourButton(QGroupBox *hourGroupBox, int hour) {
    QPushButton *hourButton = new QPushButton(hourGroupBox);
    hourButton->setText(QString::number(hour));
    hourButton->setCheckable(true);
    hourButton->setFixedSize(25, 25);

    connect(hourButton, SIGNAL(clicked()), SLOT(slotHourChanged()));
    connect(hourButton, SIGNAL(clicked()), SLOT(slotWizard()));
    return hourButton;
}

QGroupBox * CronWidget::createHoursGroup(QWidget *main) {
    QGroupBox   *hourGroupBox = new QGroupBox(QObject::tr("Hours"), main);
    QGridLayout *hoursLayout = new QGridLayout(hourGroupBox); //5 x 7
    QLabel      *label = new QLabel(QObject::tr("AM:"), this);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    hoursLayout->addWidget(label, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);

    int hourCount = 0;
    for (int column = 0; column <= 3 ; ++column) {
        for (int hour = 0; hour <= 5; ++hour) {
            QPushButton* hourButton = createHourButton(hourGroupBox, hourCount);
            hourButtons[hourCount] = hourButton;
            hoursLayout->addWidget(hourButton, column, hour + 1);
            hourCount++;
        }
    }

    label = new QLabel(QObject::tr("PM:"), this);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    hoursLayout->addWidget(label, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);

    hourClearButton = new SetOrClearAllButton(this, SetOrClearAllButton::SET_ALL);
    hoursLayout->addWidget(hourClearButton, 4, 0, 1, 7);

    connect(hourClearButton, SIGNAL(clicked()), SLOT(slotAllHours()));
    connect(hourClearButton, SIGNAL(clicked()), SLOT(slotWizard()));

    return hourGroupBox;
}

void CronWidget::setupTitleWidget(const QString& comment, EStatus status) {
    static const int constIconSize = 32;

    titleWidget->setText(comment);
    switch (status) {
    case NOT_SCHEDULED:
        statusWidget->setPixmap(QIcon::fromTheme("help-hint").pixmap(constIconSize, constIconSize));
        break;
    case SCHEDULE_OK:
        statusWidget->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(constIconSize, constIconSize));
        break;
    case SCHEDULE_ERROR:
        statusWidget->setPixmap(QIcon::fromTheme("dialog-cancel").pixmap(constIconSize, constIconSize));
    }
}

void CronWidget::load(const CTTask *ctTask) {
    onReboot->setChecked(ctTask && ctTask->reboot);
    everyDay->setChecked(isEveryDay(ctTask));

    for (int i = 0; i <= constMinuteTotal ; ++i) {
        minuteButtons[i]->setChecked(ctTask && ctTask->minute.isEnabled(i));
    }

    for (int i = 0; i <= 23; i++) {
        hourButtons[i]->setChecked(ctTask && ctTask->hour.isEnabled(i));
    }

    for (int i = CTDayOfWeek::MINIMUM; i <= CTDayOfWeek::MAXIMUM; i++) {
        dayOfWeekButtons[i]->setChecked(ctTask && ctTask->dayOfWeek.isEnabled(i));
    }

    for (int i = CTDayOfMonth::MINIMUM; i <= CTDayOfMonth::MAXIMUM; ++i) {
        dayOfMonthButtons[i]->setChecked(ctTask && ctTask->dayOfMonth.isEnabled(i));
    }

    for (int i = CTMonth::MINIMUM; i <= CTMonth::MAXIMUM; ++i) {
        monthButtons[i]->setChecked(ctTask && ctTask->month.isEnabled(i));
    }

    if (onReboot->isChecked()) {
        slotRebootChanged();
    } else if (everyDay->isChecked()) {
        slotDailyChanged();
    }

    slotMonthChanged();
    slotDayOfMonthChanged();
    slotDayOfWeekChanged();
    slotHourChanged();
    slotMinuteChanged();

    slotWizard();
}

void CronWidget::save(CTTask *ctTask) {
    if (!ctTask) {
        return;
    }

    // Make it friendly for just selecting days of the month or
    // days of the week.

    int monthDaysSelected(0);
    for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
        if (dayOfMonthButtons[dm]->isChecked()) {
            monthDaysSelected++;
        }
    }

    int weekDaysSelected(0);
    for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
        if (dayOfWeekButtons[dw]->isChecked()) {
            weekDaysSelected++;
        }
    }

    if ((monthDaysSelected == 0) && (weekDaysSelected > 0)) {
        for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
            dayOfMonthButtons[dm]->setChecked(true);
        }
    }

    if ((weekDaysSelected == 0) && (monthDaysSelected > 0)) {
        for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
            dayOfWeekButtons[dw]->setChecked(1);
        }
    }

    ctTask->reboot = onReboot->isChecked();

    for (int mo = CTMonth::MINIMUM; mo <= CTMonth::MAXIMUM; mo++) {
        ctTask->month.setEnabled(mo, monthButtons[mo]->isChecked());
    }
    for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
        ctTask->dayOfMonth.setEnabled(dm, dayOfMonthButtons[dm]->isChecked());
    }
    for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
        ctTask->dayOfWeek.setEnabled(dw, dayOfWeekButtons[dw]->isChecked());
    }
    for (int ho = 0; ho <= 23; ho++) {
        ctTask->hour.setEnabled(ho, hourButtons[ho]->isChecked());
    }
    for (int mi = 0; mi <= constMinuteTotal; ++mi) {
        ctTask->minute.setEnabled(mi, minuteButtons[mi]->isChecked());
    }
}

void CronWidget::slotRebootChanged() {
    bool reboot = !onReboot->isChecked();
    everyDay->setEnabled(reboot);
    hourGroupBox->setEnabled(reboot);
    minuteGroupBox->setEnabled(reboot);

    // if EveryDay is already checked, MonthGroupBox, DayOfMonthGroupBox, DayOfWeekGroupBox are already setEnable(flase)
    // so don't overide them ! ...
    if (!everyDay->isChecked()) {
        monthGroupBox->setEnabled(reboot);
        dayOfMonthGroupBox->setEnabled(reboot);
        dayOfWeekGroupBox->setEnabled(reboot);
    }
}

void CronWidget::slotDailyChanged() {
    if (everyDay->isChecked()) {
        for (int mo = CTMonth::MINIMUM; mo <= CTMonth::MAXIMUM; mo++) {
            monthButtons[mo]->setChecked(true);
        }
        for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
            dayOfMonthButtons[dm]->setChecked(true);
        }
        for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
            dayOfWeekButtons[dw]->setChecked(true);
        }
        monthGroupBox->setEnabled(false);
        dayOfMonthGroupBox->setEnabled(false);
        dayOfWeekGroupBox->setEnabled(false);
        monthClearButton->setEnabled(false);
        dayOfMonthClearButton->setEnabled(false);
        dayOfWeekClearButton->setEnabled(false);
    } else {
        monthGroupBox->setEnabled(true);
        dayOfMonthGroupBox->setEnabled(true);
        dayOfWeekGroupBox->setEnabled(true);
        monthClearButton->setEnabled(true);
        dayOfMonthClearButton->setEnabled(true);
        dayOfWeekClearButton->setEnabled(true);
    }

    slotMonthChanged();
    slotDayOfMonthChanged();
    slotDayOfWeekChanged();
}

void CronWidget::slotWizard() {
    if (onReboot->isChecked()) {
        setupTitleWidget(QObject::tr("<i>Synchronisation will be run on system bootup.</i>"), SCHEDULE_OK);
        ok = false;
        return;
    }

    // the months
    bool valid(false);
    for (int mo = CTMonth::MINIMUM; mo <= CTMonth::MAXIMUM; mo++) {
        if (monthButtons[mo]->isChecked()) {
            valid = true;
        }
    }

    if (!valid) {
        setupTitleWidget(QObject::tr("<i>Please select from the 'Months' section...</i>"), SCHEDULE_ERROR);
        ok = false;
        monthButtons[1]->setFocus();
        return;
    }

    // the days
    valid = false;
    for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
        if (dayOfMonthButtons[dm]->isChecked()) {
            valid = true;
        }
    }
    for (int dw = CTDayOfWeek::MINIMUM; dw <= CTDayOfWeek::MAXIMUM; dw++) {
        if (dayOfWeekButtons[dw]->isChecked()) {
            valid = true;
        }
    }

    if (!valid) {
        setupTitleWidget(QObject::tr("<i>Please select from either the 'Days of Month' or the 'Days of Week' section...</i>"), SCHEDULE_ERROR);
        ok = false;
        dayOfMonthButtons[1]->setFocus();
        return;
    }

    // the hours
    valid = false;
    for (int ho = 0; ho <= 23; ho++) {
        if (hourButtons[ho]->isChecked()) {
            valid = true;
        }
    }

    if (!valid) {
        setupTitleWidget(QObject::tr("<i>Please select from the 'Hours' section...</i>"), SCHEDULE_ERROR);
        ok = false;
        hourButtons[0]->setFocus();
        return;
    }

    // the mins
    valid = false;
    for (int mi = 0; mi <= constMinuteTotal; ++mi) {
        if (minuteButtons[mi]->isChecked()) {
            valid = true;
        }
    }

    if (!valid) {
        setupTitleWidget(QObject::tr("<i>Please select from the 'Minutes' section...</i>"), SCHEDULE_ERROR);
        ok = false;
        minuteButtons[0]->setFocus();
        return;
    }

    CTTask task;
    save(&task);
    setupTitleWidget(QObject::tr("<i>Synchronisation %1</i>").arg(task.describe()), SCHEDULE_OK);
    ok = true;
}

void CronWidget::slotAllMonths() {
    bool checked = monthClearButton->isSetAll();

    for (int mo = CTMonth::MINIMUM; mo <= CTMonth::MAXIMUM; mo++) {
        monthButtons[mo]->setChecked(checked);
    }

    slotMonthChanged();
}

void CronWidget::slotMonthChanged() {
    bool allCleared = true;
    for (int mo = CTMonth::MINIMUM; mo <= CTMonth::MAXIMUM; mo++) {
        if (monthButtons[mo]->isChecked()) {
            allCleared = false;
            break;
        }
    }

    monthClearButton->setStatus(allCleared ? SetOrClearAllButton::SET_ALL : SetOrClearAllButton::CLEAR_ALL);
}

void CronWidget::slotAllDaysOfMonth() {
    bool checked = dayOfMonthClearButton->isSetAll();

    for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
        dayOfMonthButtons[dm]->setChecked(checked);
    }

    slotDayOfMonthChanged();
}

void CronWidget::slotDayOfMonthChanged() {
    bool allCleared = true;
    for (int dm = CTDayOfMonth::MINIMUM; dm <= CTDayOfMonth::MAXIMUM; dm++) {
        if (dayOfMonthButtons[dm]->isChecked()) {
            allCleared = false;
            break;
        }
    }

    dayOfMonthClearButton->setStatus(allCleared ? SetOrClearAllButton::SET_ALL : SetOrClearAllButton::CLEAR_ALL);
}

void CronWidget::slotAllDaysOfWeek() {
    if (dayOfWeekClearButton->isSetAll()) {
        for (int dw = 1; dw <= 7; dw++) {
            dayOfWeekButtons[dw]->setChecked(true);
        }
    } else {
        for (int dw = 1; dw <= 7; dw++) {
            dayOfWeekButtons[dw]->setChecked(false);
        }
    }
    slotDayOfWeekChanged();
}

void CronWidget::slotDayOfWeekChanged() {
    bool allCleared = true;

    for (int dw = 1; dw <= 7; dw++) {
        if (dayOfWeekButtons[dw]->isChecked()) {
            allCleared = false;
        }
    }

    dayOfWeekClearButton->setStatus(allCleared ? SetOrClearAllButton::SET_ALL : SetOrClearAllButton::CLEAR_ALL);
}

void CronWidget::slotAllHours() {
    if (hourClearButton->isSetAll()) {
        for (int ho = 0; ho <= 23; ho++) {
            hourButtons[ho]->setChecked(true);
        }
    } else {
        for (int ho = 0; ho <= 23; ho++) {
            hourButtons[ho]->setChecked(false);
        }
    }
    slotHourChanged();
}

void CronWidget::slotHourChanged() {
    bool allCleared = true;
    for (int ho = 0; ho <= 23; ho++) {
        if (hourButtons[ho]->isChecked()) {
            allCleared = false;
        }
    }

    hourClearButton->setStatus(allCleared ? SetOrClearAllButton::SET_ALL : SetOrClearAllButton::CLEAR_ALL);
}

void CronWidget::slotMinutesPreselection(int index) {
    QVariant itemData = minutePreSelection->itemData(index);
    int      step = itemData.toInt();


    if (-1 == step) {
        minuteButtons[0]->setChecked(true);
        for (int mi = 1; mi <= constMinuteTotal; ++mi) {
            minuteButtons[mi]->setChecked(false);
        }
    } else if (0 != step) {
        for (int mi = 0; mi <= constMinuteTotal; ++mi) {
            minuteButtons[mi]->setChecked(mi % step == 0);
        }
    }

    if (-1 != step && step < constReducedMinuteStep) {
        increaseMinutesGroup();
    } else {
        reduceMinutesGroup();
    }
}

void CronWidget::slotMinuteChanged() {
    CTMinute minutes;

    for (int index = 0; index <= constMinuteTotal; ++index) {
        minutes.setEnabled(index, minuteButtons[index]->isChecked());
    }

    int period = minutes.findPeriod();

    for (int index = 0; index < minutePreSelection->count(); ++index) {
        if (minutePreSelection->itemData(index).toInt() == period) {
            minutePreSelection->setCurrentIndex(index);
            break;
        }
    }
}
