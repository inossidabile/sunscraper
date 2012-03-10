QT += webkit

TARGET   = sunscraper
TEMPLATE = lib

SOURCES += sunscraperexternal.cpp \
    sunscraperlibrary.cpp \
    sunscraperinterface.cpp \
    sunscraperthread.cpp

HEADERS += sunscraperlibrary.h \
    sunscraperinterface.h \
    sunscraperthread.h

INCLUDEPATH += ../common

unix:{
    LIBS += -L../common -lsunscraper_common
    POST_TARGETDEPS += ../common/libsunscraper_common.a
    LDFLAGS += -pthread
}
