QT += webkit network

TARGET = sunscraper
TEMPLATE = app

SOURCES += sunscraperrpc.cpp \
    sunscrapermain.cpp

HEADERS += sunscraperrpc.h

INCLUDEPATH += ../common

unix:{
    LIBS += -L../common -lsunscraper_common
    POST_TARGETDEPS += ../common/libsunscraper_common.a
    LDFLAGS += -pthread
}
