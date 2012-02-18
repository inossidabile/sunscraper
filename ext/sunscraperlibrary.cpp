#include "sunscraperlibrary.h"
#include "sunscraperthread.h"

SunscraperLibrary SunscraperLibrary::_instance;

SunscraperLibrary::SunscraperLibrary()
{
    _apartmentThread = new SunscraperThread();
    _apartmentThread->start();
}

SunscraperLibrary::~SunscraperLibrary()
{
    /* Do nothing. This is on purpose. */
}

SunscraperLibrary *SunscraperLibrary::instance()
{
    return &_instance;
}

SunscraperThread *SunscraperLibrary::thread()
{
    return _apartmentThread;
}
