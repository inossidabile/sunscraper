# This Makefile will get replaced by qmake.

File.open("Makefile", "w") do |mf|
  mf.puts <<-ENDM
all:
	cd embed; qmake
	cd standalone; qmake
	make -C embed
	make -C standalone
  ENDM
end
