#include "sunscraperlibrary.h"
#include "sunscraperworker.h"
#include <QtDebug>

SunscraperLibrary SunscraperLibrary::m_instance;

SunscraperLibrary::SunscraperLibrary()
{
    SunscraperWorker::invoke();
}

SunscraperLibrary::~SunscraperLibrary()
{
    /* Do nothing. This is on purpose. */
}

SunscraperLibrary *SunscraperLibrary::instance()
{
    return &m_instance;
}
