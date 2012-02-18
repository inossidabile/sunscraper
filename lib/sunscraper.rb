require 'sunscraper/library'

# Sunscraper loads an HTML page in a headless browser and waits for `Sunscraper.finish()`
# method to be called. It blocks the calling thread, but is threadsafe, does
# not acquire GIL and thus can be called from multiple threads simultaneously.
module Sunscraper
  # ScrapeTimeout error is raised when the page could not be loaded fast enough.
  class ScrapeTimeout < StandardError; end

  class << self
    # Scrape an inline HTML. The content is loaded without a particular base URL.
    # If your application depends on base URL being available, use {scrape_url}.
    #
    # @param [Integer] timeout timeout in milliseconds
    def scrape_html(html, timeout=5000)
      scrape(timeout) do |context|
        Library.load_html context, html
      end
    end

    # Scrape an URL.
    #
    # @param [Integer] timeout timeout in milliseconds
    def scrape_url(url, timeout=5000)
      scrape(timeout) do |context|
        Library.load_url context, url
      end
    end

    private

    def scrape(timeout)
      context = Library.create

      yield context

      Library.wait(context, timeout)

      data = Library.fetch(context)

      if data == "!SUNSCRAPER_TIMEOUT"
        raise ScrapeTimeout, "Sunscraper has timed out waiting for the callback"
      else
        data
      end
    ensure
      Library.discard(context) if context
    end
  end
end
