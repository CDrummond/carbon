/***************************************************************************
 *   CT Day of Month Implementation                                        *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ctdom.h"
#include <QObject>

QList<QString> CTDayOfMonth::shortName;

CTDayOfMonth::CTDayOfMonth(const QString& tokStr) :
    CTUnit(CTDayOfMonth::MINIMUM, CTDayOfMonth::MAXIMUM, tokStr) {
}

QString CTDayOfMonth::describe() const {
    initializeNames();
    return (enabledCount() == CTDayOfMonth::MAXIMUM) ? QObject::tr("every day ") : CTUnit::genericDescribe(shortName);
}

QString CTDayOfMonth::getName(const int ndx) {
    initializeNames();
    return shortName[ndx];
}

void CTDayOfMonth::initializeNames() {
    if (shortName.isEmpty()) {
        shortName << "" << QObject::tr("1st") << QObject::tr("2nd") << QObject::tr("3rd") << QObject::tr("4th") << QObject::tr("5th") << QObject::tr("6th") << QObject::tr("7th") << QObject::tr("8th") << QObject::tr("9th") << QObject::tr("10th") << QObject::tr("11th") << QObject::tr("12th") << QObject::tr("13th") << QObject::tr("14th") << QObject::tr("15th") << QObject::tr("16th") << QObject::tr("17th")
                  << QObject::tr("18th") << QObject::tr("19th") << QObject::tr("20th") << QObject::tr("21st") << QObject::tr("22nd") << QObject::tr("23rd") << QObject::tr("24th") << QObject::tr("25th") << QObject::tr("26th") << QObject::tr("27th") << QObject::tr("28th") << QObject::tr("29th") << QObject::tr("30th") << QObject::tr("31st");
    }
}
