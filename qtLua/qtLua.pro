#-------------------------------------------------
#
# Project created by QtCreator 2018-03-15T14:04:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtLua
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


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    lua-5.3.4/src/lapi.c \
    lua-5.3.4/src/lauxlib.c \
    lua-5.3.4/src/lbaselib.c \
    lua-5.3.4/src/lbitlib.c \
    lua-5.3.4/src/lcode.c \
    lua-5.3.4/src/lcorolib.c \
    lua-5.3.4/src/lctype.c \
    lua-5.3.4/src/ldblib.c \
    lua-5.3.4/src/ldebug.c \
    lua-5.3.4/src/ldo.c \
    lua-5.3.4/src/ldump.c \
    lua-5.3.4/src/lfunc.c \
    lua-5.3.4/src/lgc.c \
    lua-5.3.4/src/linit.c \
    lua-5.3.4/src/liolib.c \
    lua-5.3.4/src/llex.c \
    lua-5.3.4/src/lmathlib.c \
    lua-5.3.4/src/lmem.c \
    lua-5.3.4/src/loadlib.c \
    lua-5.3.4/src/lobject.c \
    lua-5.3.4/src/lopcodes.c \
    lua-5.3.4/src/loslib.c \
    lua-5.3.4/src/lparser.c \
    lua-5.3.4/src/lstate.c \
    lua-5.3.4/src/lstring.c \
    lua-5.3.4/src/lstrlib.c \
    lua-5.3.4/src/ltable.c \
    lua-5.3.4/src/ltablib.c \
    lua-5.3.4/src/ltm.c \
    lua-5.3.4/src/lundump.c \
    lua-5.3.4/src/lutf8lib.c \
    lua-5.3.4/src/lvm.c \
    lua-5.3.4/src/lzio.c \
    worker.cpp

HEADERS += \
        mainwindow.h \
    lua-5.3.4/src/lapi.h \
    lua-5.3.4/src/lauxlib.h \
    lua-5.3.4/src/lcode.h \
    lua-5.3.4/src/lctype.h \
    lua-5.3.4/src/ldebug.h \
    lua-5.3.4/src/ldo.h \
    lua-5.3.4/src/lfunc.h \
    lua-5.3.4/src/lgc.h \
    lua-5.3.4/src/llex.h \
    lua-5.3.4/src/llimits.h \
    lua-5.3.4/src/lmem.h \
    lua-5.3.4/src/lobject.h \
    lua-5.3.4/src/lopcodes.h \
    lua-5.3.4/src/lparser.h \
    lua-5.3.4/src/lprefix.h \
    lua-5.3.4/src/lstate.h \
    lua-5.3.4/src/lstring.h \
    lua-5.3.4/src/ltable.h \
    lua-5.3.4/src/ltm.h \
    lua-5.3.4/src/lua.h \
    lua-5.3.4/src/lua.hpp \
    lua-5.3.4/src/luaconf.h \
    lua-5.3.4/src/lualib.h \
    lua-5.3.4/src/lundump.h \
    lua-5.3.4/src/lvm.h \
    lua-5.3.4/src/lzio.h \
    worker.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += lua-5.3.4/src
