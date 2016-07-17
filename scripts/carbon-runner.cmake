#!/bin/bash

appName=`basename $0`

function show_help()
{
    exName=Wibble
    echo
    echo "@CMAKE_PROJECT_NAME@ Session runner v@CARBON_VERSION_FULL@"
    echo
    echo "(C) Craig Drummond 2013 - Released under the GPL (v3 or later)"
    echo
    echo "Usage: $appName [-d] <session/filename>"
    echo "       -d  Perform a dry-run (i.e. show what would happen, but don't"
    echo "           do any actual synchronisation)"
    echo
    echo "e.g. $appName $exName"
    echo "         - This will synchronise the $exName session (as created in"
    echo "           @CMAKE_PROJECT_NAME@). This is achieved by loading the settings"
    echo "           from $HOME/.local/share/$projectName/$fileName@CARBON_EXTENSION@"
    echo
    echo "     $appName $HOME/.local/share/$projectName/$exName@CARBON_EXTENSION@"
    echo "         - This will perform a synchronisation using the settings"
    echo "           within $HOME/$exName@CARBON_EXTENSION@"
    echo
    exit 101
}

shortOpts="dh"
longOpts="dryrun,help"
progName=carbon-runner
args=$(getopt -s bash --options $shortOpts  --longoptions $longOpts --name $progName -- "$@" )

eval set -- "$args"

while true; do
   case $1 in
      -h|--help)
         show_help
         ;;
      -d|--dryrun)
         shift
         doDryRun=true
         ;;
      *)
         shift
         break
         ;;
   esac
done

fileName=$1
sessionName=`basename "$fileName" | sed s^@CARBON_EXTENSION@^^g`

function fix_url()
{
    # Remove file:// from URL
    #fixed="`echo $1/ | sed s/file://g | sed s^//^/^g | sed s^//^/^g`"
    fixed="`echo $1/ | sed s^file://^^g | sed s^//^/^g`"
    if [[ "$fixed" != */ ]] ; then
        fixed=$fixed/
    fi

    echo $fixed
}

function log_msg()
{
    if [ "$CARBON_NO_MSG_PREFIX" = "true" ] ; then
        echo "$1"
    else
        echo "@CARBON_MSG_PREFIX@$1"
    fi
}

function log_error()
{
    if [ "$CARBON_NO_MSG_PREFIX" = "true" ] ; then
        echo "$1"
    else
        echo "@CARBON_ERROR_PREFIX@$1"
    fi
}

projectName=`echo @CMAKE_PROJECT_NAME@ | tr "[:upper:]" "[:lower:]"`

function notify()
{
    notifyApp=`which notify-send`
    if [ "$notifyApp" != "" ] ; then
        if [ "$2" = "start" ] ;then
            $notifyApp --app-name=@CMAKE_PROJECT_NAME@ --icon=$projectName "Session Starting" "Starting synchronisation of \"$sessionName\"" > /dev/null 2>&1
        elif [ "$2" = "error" ] ;then
            $notifyApp --app-name=@CMAKE_PROJECT_NAME@ --icon=$projectName "Session Failed" "Synchronisation of \"$sessionName\" failed.<br/>$3" > /dev/null 2>&1
        elif [ $1 -eq 0 ] ; then
            $notifyApp --app-name=@CMAKE_PROJECT_NAME@ --icon=$projectName "Session Complete" "Sucessfully completed synchronisation of \"$sessionName\"" > /dev/null 2>&1
        else
            $notifyApp --app-name=@CMAKE_PROJECT_NAME@ --icon=$projectName "Session Failed" "Synchronisation of \"$sessionName\" failed.<br/>Please check log file." > /dev/null 2>&1
        fi
    fi
}

function log_error_and_exit()
{
    notify $1 "error" "$2"
    log_error "$2"
    exit $1
}

if [ "$fileName" = "" ]; then
    show_help
fi

if [ ! -f "$fileName" ] ; then
    session="$HOME/.local/share/$projectName/$fileName@CARBON_EXTENSION@"
    if [ "$session" != "" ] ; then
        fileName=$session
    fi
fi

if [ ! -f "$fileName" ] ; then
    log_error_and_exit 102 "Could not read \"$fileName\""
fi

