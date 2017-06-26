#-------------------------------------------------
#
# Project created by QtCreator 2017-06-24T11:07:37
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = canTestTool
TEMPLATE = app

top_srcdir = $$PWD
top_builddir = $$shadowed($$PWD)

CONFIG(release, debug|release): {
    DEFINES += F_NO_DEBUG
    #DEFINES += F_ENABLE_TRACEFILE
    # We add debug info in release f/w so that we can get dig crash issues
    # the release bin should be stripped after "objdump -S"
    QMAKE_CXXFLAGS_RELEASE += -g
    # prevent from linker strip(no '-s')
    QMAKE_LFLAGS_RELEASE =
}

CONFIG(debug, debug|release): {
    DEFINES += F_DISABLE_CRASHDUMP
}

#QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable

win32 {
	# windows only
	QMAKE_CXXFLAGS += -march=i686
	LIBS += $${top_srcdir}/libftd/ftd2xx.lib -lwinmm
	#DEFINES += Q_PLATFORM_WIN
}

SOURCES += main.cpp\
        src/mainwindow.cpp

HEADERS  += \
		src/mainwindow.h

FORMS    += ui/mainwindow.ui

RESOURCES += \
    res/ctt.qrc

#RC_FILE = canTestTool.rc
DESTDIR += ../output/
INCLUDEPATH += src ../libQtDbc/inc libftd
LIBS+= -L../output/libQtDbc -lQtDbc
