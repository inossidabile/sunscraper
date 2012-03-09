#ifndef SUNSCRAPERLIBRARY_H
#define SUNSCRAPERLIBRARY_H

class SunscraperThread;

class SunscraperLibrary {
public:
    static SunscraperLibrary *instance();

private:
    SunscraperLibrary();
    SunscraperLibrary(SunscraperLibrary &);
    ~SunscraperLibrary();

    static SunscraperLibrary m_instance;
};

#endif // SUNSCRAPER_H