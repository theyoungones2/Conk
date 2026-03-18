SOME DOCS FOR GONK
------------------

----------------------------------------------
The Various Windows
----------------------------------------------

Tools Window
------------

The tools buttons are arranged in three columns:

Left Column, from top to bottom:

- Custom Brush: Tells Gonk to use the current custom brush (if any) for
drawing.
- Draw: Standard drawing tool. Draw with left mouse button, erase with right.
- Line: Straight line tool.
- Rectangle: Draws an unfilled rectangle.
- Circle: Unfilled circle.
- Ellipse: Unfilled ellipse.
- Grab brush: Pick a rectangular area of picture up to use as a custom brush.
- FloodFill: Flood fills an enclosed area.
- Magnify: Toggles magnify mode on/off. Use 'I' and 'O' gadgets in right
border of main editing window to zoom in or out.
- Grid on/off: Toggles the grid on or off. When grid is on, drawing
operations allign themselves to the nearest eight-pixel boundary.
- Undo: Undoes the last operation (note: there is no way to undo an undo).

Middle Column (top to bottom):

- Animbrush: Use the current animbrush (if any) for drawing.
- Continuous draw: Same as 'Draw' except the points are joined up.
- Curve: Not implemented.
- Filled rectangle
- Filled circle
- Filled ellipse
- Pick up animbrush: Lets you pick up an area of picture over a number of
frames to use as an animbrush. Only works for bob or sprite type projects.
- Brush Handle: Allows you to reposition the handle on the current brush.
- Text: Not implemented.
- Pick Colour: Pick a colour from the picture. The colour can be picked
using any mouse button.
- Clear: Clears the picture.

Right Column

The first nine buttons in this column allow you to select the in-built
circle brushes of various sizes. The last button selects collision zone
editing mode for bobs and sprites (only it doesn't seem to work very
well at the moment)...


Colour Range Window
-------------------
From the Menus Select Window/Open/Range...

At the top of the window is the range drop box. An empty range will be shown
by a heap of X's in the range. Below that is the Range Scroller, because a
range consists of 256 cells, use the scroller to display the appropriate
section.

To setup a range, select the colour you want from the palette box on the left,
(the colour number will come up on the right) and 'drop' the colour in
appropriate cell in the range. To select multiple colours at once, you can
either drag out the range you want, or shift select the colours, and
then drop the range into the box.

Gadgets:
Revert -  Will return the range to way it was when the window was opened.
Undo - Will reverse the last action
Clear - Will Clear all the cells in the range
Reverse - Reverse the order of all the cells in the range
Ok - Accept the range and close the window
Range Number Slider - There are a total of 8 ranges that can used. Use this
slider to select the range you want.

Ranges are only used for two draw modes: Shade & Cycle.


Sequencer Window
----------------

The sequencer window lets you set up animations of bob/sprite images. The top
part of the window is divided into two areas. The left area displays the
currently selected bob, while the right area displays the current anim.
You can use '1' and '2' to select the image on the left. The anim can be
selected using the up and down cursor keys. The current frame within the
anim can can be changed using the horizontal cursor keys.

"Bob"
	The name of the currently selected bob image.

"Add Frame"
	Adds the selected bob onto the anim.

"Del Frame"
	Deletes the currently showing frame of the anim.

"Del Anim"
	Delete the entire selected anim.

"Hit Zones"
	Calculates hit zones for all the images in the anim. A bounding box is
calculated which emcompasses all the images. The collision zones for every bob
in the anim is set to this.

"Add Range"
	Add a range of images to the anim.

"Del Range"
	Delete a section of the anim.

"New Anim"
	Create a new anim.

"Play"
	Play the current anim.

"Name Bobs"
	Sets the names of all the images used in the anim. Each image takes the name
of the animation, appended with its frame number. If an image occurs in an anim
more than once, then the appended number is the number of the first appearance.

"Anim"
	Enter the name you want to give the anim here. This is the name used to
specify the anim in the game.

"Frame"
	The currently displayed frame of the anim.

"Bob"
	The number of the image used for the currently displayed frame of the anim.

"Spd"
	Sets the speed at which the anim is to be played. The number entered here
determines the number of frames for which each image is displayed. So, for
example, if you entered 5 here, each image in the anim would be shown for 5
frames before the next image were shown. A value of 1 gives the fastest
playback.

"X","Y"
	These two values specify how far the image is offset (in pixels). You can
type new values in here, or drag the current image (in the area on the
right) to the offset you want with the mouse. Every image in the anim has an
associated offset.

"Rel"
	When this box is ticked, the image offsets are automatically calculated
when you add images to the anim.
