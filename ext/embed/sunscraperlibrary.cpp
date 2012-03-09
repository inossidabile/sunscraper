#include "sunscraperlibrary.h"
#include "sunscraperthread.h"
#include <QtDebug>

SunscraperLibrary SunscraperLibrary::m_instance;

SunscraperLibrary::SunscraperLibrary()
{
    SunscraperThread::invoke();
}

SunscraperLibrary::~SunscraperLibrary()
{
    /* Do nothing. This is on purpose. */
}

SunscraperLibrary *SunscraperLibrary::instance()
{
    return &m_instance;
}
