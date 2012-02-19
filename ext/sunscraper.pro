QT += webkit

TARGET = sunscraper
TEMPLATE = lib

SOURCES += sunscraperlibrary.cpp \
    sunscraperthread.cpp \
    sunscraperexternal.cpp \
    sunscraper.cpp \
    sunscraperproxy.cpp

HEADERS += sunscraperlibrary.h \
    sunscraperthread.h \
    sunscraper.h \
    sunscraperproxy.h

linux:LDFLAGS += -pthread
