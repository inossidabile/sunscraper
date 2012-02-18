#ifndef SUNSCRAPERLIBRARY_H
#define SUNSCRAPERLIBRARY_H

class SunscraperThread;

class SunscraperLibrary {
public:
    static SunscraperLibrary *instance();

    SunscraperThread *thread();

private:
    SunscraperLibrary();
    SunscraperLibrary(SunscraperLibrary &);
    ~SunscraperLibrary();

    static SunscraperLibrary _instance;

    SunscraperThread *_apartmentThread;
};

#endif // SUNSCRAPER_H
