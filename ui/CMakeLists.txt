set(carbon_SRCS
    ${crontablib_SRCS}
    cronwidget.cpp
    excludefile.cpp
    excludewidget.cpp
    generaloptionswidget.cpp
    main.cpp
    mainwindow.cpp
    rsyncoptionswidget.cpp
    runnerdialog.cpp
    sessiondialog.cpp
    session.cpp
    sessionwidget.cpp
    syncschedulewidget.cpp
    treewidget.cpp
    basicitemdelegate.cpp)

set(carbon_MOC_HDRS
    cronwidget.h
    excludewidget.h
    generaloptionswidget.h
    mainwindow.h
    rsyncoptionswidget.h
    runnerdialog.h
    sessiondialog.h
    sessionwidget.h
    syncschedulewidget.h)

set(carbon_UIS
    excludewidget.ui
    generaloptionswidget.ui
    rsyncoptionswidget.ui
    runnerwidget.ui
    sessionwidget.ui
    syncschedulewidget.ui)

set(carbon_RCS carbon.qrc)

QT5_WRAP_CPP(carbon_MOC_SRCS ${carbon_MOC_HDRS})
QT5_WRAP_UI(carbon_UI_HDRS ${carbon_UIS})
QT5_ADD_RESOURCES(carbon_RC_SRCS ${carbon_RCS})

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/support
                     ${CMAKE_SOURCE_DIR}/crontab
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_BINARY_DIR}
                     ${QTINCLUDES})

add_executable(carbon ${carbon_SRCS} ${carbon_MOC_SRCS} ${carbon_UI_HDRS} ${carbon_RC_SRCS})
target_link_libraries(carbon support crontab ${QTLIBS})
install(TARGETS carbon RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
set(XDG_APPS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/applications")
install(FILES carbon.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES default.sync default.sync.exclude DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})

configure_file (carbon-root.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/carbon-root.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/carbon-root.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
