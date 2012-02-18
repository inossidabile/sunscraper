require "sunscraper/version"
require 'sunscraper/library'

module Sunscraper
  class ScrapeTimeout < StandardError; end

  class << self

    def scrape_html(html, timeout=5000)
      scrape(timeout) do |context|
        Library.load_html context, html
      end
    end

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
