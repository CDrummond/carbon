/***************************************************************************
 *   --------------------------------------------------------------------  *
 *   CT Host Implementation                                                *
 *   --------------------------------------------------------------------  *
 *   Copyright (C) 1999, Gary Meyer <gary@meyer.net>                       *
 *   --------------------------------------------------------------------  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "cthost.h"

#include <unistd.h>  // getuid()
#include <sys/types.h>
#include <pwd.h>

#include <QFile>
#include <QTextStream>

#include "ctcron.h"
//#include "ctSystemCron.h"
#include "ctInitializationError.h"


#include "logging.h"

CTHost::CTHost(const QString& cronBinary, CTInitializationError& ctInitializationError) {
    struct passwd* userInfos = NULL;

    this->crontabBinary = cronBinary;
    #if 0 // CPD Synkronise is only interested in user crontabs...
    // If it is the root user
    if (getuid() == 0) {
        // Read /etc/passwd
        setpwent(); // restart
        while ((userInfos = getpwent())) {
            if (allowDeny(userInfos->pw_name)) {
                QString errorMessage = createCTCron(userInfos);
                if (errorMessage.isEmpty() == false) {
                    ctInitializationError.setErrorMessage(errorMessage);
                    return;
                }
            }
            //delete userInfos;
        }
        setpwent(); // restart again for others
    }
    // Non-root user, so just create user's cron table.
    else {
    #endif

        // Get name from UID, check it against AllowDeny()
        unsigned int uid = getuid();
        setpwent(); // restart
        while ((userInfos = getpwent())) {
            if ((userInfos->pw_uid == uid) && (!allowDeny(userInfos->pw_name))) {
                ctInitializationError.setErrorMessage(QObject::tr(
                        "<p>You have been blocked from scheduling synchronisations by either the "
                        "/etc/cron.allow file or the /etc/cron.deny file.</p>"
                        "<p>Check the crontab man page for further details.</p>")
                                                     );

                return;
            }
            //delete userInfos;
        }

        setpwent(); // restart again for others

        passwd* currentUserPassword = getpwuid(uid);

        QString errorMessage = createCTCron(currentUserPassword);
        if (errorMessage.isEmpty() == false) {
            ctInitializationError.setErrorMessage(errorMessage);
            return;
        }

        //delete currentUserPassword;
        #if 0
    }
        #endif

    // Create the system cron table.
    createSystemCron();

}

CTHost::~CTHost() {
    foreach (CTCron* ctCron, crons) {
        delete ctCron;
    }
}

bool CTHost::allowDeny(char *name) {
    QFile allow("/etc/cron.allow");

    // if cron.allow exists make sure user is listed
    if (allow.open(QFile::ReadOnly)) {
        QTextStream stream(&allow);
        while (!stream.atEnd()) {
            if (stream.readLine() == name) {
                allow.close();
                return true;
            }
        }
        allow.close();
        return false;
    } else {
        allow.close();
        QFile deny("/etc/cron.deny");

        // else if cron.deny exists make sure user is not listed
        if (deny.open(QFile::ReadOnly)) {
            QTextStream stream(&deny);
            while (!stream.atEnd()) {
                if (stream.readLine() == name) {
                    deny.close();
                    return false;
                }
            }
            deny.close();
            return true;
        } else {
            deny.close();
            return true;
        }
    }
}

CTSaveStatus CTHost::save() {
    //  if (isRootUser() == false) {
    logDebug() << "Save current user cron" << endl;
    CTCron* ctCron = findCurrentUserCron();

    return ctCron->save();
    #if 0
}

foreach (CTCron* ctCron, crons) {
    CTSaveStatus ctSaveStatus = ctCron->save();

    if (ctSaveStatus.isError() == true) {
        return CTSaveStatus(tr("User %1: %2").arg(ctCron->userLogin()).arg(ctSaveStatus.errorMessage()), ctSaveStatus.detailErrorMessage());
    }
}

return CTSaveStatus();
    #endif
}

void CTHost::cancel() {
    foreach (CTCron* ctCron, crons) {
        ctCron->cancel();
    }
}

bool CTHost::isDirty() {
    bool isDirty = false;

    foreach (CTCron* ctCron, crons) {
        if (ctCron->isDirty()) {
            isDirty = true;
        }
    }

    return isDirty;
}

CTCron* CTHost::createSystemCron() {
    #if 0 // CPD Synkronise is only interested in user crontabs...
    CTCron* p = new CTSystemCron(crontabBinary);

    crons.append(p);

    return p;
    #else
    return 0;
    #endif
}


QString CTHost::createCTCron(const struct passwd* userInfos) {
    bool currentUserCron = false;
    if (userInfos->pw_uid == getuid())
        currentUserCron = true;

    CTInitializationError ctInitializationError;
    CTCron* p = new CTCron(crontabBinary, userInfos, currentUserCron, ctInitializationError);
    if (ctInitializationError.hasErrorMessage()) {
        delete p;
        return ctInitializationError.errorMessage();
    }

    crons.append(p);

    return QString();
}

CTCron* CTHost::findCurrentUserCron() const {
    foreach (CTCron* ctCron, crons) {
        if (ctCron->isCurrentUserCron())
            return ctCron;
    }

    logDebug() << "Unable to find the current user Cron. Please report this bug and your crontab config to the developers" << endl;
    return NULL;
}

CTCron* CTHost::findSystemCron() const {
    foreach (CTCron* ctCron, crons) {
        if (ctCron->isMultiUserCron())
            return ctCron;
    }

    logDebug() << "Unable to find the system Cron. Please report this bug and your crontab config to the developers" << endl;
    return NULL;
}

CTCron* CTHost::findUserCron(const QString& userLogin) const {
    foreach (CTCron* ctCron, crons) {
        if (ctCron->userLogin() == userLogin)
            return ctCron;
    }

    logDebug() << "Unable to find the user Cron " << userLogin << ". Please report this bug and your crontab config to the developers" << endl;
    return NULL;
}

CTCron* CTHost::findCronContaining(CTTask* ctTask) const {
    foreach (CTCron* ctCron, crons) {
        if (ctCron->tasks().contains(ctTask) == true) {
            return ctCron;
        }
    }

    logDebug() << "Unable to find the cron of this task. Please report this bug and your crontab config to the developers" << endl;
    return NULL;

}

CTCron* CTHost::findCronContaining(CTVariable* ctVariable) const {
    foreach (CTCron* ctCron, crons) {
        if (ctCron->variables().contains(ctVariable) == true) {
            return ctCron;
        }
    }

    logDebug() << "Unable to find the cron of this variable. Please report this bug and your crontab config to the developers" << endl;
    return NULL;

}

bool CTHost::isRootUser() const {
    return (getuid() == 0);
}
