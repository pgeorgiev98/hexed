QT += core gui widgets script

TARGET = hexed
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        baseconverter.cpp \
        bufferededitor.cpp \
        byteinputwidget.cpp \
        common.cpp \
        difffilesdialog.cpp \
        endianconverter.cpp \
        expressionvalidator.cpp \
        finder.cpp \
        findwidget.cpp \
        gotodialog.cpp \
        hexview.cpp \
        hexviewinternal.cpp \
        iconprovider.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        baseconverter.h \
        bufferededitor.h \
        byteinputwidget.h \
        common.h \
        difffilesdialog.h \
        endianconverter.h \
        expressionvalidator.h \
        finder.h \
        findwidget.h \
        gotodialog.h \
        hexview.h \
        hexviewinternal.h \
        iconprovider.h \
        mainwindow.h \
        utilities.h

RESOURCES += res/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:RC_ICONS = res/icon.ico
