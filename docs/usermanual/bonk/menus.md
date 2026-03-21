# BONK - Menus

## Project Menu

**New**  
Create a new map. You can specify the size of the map using a requester.

**Open...**  
Load a Map (and possible a Blockset and palette) from a file.

**Save**  
Save a Map (Blockset/palette optional).

**Save As...**  
Save under a name specified using a file requester.

**Hide**  
Hide the currently selected Map window (and all other windows displaying
the same map, if any).

**Reveal**  
Bring up a Reveal window to reveal hidden Map windows.

**Close**  
Close the Map in the currently active Map window.

**About**  
Open the About window.

**Quit Program...**
Die.


## Edit Menu

**Undo**  
Undoes the last operation. Same as the "Undo" button in the Tools window.

**Load Blocks...**  
Load a Blockset from a file. The file can be a standard Conk file, raw
data or an IFF ILBM. ILBMs are imported and cut up into blocks. If a
palette is included you are given the oportunity of loading it.

**Load Palette...**  
Load a palette from a file. Conk files and IFF ILBMs are supported.

## Map Menu

This menu only appears on Map windows, and its commands apply to the Map in
the active window.

**Fix Background?**  
This is a checkmark menu item. When you check it, the contents of the Map
at that time are locked. You can still draw over them as normal, but whenever
a block is erased (ie drawn with the block value assigned to the right mouse
button), the block that originally occupied that position is redrawn. This
applies to all drawing tools.

**Resize Map...**  
This lets you alter the dimensions of the Map. If you specify new
dimensions smaller than the old ones, the right and/or bottom edges of the
Map will be clipped. If you expand the Map, the new area will be filled with
block zero.

**Rename Map...**  
Lets you alter the Map name. This is the name that is stored in the chunk,
**not** the filename.

**Go To Bookmark**  
Jumps the display in the Map window to one of the ten user-defined bookmark
positions. Pressing the number key corresponding to the bookmark you want to
go to while holding down the right Amiga key has the same effect.

**Set Bookmark**  
Stores the current position of the Map window as one of the bookmark
positions. This can also be done by holding down the right Amiga key and
SHIFT, then pressing the number key corresponding to the bookmark you want
to set.

## Window Menu

**Open**  
Opens a new window. The window type is selected from the submenu.

**Hide**  
Hides the current window.

**Reveal**  
Brings up a reveal window to show hidden windows.

**Close**  
Close the current window.

**Backdrop?**  
If ticked, the current window is made into a Backdrop window.
Backdrop windows are always behind other windows.

**Borderless?**  
Decides if the current window should have a border or not. Windows
without borders can look messy unless you turn off the BorderGadgets
and Title.

**Title?**  
Toggles the title bar of the current window on or off.

**BorderGadgets?**  
Allows you to remove the border gadgets of the current window.

## Settings Menu

**Make Backups?**  
When this option is set, backup (".bak") files will be made. When you
open a file, make some changes and save it back over itself, the old
file will be copied to a file of the same name appended with ".bak".
It's generally a good idea to keep backups on.

**"Set Settings...**  
Brings up the Screen Settings window.

**Load Settings...**  
Load and use settings file.

**Save Settings...**  
Save the current settings as the default "Bonk.cfg".

**Save Settings As...**  
Save the current settings under a different filename.

## Blockset Menu

This menu only appears with the Blocks window, and has only one option,
"Rename Blockset". When selected, a requester comes up to let you change the
name of the Blockset. This is the name used to reference the Blockset in a
game.
