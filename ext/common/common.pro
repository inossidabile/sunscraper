TEMPLATE = lib
TARGET   = sunscraper_common

QT += webkit network
CONFIG += staticlib

SOURCES += sunscraperproxy.cpp \
    sunscraperwebpage.cpp \
    sunscraperworker.cpp

HEADERS += sunscraperproxy.h \
    sunscraperwebpage.h \
    sunscraperworker.h
