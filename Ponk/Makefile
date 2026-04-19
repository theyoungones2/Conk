# Use USERPROFILE environment variable instead of hardcoding path
vasmBinDir = $(USERPROFILE)\.vscode\extensions\prb28.amiga-assembly-1.8.13\resources\bin\win32
vasmbuild = ${vasmBinDir}/vasmm68k_mot -m68000 -devpac -Fhunk -linedebug -nowarn=62 -I../Devpac/Include/

all: Obj/Ponk.o Obj/Action.o Obj/Operators.o Obj/IH.o Obj/ActionRoutines.o Obj/GroovyCon.o Obj/Sprites.o Obj/BensActionRoutines.o Obj/Scroll.o

Obj/Ponk.o: Source/Ponk.s
	${vasmbuild} Source/Ponk.s -o Obj/Ponk.o

Obj/Action.o: Source/Action.s
	${vasmbuild} Source/Action.s -o Obj/Action.o

Obj/Operators.o: Source/Operators.s
	${vasmbuild} Source/Operators.s -o Obj/Operators.o

Obj/IH.o: Source/IH.s
	${vasmbuild} Source/IH.s -o Obj/IH.o

Obj/ActionRoutines.o: Source/ActionRoutines.s
	${vasmbuild}  Source/ActionRoutines.s -o Obj/ActionRoutines.o

Obj/GroovyCon.o: Source/GroovyCon.s
	${vasmbuild} Source/GroovyCon.s -o Obj/GroovyCon.o

Obj/Sprites.o: Source/Sprites.s
	${vasmbuild} Source/Sprites.s -o Obj/Sprites.o
	
Obj/BensActionRoutines.o: Source/BensActionRoutines.s
	${vasmbuild} Source/BensActionRoutines.s -o Obj/BensActionRoutines.o

Obj/Scroll.o: Source/Scroll.s
	${vasmbuild} Source/Scroll.s -o Obj/Scroll.o

clear:
	del Obj\Ponk.o Obj\Action.o Obj\Operators.o Obj\IH.o Obj\ActionRoutines.o Obj\GroovyCon.o Obj\Sprites.o Obj\BensActionRoutines.o Obj\Scroll.o