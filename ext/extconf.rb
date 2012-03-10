# This Makefile will get replaced by qmake.

require 'rbconfig'

if RUBY_PLATFORM =~ /darwin/i || RbConfig::CONFIG['target_os'] == 'darwin'
  # Cannot you OS X have a build system like all sane people?
  # Win32 wins again.
  qmake = %{qmake CONFIG+=debug -spec macx-g++}

  File.open("Makefile", "w") do |mf|
    mf.puts <<-ENDM
all:
	(cd embed && #{qmake}; make)
	(cd standalone && #{qmake}; make)
install:
	# do nothing
    ENDM
  end
else
  if Gem.win_platform?
    qmake = %{qmake CONFIG+=debug -spec win32-g++}
  else
    qmake = %{qmake CONFIG+=debug}
  end

  File.open("Makefile", "w") do |mf|
    mf.puts <<-ENDM
all:
	#{qmake}
	make
    ENDM
  end
end