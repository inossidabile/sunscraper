Sunscraper
==========

**DEPRECATED**: Sunscraper is not actively supported anymore. We are still here to accept pull requests but you better be using another headless scraping solution.

Sunscraper is a gem for prerendering pages with hashbang URLs like `http://whatever.com/#!/page`.

It works by loading content in the embedded web browser and waiting for a JavaScript method to be
called.

``` ruby
HTML = %{
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
}

Sunscraper.scrape_html(HTML).include?('It works!') # => true
```

See also [documentation][].

  [documentation]: http://rdoc.info/gems/sunscraper/Sunscraper

Installation
------------

Sunscraper requires Qt 4.x and QtWebkit packages to be installed on the target system. *Sunscraper is not a Ruby
C extension*; it works by building a Qt shared library and loading it through [FFI][].

  [FFI]: http://en.wikipedia.org/wiki/Foreign_Function_Interface

    gem install sunscraper

Runtime requirements
--------------------

On Linux, Sunscraper requires a running X server and a valid `DISPLAY` environment
variable. Consider using [Xvfb][] on a GUI-less production server.

  [Xvfb]: http://www.x.org/releases/X11R7.6/doc/man/man1/Xvfb.1.xhtml

Compatibility
-------------

Sunscraper should be compatible across all major implementations on all major operating systems, including
Ruby MRI 1.9, JRuby, Rubinius and MacRuby running on GNU/Linux, OS X and Windows.

JRuby versions up to 1.6.5 are known not to work due to a bug in its FFI library.

Ruby MRI 1.8 is not supported because it has a braindead threading model and will never be because I don't care.

Qt 4.7.2 has a race condition in QtWebkit internals which sometimes prevents pages from
loading. Use Qt versions not less than 4.7.4 to avoid this bug. As (at this moment) Travis
only has Qt 4.7.2, you cannot test Sunscraper-dependent gems on Travis.

However Qt 4.6.3 from Debian stable repositories showed itself working quite well in production mode and can be considered a good choice.

Thread safety
-------------

Sunscraper is thread-safe.

Maintainers
-----------

* Peter Zotov, [@whitequark](http://twitter.com/whitequark)
* Boris Staal, [@inossidabile](http://staal.io)

License
-------

It is free software, and may be redistributed under the terms of MIT license.

[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/inossidabile/sunscraper/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

