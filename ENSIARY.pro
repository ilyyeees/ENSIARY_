QT += core widgets

CONFIG += c++20

TARGET = ENSIARY
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/models/resource.cpp \
    src/models/book.cpp \
    src/models/article.cpp \
    src/models/thesis.cpp \
    src/models/digitalcontent.cpp \
    src/models/user.cpp \
    src/models/loan.cpp \
    src/models/reservation.cpp \
    src/services/library_manager.cpp \
    src/services/persistence_service.cpp \
    src/dialogs/resource_dialog.cpp \
    src/dialogs/user_dialog.cpp \
    src/dialogs/user_loans_dialog.cpp \
    src/dialogs/reservation_management_dialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/models/resource.h \
    src/models/book.h \
    src/models/article.h \
    src/models/thesis.h \
    src/models/digitalcontent.h \
    src/models/user.h \
    src/models/loan.h \
    src/models/reservation.h \
    src/services/library_manager.h \
    src/services/persistence_service.h \
    src/dialogs/resource_dialog.h \
    src/dialogs/user_dialog.h \
    src/dialogs/user_loans_dialog.h \
    src/dialogs/reservation_management_dialog.h

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Include paths
INCLUDEPATH += src/models src/services src/dialogs

# For JSON handling (Qt has built-in JSON support)
# Using Qt's built-in JSON classes instead of external library

# Compiler flags for C++20
QMAKE_CXXFLAGS += -std=c++20

# Application icon (optional)
# RC_ICONS = ensiary.ico

# Application version
VERSION = 1.0.0

# Application display name
TARGET = ENSIARY

# Define application name
DEFINES += APP_NAME=\\\"ENSIARY\\\"
DEFINES += APP_VERSION=\\\"1.0.0\\\"

# Resources
RESOURCES += resources/resources.qrc

# Application icon - using PNG only
# win32:RC_ICONS = resources/icon.ico  # Disabled - using PNG only
# macx:ICON = resources/icon.icns
unix:!macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/bin
    shortcutfiles.files = resources/ENSIARY.desktop
    shortcutfiles.path = $$PREFIX/share/applications/
    pixmapfiles.files = resources/icon.png
    pixmapfiles.path = $$PREFIX/share/pixmaps/
    INSTALLS += shortcutfiles
    INSTALLS += pixmapfiles
    INSTALLS += target
}
