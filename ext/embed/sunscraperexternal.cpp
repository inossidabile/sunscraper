#include "sunscraperinterface.h"
#include "sunscraperthread.h"

extern "C" {
    unsigned sunscraper_create()
    {
        return SunscraperInterface::instance()->createQuery();
    }

    void sunscraper_load_html(unsigned queryId, const char *html, const char *url)
    {
        SunscraperInterface::instance()->loadHtml(queryId, html, QUrl(url));
    }

    void sunscraper_load_url(unsigned queryId, const char *url)
    {
        SunscraperInterface::instance()->loadUrl(queryId, QUrl(url));
    }

    int sunscraper_wait(unsigned queryId, unsigned timeout)
    {
        return SunscraperInterface::instance()->wait(queryId, timeout);
    }

    const char *sunscraper_fetch(unsigned queryId)
    {
        /* VERIFY that the string won't be deleted prematurely */
        return SunscraperInterface::instance()->fetch(queryId).constData();
    }

    void sunscraper_finalize(unsigned queryId)
    {
        SunscraperInterface::instance()->finalize(queryId);
    }

    void sunscraper_quit()
    {
        SunscraperThread::commitSuicide();
    }
}
