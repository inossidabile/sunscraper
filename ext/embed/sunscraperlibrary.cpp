#include "sunscraperlibrary.h"
#include "sunscraperthread.h"
#include <QtDebug>

SunscraperLibrary SunscraperLibrary::m_instance;

SunscraperLibrary::SunscraperLibrary()
{
    SunscraperThread::invoke();
}
