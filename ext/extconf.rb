# This Makefile will get replaced by qmake.

require 'rbconfig'

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