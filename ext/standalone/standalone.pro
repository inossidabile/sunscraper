QT += webkit network

TARGET = sunscraper
TEMPLATE = app

SOURCES += sunscraperrpc.cpp \
    sunscraperrpcserver.cpp \
    sunscrapermain.cpp

HEADERS += sunscraperrpc.h \
    sunscraperrpcserver.h

INCLUDEPATH += ../common

unix:{
    LIBS += -L../common -lsunscraper_common
    POST_TARGETDEPS += ../common/libsunscraper_common.a
    LDFLAGS += -pthread
}
