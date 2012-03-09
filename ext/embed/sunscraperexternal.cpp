#include "sunscraper.h"
#include "sunscraperthread.h"

extern "C" {
    Sunscraper *sunscraper_create()
    {
        return new Sunscraper();
    }

    void sunscraper_load_html(Sunscraper *sunscraper, const char *html)
    {
        sunscraper->loadHtml(html);
    }

    void sunscraper_load_url(Sunscraper *sunscraper, const char *url)
    {
        sunscraper->loadUrl(url);
    }

    void sunscraper_wait(Sunscraper *sunscraper, unsigned timeout)
    {
        sunscraper->wait(timeout);
    }

    const char *sunscraper_fetch(Sunscraper *sunscraper)
    {
        return sunscraper->fetchAsCString();
    }

    void sunscraper_discard(Sunscraper *sunscraper)
    {
        delete sunscraper;
    }

    void sunscraper_finalize()
    {
        SunscraperThread::commitSuicide();
    }
}