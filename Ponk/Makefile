# Use USERPROFILE environment variable instead of hardcoding path
vasmBinDir = $(USERPROFILE)\.vscode\extensions\prb28.amiga-assembly-1.8.13\resources\bin\win32
vlinkBinDir = $(USERPROFILE)\.vscode\extensions\prb28.amiga-assembly-1.8.13\resources\bin\win32
vasmbuild = ${vasmBinDir}/vasmm68k_mot -m68000 -devpac -Fhunk -linedebug -nowarn=62 -I../Devpac/Include/
vlink = ${vlinkBinDir}/vlink
#vlinkargs = -bamigahunk -Bstatic
vlinkargs = -bamigahunk -Bstatic
linklibs = ../sc/lib/scnb.lib ../sc/lib/amiga.lib 
# linklibs = ../sc/lib/amiga.lib

.PHONY: all link clear

all: Obj/Ponk.o Obj/Action.o Obj/Operators.o Obj/IH.o Obj/ActionRoutines.o Obj/GroovyCon.o Obj/Sprites.o Obj/BensActionRoutines.o Obj/Scroll.o

link: all
	${vlink} ${vlinkargs} \
 "obj/Ponk.o" \
 "obj/Action.o" \
 "obj/ActionRoutines.o" \
 "obj/Operators.o" \
 "obj/Scroll.o" \
 "obj/Sprites.o" \
 "obj/IH.o" \
 "obj/Spam.o" \
 "obj/ModulePlayer.o" \
 "obj/GroovyCon.o" \
 "obj/LameMain.o" \
 "obj/LameMisc.o" \
 "obj/LameDisplay.o" \
 "obj/LameILBM.o" \
 "obj/LameProg.o" \
 "obj/LameDudes.o" \
 "obj/LameBobSpriteAnim.o" \
 "obj/LamePlayer.o" \
 "obj/LameWeapon.o" \
 "obj/LameSound.o" \
 "obj/LameAnimations.o" \
 "obj/LameEOTR.o" \
 "obj/BensActionRoutines.o" \
 -o Ponk -lscnb -lamiga

# ${linklibs} \
 # 			"vlink": {
# 				"enabled": true,
# 				"command": "${config:amiga-assembly.binDir}/vlink",
# 				"includes": "*.{s,S,asm,ASM}",
# 				"excludes": "",
# 				"exefilename": "../uae/dh0/example",
# 				"entrypoint": "example.s",
# 				"args": [
# 					"-bamigahunk",
# 					"-Bstatic"
# 				]
# 			},

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
	if exist Obj\Ponk.o del Obj\Ponk.o
	if exist Obj\Action.o del Obj\Action.o
	if exist Obj\Operators.o del Obj\Operators.o
	if exist Obj\IH.o del Obj\IH.o
	if exist Obj\ActionRoutines.o del Obj\ActionRoutines.o
	if exist Obj\GroovyCon.o del Obj\GroovyCon.o
	if exist Obj\Sprites.o del Obj\Sprites.o
	if exist Obj\BensActionRoutines.o del Obj\BensActionRoutines.o
	if exist Obj\Scroll.o del Obj\Scroll.o
	if exist Ponk del Ponk