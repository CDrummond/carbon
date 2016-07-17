/***************************************************************************
 *   CT Month Implementation                                               *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ctmonth.h"
#include <QObject>

QList<QString> CTMonth::shortName;

CTMonth::CTMonth(const QString& tokStr) :
    CTUnit(CTMonth::MINIMUM, CTMonth::MAXIMUM, tokStr) {
}

QString CTMonth::describe() const {
    initializeNames();
    return (enabledCount() == CTMonth::MAXIMUM) ? QObject::tr("every month") : CTUnit::genericDescribe(shortName);
}

QString CTMonth::getName(const int ndx) {
    initializeNames();
    return shortName[ndx];
}

void CTMonth::initializeNames() {
    if (shortName.isEmpty()) {
        shortName << "" << QObject::tr("January") << QObject::tr("February") << QObject::tr("March") << QObject::tr("April") << QObject::tr("May", "Month") << QObject::tr("June") << QObject::tr("July", "Month") << QObject::tr("August") << QObject::tr("September") << QObject::tr("October") << QObject::tr("November") << QObject::tr("December");
    }
}
