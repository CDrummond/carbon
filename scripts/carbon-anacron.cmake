#!/bin/bash

#
# Carbon helper script.
#
# This script is intended to be used by anacron, and should be located in
# /etc/cron.@CARBON_ANACRON_PERIOD@
#
# The script will attempt to locate each user's .sync files (located in
# $HOME/.local/share/@CMAKE_PROJECT_NAME@/@CARBON_ANACRON_PERIOD@). It will then call @CMAKE_PROJECT_NAME@-runner
# with the sync file name.
#
# NOTE: This script must be run as the root user, as it uses su to run
# synkronise-runner as the sync user.
#
#----------------------------------------------------------------------------
# (C) Craig Drummond, 2013.
#
# Released under the GPL, V2 or later.
#----------------------------------------------------------------------------

IFS="
"

for line in `cat /etc/passwd` ; do
    name=`echo $line | awk -F: '{print $1}'`
    home=`echo $line | awk -F: '{print $6}'`
    uid=`echo $line | awk -F: '{print $3}'`
    if [ ! -z "$home" ] && [ ! -z "$uid" ] && ( [ "$name" = "root" ] || [ $uid -ge @CARBON_LOWEST_UID@ ] ) &&  [ -d "$home" ] ; then
        if [ -d "$home"/.local/share/@CMAKE_PROJECT_NAME@/@CARBON_ANACRON_PERIOD@ ] ; then
            for link in `find "$home"/.local/share/@CMAKE_PROJECT_NAME@/@CARBON_ANACRON_PERIOD@ -name \*@CARBON_EXTENSION@` ; do
                sync=`readlink -e "$link"`
                if [ ! -z "$sync" ] ; then
                    su $name -c "@CMAKE_INSTALL_PREFIX@/share/@CMAKE_PROJECT_NAME@/scripts/@CMAKE_PROJECT_NAME@-runner \"$sync\""
                fi
            done
        fi
    fi
done
