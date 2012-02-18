Sunscraper
==========

Sunscraper is a gem for prerendering pages with hashbang URLs like `http://whatever.com/#!/page`.

It works by loading content in the embedded web browser and waiting for a JavaScript method to be
called.

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

On Linux with Qt versions <= 4.8, Sunscraper requires a running X server and a valid `DISPLAY` environment
variable. Consider using [Xvfb][] on a GUI-less production server.

  [Xvfb]: http://www.x.org/releases/X11R7.6/doc/man/man1/Xvfb.1.xhtml

Compatibility
-------------

Sunscraper should be compatible across all major implementations on all major operating systems, including
Ruby MRI 1.9, JRuby, Rubinius and MacRuby running on GNU/Linux, OS X and Windows.

JRuby versions up to 1.6.5 are known not to work due to a bug in its FFI library.

Ruby MRI 1.8 is not supported because it has a braindead threading model and will never be because I don't care.

Thread safety
-------------

Sunscraper is thread-safe.

License
-------

Sunscraper is distributed under the terms of a MIT license; see LICENSE in the source distribution.