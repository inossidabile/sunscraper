QT += webkit

TARGET = sunscraper
TEMPLATE = lib

SOURCES += sunscraperlibrary.cpp \
    sunscraperworker.cpp \
    sunscraperwebpage.cpp \
    sunscraperexternal.cpp \
    sunscraper.cpp \
    sunscraperproxy.cpp

HEADERS += sunscraperlibrary.h \
    sunscraperworker.h \
    sunscraperwebpage.h \
    sunscraper.h \
    sunscraperproxy.h

linux:LDFLAGS += -pthread
