#ifndef __CONFIG_H__
#define __CONFIG_H__

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

#define CARBON_VERSION "@CARBON_VERSION_FULL@"
#define CARBON_EXTENSION "@CARBON_EXTENSION@"
#define CARBON_LOG_EXTENSION "@CARBON_LOG_EXTENSION@"
#define CARBON_INFO_EXTENSION "@CARBON_INFO_EXTENSION@"
#define CARBON_LOCK_EXTENSION "@CARBON_LOCK_EXTENSION@"
#define CARBON_EXCLUDE_EXTENSION "@CARBON_EXCLUDE_EXTENSION@"
#define CARBON_PREFIX "@CARBON_PREFIX@"
#define CARBON_MSG_PREFIX "@CARBON_MSG_PREFIX@"
#define CARBON_ERROR_PREFIX "@CARBON_ERROR_PREFIX@"
#define CARBON_GUI_PARENT "@CARBON_GUI_PARENT@"
#define CARBON_RUNNER "@CMAKE_INSTALL_PREFIX@/share/@CMAKE_PROJECT_NAME@/scripts/@CMAKE_PROJECT_NAME@-runner"
#define CARBON_TERMINATE "@CMAKE_INSTALL_PREFIX@/share/@CMAKE_PROJECT_NAME@/scripts/@CMAKE_PROJECT_NAME@-terminate"
#define INSTALL_PREFIX "@CMAKE_INSTALL_PREFIX@"  /* No CARBON_ prefix to this name, as its used in 'support' */
#define CARBON_SYSTEM_DEF_FILE "@CMAKE_INSTALL_PREFIX@/share/@CMAKE_PROJECT_NAME@/default@CARBON_EXTENSION@"
#define CARBON_PACKAGE_NAME "@PROJECT_NAME@"
#define SHARE_INSTALL_PREFIX "@SHARE_INSTALL_PREFIX@"
#cmakedefine QT_QTDBUS_FOUND 1
#endif
