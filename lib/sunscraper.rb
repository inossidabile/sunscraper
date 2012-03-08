if !defined?(RUBY_ENGINE) && RUBY_VERSION =~ /^1.8/
  raise RuntimeError, "Sunscraper does not work on Ruby MRI 1.8.x."
end

# Sunscraper loads an HTML page in a headless browser and waits for `Sunscraper.finish()`
# method to be called. It blocks the calling thread, but is threadsafe, does
# not acquire GIL and thus can be called from multiple threads simultaneously.
module Sunscraper
  # ScrapeTimeout error is raised when the page could not be loaded fast enough.
  class ScrapeTimeout < StandardError; end

  class << self
    attr_reader :worker
    def worker=(worker_type)
      if [:embed, :standalone].include?(worker_type)
        @worker = worker_type
      else
        raise RuntimeError, "Invalid Sunscraper worker type: #{worker_type.inspect}"
      end
    end

    # Scrape an inline HTML. The content is loaded without a particular base URL.
    # If your application depends on base URL being available, use {scrape_url}.
    #
    # @param [Integer] timeout timeout in milliseconds
    def scrape_html(html, timeout=5000)
      scrape(timeout) do |worker, context|
        worker.load_html context, html
      end
    end

    # Scrape an URL.
    #
    # @param [Integer] timeout timeout in milliseconds
    def scrape_url(url, timeout=5000)
      scrape(timeout) do |worker, context|
        worker.load_url context, url
      end
    end

    private

    def scrape(timeout)
      worker = load_worker

      context = worker.create
      yield worker, context
      worker.wait(context, timeout)

      data = worker.fetch(context)

      if data == "!SUNSCRAPER_TIMEOUT"
        raise ScrapeTimeout, "Sunscraper has timed out waiting for the callback"
      else
        data
      end
    ensure
      worker.discard(context) if context
    end

    def load_worker
      case @worker
      when :standalone
        require 'sunscraper/standalone'

        Sunscraper::Standalone

      when :embed
        require 'sunscraper/library'

        Sunscraper::Library
      end
    end
  end
end

if RUBY_PLATFORM =~ /darwin/i || RbConfig::CONFIG['target_os'] == 'darwin'
  # OS X is braindead
  Sunscraper.worker = :standalone
else
  # ... even Win32 is better.
  Sunscraper.worker = :embed
end