#-------------------------------------------------
#
# Project created by QtCreator 2011-10-13T21:58:03
#
#-------------------------------------------------

QT       += core gui sql

TARGET = piqs
TEMPLATE = app


SOURCES += main.cpp\
        piqs.cpp \
    thumbnailmodel.cpp \
    gallery.cpp \
    picture.cpp \
    database.cpp \
    iconcache.cpp \
    browserwidget.cpp \
    imageview.cpp \
    rescanthread.cpp \
    rescandialog.cpp \
    tagset.cpp \
    util.cpp \
    tagvalidator.cpp \
    tagquery.cpp

HEADERS  += piqs.h \
    thumbnailmodel.h \
    gallery.h \
    picture.h \
    database.h \
    iconcache.h \
    browserwidget.h \
    imageview.h \
    rescanthread.h \
    rescandialog.h \
    tagset.h \
    util.h \
    tagvalidator.h \
    tagquery.h

FORMS += \
    imageview.ui \
    rescandialog.ui
