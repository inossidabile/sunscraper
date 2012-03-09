# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)

Gem::Specification.new do |s|
  s.name        = "sunscraper"
  s.version     = "1.1.0.beta2"
  s.authors     = ["Peter Zotov"]
  s.email       = ["whitequark@whitequark.org"]
  s.homepage    = "http://github.com/whitequark/sunscraper"
  s.summary     = %q{A WebKit-based, JavaScript-capable HTML scraper.}
  s.description = s.summary

  s.rubyforge_project = "sunscraper"

  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.extensions    = ["ext/extconf.rb"]
  s.require_paths = ["lib"]

  s.add_development_dependency "rspec"
  s.add_runtime_dependency     "ffi",  '>= 1.0.11'
end
