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

    static SunscraperLibrary _instance;
};

#endif // SUNSCRAPER_H
