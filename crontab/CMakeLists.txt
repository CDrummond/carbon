set(CRONTAB_SRCS
    cthost.cpp ctcron.cpp ctmonth.cpp ctminute.cpp cthour.cpp ctdom.cpp ctdow.cpp
    cttask.cpp ctunit.cpp ctvariable.cpp ctGlobalCron.cpp ctHelper.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/support ${QTINCLUDES} )
add_library(crontab STATIC ${CRONTAB_SRCS})
