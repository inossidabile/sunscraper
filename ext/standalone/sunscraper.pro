QT += webkit

TARGET = sunscraper
TEMPLATE = app

SOURCES += sunscraperproxy.cpp \
    sunscraperworker.cpp \
    sunscraperrpc.cpp \
    sunscrapermain.cpp

HEADERS += sunscraperproxy.h \
    sunscraperrpc.h \
    sunscraperworker.h
