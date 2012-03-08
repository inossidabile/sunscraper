require 'spec_helper'

require 'webrick'

HTML = <<HTML
<html>
<head>
  <script type="text/javascript">
  document.addEventListener("DOMContentLoaded", function() {
    document.getElementById('fuga').textContent =
              ("!skrow tI").split("").reverse().join("");
    Sunscraper.finish();
  }, true);
  </script>
</head>
<body>
  <div id='fuga'></div>
</body>
</html>
HTML

def with_webserver
  port = 32768 + rand(10000)

  server = WEBrick::HTTPServer.new :Port => port, :Logger => WEBrick::Log.new('/dev/null'), :AccessLog => []
  server.mount_proc '/' do |req, res|
    res.body = HTML
  end
  Thread.new { server.start }

  yield port
ensure
  server.stop if server
end

describe "Sunscraper::Library" do
  before do
    Sunscraper.worker = :embed
  end

  it "can scrape an HTML provided as a string" do
    Sunscraper.scrape_html(HTML).should include('It works!')
  end

  it "can scrape an URL" do
    with_webserver do |port|
      Sunscraper.scrape_url("http://localhost:#{port}/", 15000).should include('It works!')
    end
  end

  it "should time out if callback is not called" do
    lambda { Sunscraper.scrape_html("<!-- nothing. at least no callbacks -->", 1000) }.
        should raise_exception(Sunscraper::ScrapeTimeout)
  end
end

describe "Sunscraper::Standalone" do
  before do
    Sunscraper.worker = :standalone
  end

  it "can scrape an HTML provided as a string" do
    Sunscraper.scrape_html(HTML).should include('It works!')
  end

  it "can scrape an URL" do
    with_webserver do |port|
      Sunscraper.scrape_url("http://localhost:#{port}/", 15000).should include('It works!')
    end
  end

  it "should time out if callback is not called" do
    lambda { Sunscraper.scrape_html("<!-- nothing. at least no callbacks -->", 1000) }.
        should raise_exception(Sunscraper::ScrapeTimeout)
  end
end