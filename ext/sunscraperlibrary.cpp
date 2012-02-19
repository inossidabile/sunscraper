#include "sunscraperlibrary.h"
#include "sunscraperthread.h"

SunscraperLibrary SunscraperLibrary::_instance;

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
    return &_instance;
}