if [ -e "$fileName@CARBON_LOCK_EXTENSION@" ] ; then
    prevPid=`cat "$fileName@CARBON_LOCK_EXTENSION@"`
    if ps -p $prevPid > /dev/null ; then
        log_error_and_exit 113 "Session is already running"
    fi
fi

# Keys used in backup info file
backupTimeKey=BackupTime

maxBackupAge=0
maxFileSize=0

if [ "$@CARBON_GUI_PARENT@" != "true" ] ; then
    if [ -f "$fileName@CARBON_LOG_EXTENSION@" ] ; then
        rm "$fileName@CARBON_LOG_EXTENSION@"
    fi

    # We are not being run via the GUI - so we need to log all output
    # So re-run this script, but capture all output to log file...
    @CARBON_GUI_PARENT@=true CARBON_NO_MSG_PREFIX=true $0 "$@" > "$fileName@CARBON_LOG_EXTENSION@" 2>&1
else
    notify 0 start
    eval `cat "$fileName" | grep -A999 "\[Settings\]" | tail -n +2`
    src=`fix_url $src`
    dest=`fix_url $dest`

    if [ -z "$src" ] ; then
        log_error_and_exit 109 "Source is empty"
    fi

    srcIsRemote=0
    if [ "`echo $src | grep \: | wc -l`" != "0" ] ; then
        srcIsRemote=1
    fi

    if [ $srcIsRemote -eq 0 ] && [ ! -e "$src" ] ; then
        log_error_and_exit 112 "Source does not exist"
    fi

    if [ -z "$dest" ] ; then
        log_error_and_exit 110 "destination is empty"
    fi

    if [ "$CARBON_NO_MSG_PREFIX" = "true" ] ; then
        command="rsync -v --progress --include=*.exe --exclude=@CARBON_EXTENSION@@CARBON_LOG_EXTENSION@ --out-format=%f"
    else
        command="rsync -v --progress --include=*.exe --exclude=@CARBON_EXTENSION@@CARBON_LOG_EXTENSION@ --out-format=@CARBON_PREFIX@%f"
    fi

    # Map @CARBON_EXTENSION@ file options to rsync options...
    if [ "$archive" = "true" ] || [ "$archive" = "" ] || [ "$makeBackups" = "true" ] || [ "$makeBackups" = "" ] ; then command="$command --archive"; fi
    if [ "$recursive" = "true" ] || [ "$recursive" = "" ] ; then command="$command --recursive"; fi
    if [ "$skipFilesOnSizeMatch" = "true" ]; then command="$command --size-only"; fi
    if [ "$skipReceiverNewerFiles" = "true" ] || [ "$skipReceiverNewerFiles" = "" ]; then command="$command --update"; fi
    if [ "$keepPartial" = "true" ]; then command="$command --partial"; fi
    if [ "$onlyUpdate" = "true" ]; then command="$command --existing"; fi
    if [ "$useCompression" = "true" ]; then command="$command --compress"; fi
    if [ "$checksum" = "true" ]; then command="$command --checksum"; fi
    if [ "$windowsCompat" = "true" ]; then command="$command --modify-window=1"; fi
    if [ "$ignoreExisting" = "true" ]; then command="$command --ignore-existing"; fi
    if [ "$archive" != "true" ] && ( [ "$preservePermissions" = "true" ] || [ "$preservePermissions" = "" ] ) ; then command="$command --perms"; fi
    if [ "$archive" != "true" ] && ( [ "$preserveGroup" = "true" ] || [ "$preserveGroup" = "" ] ) ; then command="$command --group"; fi
    if [ "$modificationTimes" = "true" ] || [ "$modificationTimes" = "" ]; then command="$command --times"; fi
    if ( [ "$deleteExtraFilesOnReceiver" = "true" ] || [ "$deleteExtraFilesOnReceiver" = "" ] ) && [ "$recursive" != "false" ] ; then
         command="$command --delete";
    fi
    if [ "$archive" != "true" ] && ( [ "$preserveOwner" = "true" ] || [ "$preserveOwner" = "" ] ) ; then command="$command --owner"; fi
    if [ "$archive" != "true" ] && ( [ "$preserveSpecialFiles" = "true" ] || [ "$preserveSpecialFiles" = "" ] ) ; then command="$command -D"; fi
    if [ "$archive" != "true" ] && ( [ "$copySymlinksAsSymlinks" = "true" ] || [ "$copySymlinksAsSymlinks" = "" ] ) ; then command="$command --links"; fi
    if [ "$dontLeaveFileSystem" = "true" ]; then command="$command --one-file-system"; fi
    if [ "$cvsExclude" = "true" ] || [ "$cvsExclude" = "" ]; then command="$command --cvs-exclude"; fi
    if [ "$CustomOptions" != "" ] ; then command="$command $CustomOptions"; fi
    if [ "$maxFileSize" != "" ] && [ $maxFileSize -gt 0 ] ; then command="$command --max-size=$maxFileSize"M ; fi

    destIsRemote=0
    if [ "`echo $dest | grep \: | wc -l`" != "0" ] ; then
        destIsRemote=1
    fi

    if [ $destIsRemote -eq 0 ] && [ ! -d "$dest" ] ; then
        log_error_and_exit 103 "$dest does not exist"
    fi

    excludeFrom="$fileName@CARBON_EXCLUDE_EXTENSION@"

    if [ "$doDryRun" = "true" ]; then
        command="$command -n"
    fi

    destFolder="$dest"
    linkDestFolder="$dest"

    if [ "$makeBackups" = "true" ]; then
        currentBackupTime=`date +"%Y-%m-%d %H:%M:%S"`
        currentBackupTimeFile="$fileName@CARBON_INFO_EXTENSION@"
        previousBackupTime=`cat "$currentBackupTimeFile" | grep "$backupTimeKey" | awk -F\= '{print $2}'`

        if [ -f "$dest$currentBackupTime" ] ; then
            log_error_and_exit 104 " $dest$currentBackupTime is a file!"
        fi

        if [ ! -z "$previousBackupTime" ] ; then
            linkDestFolder="$dest/$previousBackupTime"
        fi
        if [ ! -z "$currentBackupTime" ] ; then
            destFolder="$dest/$currentBackupTime"
        fi

        if [ -z "$previousBackupTime" ]; then
            log_msg "Full backup, as no previous backup"
        elif [ $destIsRemote -eq 0 ] && [ ! -d "$linkDestFolder" ] ; then
            log_msg "Full backup, as previous folder does not exist"
            linkDestFolder=""
        else
            log_msg "Incremental backup (previous $linkDestFolder)"
        fi
    fi

    # Store PID in lock file, to prevent multiple executions...
    echo $$ > "$fileName@CARBON_LOCK_EXTENSION@"

    if [ "$linkDestFolder" = "" ] ; then
        # linkDest is empty, so dont pass as arg - else rsync moans...
        if [ -f "$excludeFrom" ] ; then
            $command "$src" "$destFolder" --exclude-from="$excludeFrom"
        else
            $command "$src" "$destFolder"
        fi
    else
        if [ -f "$excludeFrom" ] ; then
            $command "$src" "$destFolder" --exclude-from="$excludeFrom" --link-dest="$linkDestFolder"
        else
            $command "$src" "$destFolder" --link-dest="$linkDestFolder"
        fi
    fi

    if [ "$makeBackups" = "true" ] && [ -d "$destFolder" ] ; then
        # Store date of this backup
        echo "$backupTimeKey=$currentBackupTime" > "$currentBackupTimeFile"
    fi

    rv=$?

    if [ $destIsRemote -eq 0 ] ; then
        let maxBackupAge="$maxBackupAge * 24 * 60 * 60"

        # Remove any old increments
        if [ "$makeBackups" = "true" ] && [ $maxBackupAge -gt 0 ] && [ -d "$destFolder" ] && [ "$dest" != "/" ] ; then
            log_msg "Cleaning old backups"
            touch "$destFolder"
            currentAge=`stat --terse --format=%Y "$destFolder"`

            oldIfs=$IFS
            IFS="
"

            for old in "$dest"/* ; do
                if [ -d "$old" ] && [ "$old" != "/" ] ; then
                    let diff="$currentAge - `stat --terse --format=%Y $old`"

                    if [ $diff -gt $maxBackupAge ] ; then
                        log_msg "Erasing $old"
                        \rm -r "$old"
                    fi
                fi
            done
            IFS=$oldIfs
        fi
    fi

    # Remove lock
    rm "$fileName@CARBON_LOCK_EXTENSION@"

    notify $rv

    # Return rsyncs exit value...
    exit $rv
fi
