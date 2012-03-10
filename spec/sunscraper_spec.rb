require 'spec_helper'
require 'webrick'

HTML_TEMPLATE = <<HTML
<html>
<head>
  <script type="text/javascript">
  document.addEventListener("DOMContentLoaded", function() {
    %code%
  }, true);
  </script>
</head>
<body>
  <div id='fuga'></div>
</body>
</html>
HTML

HTML_FUGA = HTML_TEMPLATE.sub("%code%", <<CODE)
    document.getElementById('fuga').textContent =
              ("!skrow tI").split("").reverse().join("");
    Sunscraper.finish();
CODE

HTML_BASEURL = HTML_TEMPLATE.sub("%code%", <<CODE)
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
      if(xhr.readyState > 3) {
        document.getElementById('fuga').textContent = xhr.responseText;
        Sunscraper.finish();
      }
    };
    xhr.open('GET', '/comicstrip', 1);
    xhr.send();
CODE

HTML_USERAGENT = HTML_TEMPLATE.sub("%code%", <<CODE)
    document.getElementById('fuga').textContent =
              window.navigator.userAgent;
    Sunscraper.finish();
CODE

HTML_LOCALSTORAGE = HTML_TEMPLATE.sub("%code%", <<CODE)
    window.localStorage.setItem("key", ["O", "K"].join(""))
    document.getElementById('fuga').textContent =
              window.localStorage.getItem("key");
    Sunscraper.finish();
CODE

def with_webserver(html)
  port = 45555
  server = WEBrick::HTTPServer.new :Port => port, :Logger => WEBrick::Log.new('/dev/null'), :AccessLog => []
  server.mount_proc '/' do |req, res|
    res.body = html
  end
  server.mount_proc '/comicstrip' do |req, res|
    res.body = 'Go Get a Roomie!'
  end
  thread = Thread.new { server.start }

  yield "http://127.0.0.1:#{port}/"
ensure
  server.shutdown
  thread.join
end

define_tests = lambda do |klass, worker|
  describe klass do
    before(:all) do
      Sunscraper.worker = worker
    end

    after(:all) do
      sleep(5) # let threads rest in peace
    end

    it "can scrape an HTML provided as a string" do
      Sunscraper.scrape_html(HTML_FUGA).should include('It works!')
    end

    it "can scrape an URL" do
      with_webserver(HTML_FUGA) do |url|
        Sunscraper.scrape_url(url).should include('It works!')
      end
    end

    it "should time out if callback is not called" do
      lambda { Sunscraper.scrape_html("<!-- nothing. at least no callbacks -->", "about:blank", 500) }.
          should raise_exception(Sunscraper::ScrapeTimeout)
    end

    it "respects baseUrl parameter" do
      with_webserver("<!-- nothing -->") do |url|
        Sunscraper.scrape_html(HTML_BASEURL, url).should include('Go Get a Roomie')
      end
    end

    it "should identify itself as Sunscraper" do
      Sunscraper.scrape_html(HTML_USERAGENT).should include("Sunscraper")
    end

    it "should work with window.localStorage through webserver" do
      with_webserver(HTML_LOCALSTORAGE) do |url|
        Sunscraper.scrape_url(url).should include("OK")
      end
    end

    it "should withstand a lot of concurrent threads" do
      500.times.map {
        Thread.new {
          Sunscraper.scrape_html(HTML_FUGA)
        }
      }.each(&:join).
        map(&:value).
        each { |result|
        result.should include('It works!')
      }
    end
  end
end

unless Sunscraper.os_x?
  # This part currently crashes on OS X (and will forever).
  define_tests.("Sunscraper-Embed", :embed)
end

if !(RUBY_ENGINE =~ /rbx/ || RUBY_ENGINE =~ /jruby/) ||
   ENV['EXPERIMENTAL'] == 'true'
  # This part currently crashes Rubinius (as of Mar 09, 2012),
  # and crashes jruby < 1.7.0, and uses Unix sockets which don't
  # work even on jruby master (as of Mar 09, 2012).
  define_tests.("Sunscraper-Standalone", :standalone)
end