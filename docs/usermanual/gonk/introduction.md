------------------------------------------------------------------------------
Bob/Sprite Editing.
------------------------------------------------------------------------------

This is, for the most part, the same as editing animations in DPaint,
except for a few subtle differences.

All the editing modes are in the edit menu, and they all have appropriate
submenus (though only bobs and sprites have any useful submenus).

Gonk allocates the number of bob frames dynamically, so that when you start
editing there is only one frame, and as soon as you edit that first frame,
a second blank frame is allocated, so you can always be sure there is a
'next' frame as it were.

You use the '1' and '2' keys to change frames, and like DPaint you can
use shift to jump to the start or end of the frames.

You can edit a bob anywhere inside the project window, because when you
are finished editing, Gonk cuts the bob from the project, and stores it
internally in it's smallest possible form.

At the bottom the project window, is a string gadget. This is the bob
name, and can be changed by the normal means. Bob names can also be
set using the Animation Sequencer (explained later).

Bobs and Sprites have a 'Chunk Name', this different from the file name,
and can be 15 characters long. You can see the Chunk Name in square
brackets, in the window title bar. The name could be anything, as long as
every bob/sprite chunk has a different name. The Chunk Name is used so
that the animations know which chunk of bobs to use for the images.
To edit the name select 'Name Chunk...' from the Edit Menu. If the
name is [-<UNTITLED>-], as it is when you start a new project, then when
you go to save it, it will ask you for a name.

------------------------------------------------------------------------------
Sequencing Animations
------------------------------------------------------------------------------

Having a collection of bob frames made up in Gonk does not automatically
make them an animation. To make a bob animation you have to use the
Animation Sequencer. To open the Sequencer window, select from the menus
Window/Open/Sequencer...

At the top of the sequencer window are two image windows. The one on the
left shows a list of all the bobs in this bank (use '1' and '2' to cycle
this), and on the right is the animation window.

To add a frame to an animation, just set the bob window to the correct bob
and hit the 'Add Frame' button. This will add that bob to the animation
after the currently display frame (obviously at the start if the animtion
is empty), and the animation window will goto that new frame you just
added. So to put together a three frame animation, goto the first frame you
want, select 'Add Frame', press '2', select 'Add Frame', press '2', and select
'Add Frame'.

Cursor left/right steps through the frames of the current animation.

Cursor up/down lets you select other animations.

To see the current animation just select the 'Play' Button.

Animation Frames are of course usually made up in the correct order, so to
add a range of bobs into an animation, just select the 'Add Range' button.

The Speed of an animation can be set using the 'Spd' gadget. This is the
number of game cycles that you want to pause on one frame before moving to
the next frame (1 being the fastest).

As with everything in CONK, animations have names. This can be edited by
typing in the string gadget just below the animation window.

To speed up the process of naming bobs, there is a 'Name Bobs' button,
which will name all the bobs in the current animtion, with the animation
name, and frame suffix, so if an animation was called 'PLAYER' the bobs
would be called 'PLAYER1','PLAYER2',... 

To start a new animation select the 'New Anim' button. This will start
a new animation after the current one. The anim name gadget is automatically
activated when you start a new animation, so you might as well enter the
name first. The current animation number is just to the right of the animation
name.

When you save the animations it stores the chunk name of the correct bobs
to use for the animations. This lets Ponk link up the Bob chunk and Anim
chunk at runtime.


------------------------------------------------------------------------------
The Various Windows
------------------------------------------------------------------------------

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

Ranges are only used for two tools, Shade & Cycle.
