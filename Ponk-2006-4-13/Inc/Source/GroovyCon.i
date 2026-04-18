	include	"exec/types.i"

 STRUCTURE GroovyCon,0
	;Set these fields up yourself.
	APTR		gc_GfxMem
	ULONG		gc_BytesPerPlane
	ULONG		gc_BytesPerRow
	UWORD		gc_Depth
	UWORD		gc_ConWidth
	UWORD		gc_ConHeight
	APTR		gc_TextFont

	;These fields are private.
	UWORD		gc_CursorX
	UWORD		gc_CursorY
	LABEL		gc_SIZEOF

* Description of GroovyCon Struct *
* GfxMem	-	Pointer to the start of the consoles Graphics memory. This pointer can
*			be to an odd address.
* BytesPerPlane	-	Number of bytes to add to get to the next plane.
* BytesPerRow	-	Number of bytes to add to get to the row, this will as far as I can see
*			just be BytesPerPlane * Depth.
* Depth		-	Depth of the GfxMem buffer (1-8).
* ConWidth	-	Number of characters wide the console is.
* ConHeight	-	Number of characters high the console is.
* TextFont	-	Ptr to a Mono-spaced 8 point TextFont structure (ie Topaz.font 8)

 STRUCTURE ConsoleDevice,0
	APTR		cd_ConsoleStruct ;Ptr to the Console's Structure (ie, 'GroovyCon' Struct)
	APTR		cd_PutChar	;a0.l = Console Struct, d0.b = ASCII char value
	APTR		cd_Clear	;a0.l = Console Struct
	APTR		cd_CursorUp	;a0.l = Console Struct
	APTR		cd_CursorDown	;a0.l = Console Struct
	APTR		cd_CursorLeft	;a0.l = Console Struct
	APTR		cd_CursorRight	;a0.l = Console Struct
	APTR		cd_CursorSetX	;a0.l = Console Struct, d0.w = New X Pos
	APTR		cd_CursorSetY	;a0.l = Console Struct, d0.w = New Y Pos
	LABEL		cd_SIZEOF

 STRUCTURE MyMenu,0
	APTR	mn_Init
	APTR	mn_Update
	APTR	mn_Close
	LABEL	mn_SIZEOF
