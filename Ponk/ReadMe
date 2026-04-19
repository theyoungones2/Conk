## Prerequisites

Object (*.o) code for ModulePlayer, Spam and Lame must be copied into the Obj folder.
```
copy /MyPlay/code/ModulePlayer.o Obj/
copy /MyPlay/code/Spam.o Obj/
copy /Lame/code/#?.o Obj/
```

Code is configured for Vasm. It is recommended to use Visual Studio Code with the [Amiga Assembly](https://marketplace.visualstudio.com/items?itemName=prb28.amiga-assembly) extension. This comes with Vasm, and will offer lots of benefits including intellisense. See also [Amiga Assembly Wiki](https://github.com/prb28/vscode-amiga-assembly/wiki).  
In VSCode Settings (Ctrl + ,) Disable Extensions > Amiga Assembler > "Check Error On Save". This doesn't use the required Vasm settings and will give false errors.

[Makefile Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.makefile-tools) is probably also useful.

"Make" needs to be installed and in your PATH.

## Assemble

Edit [Makefile](./Makefile), you will likely need to modify vasmBinDir for your path to vasm.  
TODO: find a better solution.

### Make commands
`make` or `make all`  
Assemble all Ponk assembly file. Output to ./Obj

`make clear`  
Delete Ponk Object (*.o) files. Objects for ModulePlayer + Spam + Lame are kept.

## Link
From Amiga NewShell run "Glue" which will use Ponk.WITH to link.
Is Glue is not exectable run "protect Glue +s" to make it a script.


<details>
<summary>Legacy Devpac Instructions</summary>

Use 'smake' (comes with SAS) to assemble and link.
The lame and Spam/moduleplayer.o files need to be already present.

```
copy /MyPlay/code/ModulePlayer.o Obj/
copy /MyPlay/code/Spam.o Obj/
copy /Lame/code/#?.o Obj/
```

## How to create Libs.gs

The is the Global Symbol Table.  

1. Open Devpac
2. Open Inc/Source/Libs.i in the editor
3. From the **Program Menu:** Select **Output Symbols**. This will generate Libs.gs in the same folder
```
GenAm Macro Assembler Copyright © HiSoft 1985-93
All Rights Reserved - version 3.04

Assembling Libs.i
Pass 1
Pass 2

0 errors found
15165 lines assembled into 0 bytes, Gen symbol position-independent code
683600 bytes used
```
4. Close the Assembling windows
5. Quit Devpac

## GenAm.opts  
A definition for the GenAm Options

`-vow-`	Disable (vowel) - Turns OFF automatic optimization warnings  
`o+`	Optimize - Enables code optimization  
`alink`	Use Alink linker - Specifies Alink as the linker instead of the default  
`chkimm`	Check immediates - Validates immediate values for Motorola 68000 instructions  
`user`	User mode - Generates code for user mode execution (vs supervisor mode)  

</details>
