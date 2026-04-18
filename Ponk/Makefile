vasmBinDir = C:\Users\theyo\.vscode\extensions\prb28.amiga-assembly-1.8.13\resources\bin\win32

all: Obj/Action.o

Obj/Action.o: Source/Action.s
	${vasmBinDir}/vasmm68k_mot -m68000 -devpac -Fhunk -linedebug -nowarn=62 Source/Action.s -o Obj/Action.o

# Obj/Ponk.o: Source/Ponk.s
# 	${vasmBinDir}/vasmm68k_mot -m68000 -devpac -Fhunk -linedebug -nowarn=62 Source/Ponk.s -o Obj/Ponk.o
