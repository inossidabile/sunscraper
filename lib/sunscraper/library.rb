if RUBY_PLATFORM =~ /darwin/i || RbConfig::CONFIG['target_os'] == 'darwin'
  raise RuntimeError, "Sunscraper/embed does not work on OS X. Use Sunscraper/standalone."
end

require 'ffi'
require 'rbconfig'

# @private
module Sunscraper
  module Library
    extend FFI::Library

    if Gem.win_platform?
      extension = 'dll'
    else
      extension = 'so'
    end

    ffi_lib File.join(Gem.loaded_specs['sunscraper'].full_gem_path,
                      'ext', 'embed', "libsunscraper.#{extension}")

    attach_function 'create',    :sunscraper_create,    [],                        :uint
    attach_function 'load_html', :sunscraper_load_html, [:uint, :string, :string], :void
    attach_function 'load_url',  :sunscraper_load_url,  [:uint, :string],          :void
    attach_function 'fetch',     :sunscraper_fetch,     [:uint],                   :string
    attach_function 'finalize',  :sunscraper_finalize,  [:uint],                   :void

    if RUBY_ENGINE == 'ruby'
      # MRI uses ffi gem and has GVL. Hence, it needs a rb_thread_blocking_region call.
      attach_function 'wait',    :sunscraper_wait,      [:uint, :uint],            :bool,    :blocking => true
    else
      # Rubinius does not have GVL neither it has options in attach_function.
      # Same for JRuby.
      attach_function 'wait',    :sunscraper_wait,      [:uint, :uint],            :bool
    end

    attach_function 'quit',      :sunscraper_quit,      [],                        :void

    at_exit { quit }
  end
end