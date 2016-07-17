set(SUPPORT_SRCS messagebox.cpp
    utils.cpp dialog.cpp
    squeezedtextlabel.cpp pathrequester.cpp pagewidget.cpp)

set(SUPPORT_MOC_HDRS pagewidget.h dialog.h pathrequester.h)

qt5_wrap_cpp(SUPPORT_MOC_SRCS ${SUPPORT_MOC_HDRS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QTINCLUDES} )
add_library(support STATIC ${SUPPORT_MOC_SRCS} ${SUPPORT_SRCS} ${SUPPORT_UI_HDRS} )
