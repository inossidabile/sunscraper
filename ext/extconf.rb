# This Makefile will get replaced by qmake.

if Gem.win_platform?
  qmake = %{qmake -spec win32-g++}
elsif RUBY_PLATFORM =~ /darwin/i || RbConfig::CONFIG['target_os'] == 'darwin'
  qmake = %{qmake -spec macx-g++}
else
  qmake = %{qmake}
end

File.open("Makefile", "w") do |mf|
  mf.puts <<-ENDM
all:
	#{qmake}
	make
  ENDM
end
