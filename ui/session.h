#ifndef __SESSION_H__
#define __SESSION_H__

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

#include <QString>
#include <QLatin1String>
#include "cttask.h"
#include "excludefile.h"
#include "config.h"

class Session {
public:

    enum ESchedule {
        NoSchedule,
        DailySchedule,
        WeeklySchedule,
        MonthlySchedule,
        CronSchedule,

        NumScheduleTypes
    };

    Session(const QString &name, bool def = false);
    Session();
    ~Session();

    operator bool()                                           {
        return !src.isEmpty();
    }
    void            updateLast();
    bool            save()                                    {
        return save(sessionName);
    }
    bool            save(const QString &name);
    bool            erase();
    bool            sync(bool dryRun);

    bool            removeLockFile();
    bool            isDefault() const                         {
        return isDef;
    }
    const QString & name() const                              {
        return sessionName;
    }
    QString         fileName() const                          {
        return dirName + sessionName + QLatin1String(CARBON_EXTENSION);
    }
    QString         logFileName() const                       {
        return dirName + sessionName + QLatin1String(CARBON_EXTENSION CARBON_LOG_EXTENSION);
    }
    QString         infoFileName() const                      {
        return dirName + sessionName + QLatin1String(CARBON_EXTENSION CARBON_INFO_EXTENSION);
    }
    QString         lockFileName() const                      {
        return dirName + sessionName + QLatin1String(CARBON_EXTENSION CARBON_LOCK_EXTENSION);
    }
    QString         excludeFileName() const                   {
        return dirName + sessionName + QLatin1String(CARBON_EXTENSION CARBON_EXCLUDE_EXTENSION);
    }
    const QString & last() const                              {
        return lastSyncDate;
    }
    const QString & source() const                            {
        return src;
    }
    const QString & destination() const                       {
        return dest;
    }
    const QString & customOpts() const                        {
        return customOptions;
    }
    CTTask *        sched() const                             {
        return schedule;
    }
    ExcludeFile *   excludeFile() const                       {
        return exclude;
    }
    void            setSched(CTTask *sched);
    QString         scheduleStr() const                       {
        return schedule ? schedule->describe() : scheduleStr(scheduleType);
    }
    static QString  scheduleStr(ESchedule type);
    ESchedule       schedType() const                         {
        return scheduleType;
    }
    const QString & crontabStr() const                        {
        return cronStr;
    }
    void            setName(const QString &v);
    void            setDir(const QString &v)                  {
        dirName = v;
    }
    void            setSource(const QString &v)               {
        src = v;
    }
    void            setDestination(const QString &v)          {
        dest = v;
    }
    void            setCustomOpts(const QString &v)           {
        customOptions = v;
    }
    void            setExcludePatterns(const ExcludeFile::PatternList &list);
    bool            archiveFlag() const                       {
        return archive;
    }
    bool            recursiveFlag() const                     {
        return recursive;
    }
    bool            skipFilesOnSizeMatchFlag() const          {
        return skipFilesOnSizeMatch;
    }
    bool            skipReceiverNewerFilesFlag() const        {
        return skipReceiverNewerFiles;
    }
    bool            keepPartialFlag() const                   {
        return keepPartial;
    }
    bool            onlyUpdateFlag() const                    {
        return onlyUpdate;
    }
    bool            useCompressionFlag() const                {
        return useCompression;
    }
    bool            checksumFlag() const                      {
        return checksum;
    }
    bool            windowsFlag() const                       {
        return windowsCompat;
    }
    bool            ignoreExistingFlag() const                {
        return ignoreExisting;
    }
    bool            makeBackupsFlag() const                   {
        return makeBackups;
    }
    bool            deleteExtraFilesOnReceiverFlag() const    {
        return deleteExtraFilesOnReceiver;
    }
    bool            copySymlinksAsSymlinksFlag() const        {
        return copySymlinksAsSymlinks;
    }
    bool            preservePermissionsFlag() const           {
        return preservePermissions;
    }
    bool            preserveSpecialFilesFlag() const          {
        return preserveSpecialFiles;
    }
    bool            preserveOwnerFlag() const                 {
        return preserveOwner;
    }
    bool            dontLeaveFileSystemFlag() const           {
        return dontLeaveFileSystem;
    }
    bool            preserveGroupFlag() const                 {
        return preserveGroup;
    }
    bool            modificationTimesFlag() const             {
        return modificationTimes;
    }
    bool            cvsExcludeFlag() const                    {
        return cvsExclude;
    }
    int             maxBackupDays() const                     {
        return maxBackupAge;
    }
    int             maxSize() const                           {
        return maxFileSize;
    }
    void            setArchiveFlag(bool v)                    {
        archive = v;
    }
    void            setRecursiveFlag(bool v)                  {
        recursive = v;
    }
    void            setSkipFilesOnSizeMatchFlag(bool v)       {
        skipFilesOnSizeMatch = v;
    }
    void            setSkipReceiverNewerFilesFlag(bool v)     {
        skipReceiverNewerFiles = v;
    }
    void            setKeepPartialFlag(bool v)                {
        keepPartial = v;
    }
    void            setOnlyUpdateFlag(bool v)                 {
        onlyUpdate = v;
    }
    void            setUseCompressionFlag(bool v)             {
        useCompression = v;
    }
    void            setChecksumFlag(bool v)                   {
        checksum = v;
    }
    void            setWindowsFlag(bool v)                    {
        windowsCompat = v;
    }
    void            setIgnoreExistingFlag(bool v)             {
        ignoreExisting = v;
    }
    void            setMakeBackupsFlag(bool v)                {
        makeBackups = v;
    }
    void            setDeleteExtraFilesOnReceiverFlag(bool v) {
        deleteExtraFilesOnReceiver = v;
    }
    void            setCopySymlinksAsSymlinksFlag(bool v)     {
        copySymlinksAsSymlinks = v;
    }
    void            setPreservePermissionsFlag(bool v)        {
        preservePermissions = v;
    }
    void            setPreserveSpecialFilesFlag(bool v)       {
        preserveSpecialFiles = v;
    }
    void            setPreserveOwnerFlag(bool v)              {
        preserveOwner = v;
    }
    void            setDontLeaveFileSystemFlag(bool v)        {
        dontLeaveFileSystem = v;
    }
    void            setPreserveGroupFlag(bool v)              {
        preserveGroup = v;
    }
    void            setModificationTimesFlag(bool v)          {
        modificationTimes = v;
    }
    void            setCvsExcludeFlag(bool v)                 {
        cvsExclude = v;
    }
    void            setMaxBackupDays(int v)                   {
        maxBackupAge = v;
    }
    void            setMaxSize(int v)                         {
        maxFileSize = v;
    }
    void            setSchedType(ESchedule v);

private:
    Session(const Session &o);

    bool            removeFiles();
    QString         simpleScheduleLinkDir() const;
    QString         simpleScheduleLink() const;
    bool            simpleScheduleLinkExists() const;
    void            addSimpleSchedule() const;
    void            removeSimpleSchedule() const;

private:
    bool isDef;
    QString dirName;
    QString sessionName;
    QString lastSyncDate;
    QString src;
    QString dest;
    bool archive;
    bool recursive;
    bool skipFilesOnSizeMatch;
    bool skipReceiverNewerFiles;
    bool keepPartial;
    bool onlyUpdate;
    bool useCompression;
    bool checksum;
    bool windowsCompat;
    bool ignoreExisting;
    bool makeBackups;
    bool deleteExtraFilesOnReceiver;
    bool copySymlinksAsSymlinks;
    bool preservePermissions;
    bool preserveSpecialFiles;
    bool preserveOwner;
    bool dontLeaveFileSystem;
    bool preserveGroup;
    bool modificationTimes;
    bool cvsExclude;
    int maxBackupAge;
    int maxFileSize;
    ExcludeFile *exclude;
    QString customOptions;
    QString cronStr;
    CTTask *schedule; // DO NOT new/delete this - it is controlled externally!
    ESchedule scheduleType;
};

#endif
