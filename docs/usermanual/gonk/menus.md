SOME DOCS FOR GONK
------------------

Project Menu
------------

"New"
	Opens a new project.

"Open..."
	Loads a project from a file. A requester with a number of checkboxes will
appear to let you select specifc chunks to load. If "Storage" is ticked, then
Gonk will store any chunk types it doesn't know how to deal with and save
them out again with the project.

"Save"
	Save a project to a file.

"Save As..."
	Save a project under a new name.

"Hide"
	Hide a project.

"Reveal..."
	Brings up a Reveal window displaying hidden projects.

"Close"
	Close a project.

"Page Size..."
	Lets you set the size of the projects bitmap.

"Screen Mode..."
	Change to another screen mode.

"Iconify..."
	Not implemented.

"About..."
	Bring up an About window.

"Quit Program..."
	Quit out of Gonk.

Edit Menu
---------

"Blocks"
	If ticked, the project is a Blockset.

"Bobs"
	If ticked, the project is a bank of Bob images.

"Sprites"
	If ticked, the project is a bank of Sprite images.

"Still Frame"
	If ticked, the project is an IFF ILBM picture.

"Blocks >>"
	Submenu - see below.

"Bobs >>"
	Submenu - see below.

"Sprites >>"
	Submenu - see below.

"Frame >>"

	Submenu - see below.
"Colour >>"

"Undo"
	Undoes the last drawing operaton. Same as the button in the tools window.

Blocks Submenu:
	This submenu is only a placeholder.

Bobs/Sprites Submenus:

"Previous"
	Go to the previous image.
"Next"
	Advance to the next image
"Goto..."
	Goto a specific image.
"Play..."
	Not implemented.
"Ping Pong..."
	Not implemented.
"Insert..."
	Lets you insert a number of blank images.
"Delete..."
	Deletes images.
"Copy..."
	Copys images.
"Edit Mask?"
	If ticked, then project is in mask editing mode. In this mode, images are
displayed in one colour. If a mask pixel is off, then the background will show
through in a game. By default, bob images don't have masks - the game just
uses colour zero to determine transparency. Images with a mask have an 'M'
appear beside their name in the window titlebar. When you enter mask editing
mode, a default mask is calculated.

Delete Mask"
	Delete an images mask.

"Remake Mask"
	Automatically make a mask, using colour zero as transparent.

"Edit Zone"
	Enters collision zone editing. By default, images don't have collision
zones, and a bounding box method is used ingame. However, for a lot of images
you will want to manually edit the collision zone. Images with custom zones
have a 'Z' in the titlebar. In editing mode, the zone appears as a rectangle.
Just drag it out to the size you want. When you enter Zone editing mode a
default bounding-box zone will be calculated. 
Zone editing is handled like a tool - to exit zone editing, select another
tool from the tools window.

"Delete Zone"
	Deletes an images zone.

"Remake Zone"
	Automatically calculate a zone for the image using a bounding box.

Frame Submenu:

	This submenu is just a placeholder.

Colour Submenu:

"Use Palette"
	Makes the screen use the projects palette.

"Remap"
	Remaps the project to another projects palette.

"Load Palette"
	Load a palette into this project.

"Save Palette"
	Save the projects palette.



Window Menu
-----------

"Open >>"
	Submenu - opens a specific type of window.

"Hide"
	Hide a window.

"Reveal..."
	Bring up a Reveal window to reveal hidden windows.

"Close"
	Close a window.

"Make Default"
	Store the current windows position/size as the default for that type of
window.

"Backdrop?"
"Borderless?"
"Title?"
"BorderGadgets?"
	Various toggleable window properties.

"Vertical?"
	Open available on the tools window. Determines whether tools are laid out
horizontally or vertically.

Prefs Menu
----------

"Save Chunky?"
	A testing option - leave unselected!

"Make Backups?"
	Toggles the creation of ".bak" files.

"Auto Activate?"
	If selected, Gonk windows auto-activate themselves when you move the mouse
over them.

"Keep Screen Mode?"
	Stops Gonk automatically changing screenmodes when it loads in new projects.

"Exclude Brush?"
	Stops the brush tool from picking up the right and bottom rows of pixels
when the grid is on.

"Show Window IDs?"
	Obsolete item.

"Create Icons?"
	Not implemented.

"Screen Settings..."
	Brings up a screen settings window.

"Load Settings..."
	Load a settings file.

"Save Settings"
	Save the current settings as the default.

"Save Settings As..."
	Save the current settings into a different file.

Mode Menu
---------

"Matte"
	In this drawing mode, tools draw using colours from the brush.

"Colour"
	Draw using the selected colour from the palette.

"Replace"
	Draw the brush without masking - transparent areas on the brush are
ignored.

"Shade"
	Only affects colours in the currently selected range. If drawing with the
left mouse button, a colour will be replaced by the colour one position to
the left of it in the range. With the right mouse button, colours are
replaced by their right neighbours in the range. See the section on the
range window.

"Smooth"
	Averages out colours - sharp contrasts are smoothed by using intermediate
colours.

"Cycle"
	Each subsequent drawing operation uses the next colour in the currently
selected range (see the range window).

"Dummy"
"Dummy"
"Dummy"
	Placeholder items.

Brush Menu
----------

"AnimBrush >>"
	Animbrush submenu. See below.

"Load..."
	Load a brush.

"Save..."
	Save the current brush.

"Copy"
	Copy the brush to the Clipboard.

"Paste"
	Copy a brush from the Clipboard

"Restore"
	Restore the brush.

"Size >>"
	The Size submenu. See below.

"Flip >>"
	The Flip submenu. See below.

"Outline"
	Outlines the current brush with the current drawing colour.

"Handle >>"
	The Handle submenu. See below.

"Free"
	Discard the current brush.

Animbrush SubMenu:

"Load..."
	Load an animbrush.

"Save As..."
	Save the animbrush.

"Pick Up..."
	Pick up a sequence of bob images as an animbrush.

"Settings..."
	Change the animbrush settings (ping-pong etc...).

"Use"
	Use the current animbrush.

"Free"
	Discard the animbrush.

Size SubMenu:

"Stretch"
	Not implemented.

"Halve"
	Halves the size of the current brush.

"Double"
"Double Horiz"
"Double Vert"
	Not implemented.

Flip SubMenu:

"Horizontal"
	Flip the current brush horizontally.

"Vertical"
	Flip the current brush vertically.

"Rotate 90�"
	Rotate the current brush by 90�.

Handle Submenu:

"Rotate"
	Moves the brush handle to one of the corners of the brush. After all four
corners have been done, the next "Rotate" will place the handle in the
centre of the brush.

"Place"
	Lets you place the handle at an arbitrary position on the brush.
