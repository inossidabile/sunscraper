# This Makefile will get replaced by qmake.

File.open("Makefile", "w") do |mf|
  mf.puts <<-ENDM
all:
	qmake
	make
  ENDM
end