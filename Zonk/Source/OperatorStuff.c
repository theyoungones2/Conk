/********************************************************/
//
// OperatorStuff.c
//
/********************************************************/

#define OPERATORSTUFF_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"

#define VARNAMESIZE 10

BOOL EditOperator( struct Window *parentwin, UBYTE *opdata );
struct HoopyObject *CreateOpEditLayout( UBYTE *opdata, UWORD pagenum,
	BOOL nextbutton, BOOL prevbutton );
VOID FreeOpEditLayout( struct HoopyObject *hob );
static UWORD PageObjectCount( UBYTE *opdata, UWORD pagenum );
UBYTE *FindOpParam( UBYTE *opdata, UWORD num );
VOID ReadOpParamsFromLayout( struct HoopyObject *hob, UBYTE *opdata, UWORD pagenum );

static VOID HandleSpecialGadget( struct Window *win, UBYTE *opbase,
	UWORD paramnum, UWORD specialcode );
static BOOL InitOpData( UBYTE *opdata );

static UBYTE *nulllabels[] = { "" };
static UBYTE *xspeedanimlabels[] = { "LeftThreshold", "LeftAnim", "RightThreshold", "RightAnim", "NeutralAnim" };
static UBYTE *linearaccellabels[] = { "Initial XSpd", "Initial YSpd", "XAccel", "YAccel", "", "Ignore Initial Spds" };
static UBYTE *initasteroidsplayerlabels[] = { "Joystick", "RotSpeed","FwdAccel","RevAccel","MaxSpeed","Drag","FireActionList","","FireWeaponBays?\0AutoFireRepeat?" };
static UBYTE *initinertialplayerlabels[] = { "Joystick", "XAccel","YAccel", "MaxXSpd", "MaxYSpd", "XDrag", "YDrag",
	"FireActionList", "", "Lock dir while firing?\0FireWeaponBays?\0AutoFireRepeat?", "Gravity","BaseXSpd","SetDirection","AxisFreedom" };
static UBYTE *firebulletlabels[] = { "Image", "X Offset", "Y Offset", "ActionList" };
static UBYTE *executelabels[] = { "ActionList" };
static UBYTE *cyclicanimlabels[] = {"Anim", "Blank when done?","Startframe", "Cycles" };
static UBYTE *OLDdir16animlabels[] = {
	"Anim0", "Startframe", "Repeats",
	"Anim1", "Startframe", "Repeats",
	"Anim2", "Startframe", "Repeats",
	"Anim3", "Startframe", "Repeats",
	"Anim4", "Startframe", "Repeats",
	"Anim5", "Startframe", "Repeats",
	"Anim6", "Startframe", "Repeats",
	"Anim7", "Startframe", "Repeats",
	"Anim8", "Startframe", "Repeats",
	"Anim9", "Startframe", "Repeats",
	"Anim10", "Startframe", "Repeats",
	"Anim11", "Startframe", "Repeats",
	"Anim12", "Startframe", "Repeats",
	"Anim13", "Startframe", "Repeats",
	"Anim14", "Startframe", "Repeats",
	"Anim15", "Startframe", "Repeats",
	"NeutralAnim", "Startframe", "Repeats" };
static UBYTE *obsbgcollisionlabels[] = { "Last Space Block", "Action On Collision", "Shifts on Collision" };
//static UBYTE *inertialjoymovelabels[] = {	"Max XSpd", "Max YSpd", "XAccel","YAccel", "XDrag", "YDrag", "Input Channel",
//																				  "FireFlags", "Flags", "Left Action", "Right Action", "Up Action", "Down Action"};
static UBYTE *inertialjoymovelabels[] = {	"Max XSpd", "Max YSpd", "XAccel","YAccel", "XDrag", "YDrag", "Input Channel",
  "Fire Bay 15\0Fire Bay 14\0Fire Bay 13\0Fire Bay 12\0"
	"Fire Bay 11\0Fire Bay 10\0Fire Bay 9\0Fire Bay 8\0",
	"Fire Bay 7\0Fire Bay 6\0Fire Bay 5\0Fire Bay 4\0"
	"Fire Bay 3\0Fire Bay 2\0Fire Bay 1\0Fire Bay 0",
	"Flags", "Left Action", "Right Action", "Up Action", "Down Action"};
static UBYTE *scrollconstantxspeedlabels[] = {	"Speed", "ScrollBackDistance", "ScrollBackSpeed","ScrollBackZoneWidth","EndOfMapActionList","","" };
//static UBYTE *setxposlabels[] = { "Xpos" };
static UBYTE *statprintvarlabels[] = { "XPos","YPos","Variable" };
//static UBYTE *setyposlabels[] = { "Ypos" };
static UBYTE *platformplayerlabels[] = { "Joystick","XAccel","MaxXSpd","XDrag","ClimbSpeed",
	"JumpSpeed","Gravity","MaxFallSpeed","EdgeOfTheRoad","","No mid-air firing?\0Fire all WeaponBays?\0Auto fire repeat?",
	"FireActionList","CollisionLeft","CollisionTop",
	"CollisionWidth","CollisionHeight",
	"WalkLeftAnim","WalkRightAnim","FaceLeftAnim","FaceRightAnim",
	"JumpLeftAnim","JumpRightAnim","ClimbUpAnim","ClimbDownAnim" };
static UBYTE *setimagelabels[] = { "Image" };
static UBYTE *directionalhomerlabels[] = { "Type", "Player", "Quadrant Bits - TL,TR,BR,BL", "Homeless Action"};
static UBYTE *pushscrolltrackxlabels[] = { "Player to track","Left Push Pos","Right Push Pos","Max Speed" };
static UBYTE *pushscrolltrackylabels[] = { "Player to track","Top Push Pos","Bottom Push Pos","Max Speed" };
static UBYTE *initscrolllabels[] = { "X Position","Y Position","","","","","","","","" };
static UBYTE *fireweaponbaylabels[] = { "Weapon Bay" };
static UBYTE *powerupbaylabels[] = { "Weapon Bay","DefaultWeapon" };
static UBYTE *powerdownbaylabels[] = { "Weapon Bay","","KeepLowest" };
static UBYTE *setweaponbaylabels[] = { "Weapon Bay","Weapon" };
static UBYTE *setchannellabels[] = { "Channel","Routine","Timer","DoneActionList" };
static UBYTE *highlightboblabels[] = { "Duration" };
static UBYTE *setcolourlabels[] = { "ColourIndex", "Red","Green","Blue" };
static UBYTE *killchannellabels[] = { "Channel" };
static UBYTE *initmouseplayerlabels[] = { "Mouse","XAccel","YAccel","MaxXSpd","MaxYSpd","XDrag","YDrag",
	"FireActionList","","DirectMovement?\0FireAllWeaponBays?\0AutoFireRepeat?",
	"AxisFreedom","BaseXSpd","Gravity" };
static UBYTE *initactionlistplayerlabels[] = { "Joystick","","",
	"UpPressed", "UpRelease", "DnPressed", "DnRelease",
	"LfPressed", "LfRelease", "RtPressed", "RtRelease",
	"Fire0Pressed", "Fire0Release", "Fire1Pressed", "Fire1Release",
	"Fire2Pressed", "Fire2Release", "Fire3Pressed", "Fire3Release",
	"",
	"RepeatFire3Press?\0RepeatFire2Press?\0RepeatFire1Press?\0RepeatFire0Press?\0"
	"RepeatRtPress?\0RepeatLfPress?\0RepeatDnPress?\0RepeatUpPress?" };
static UBYTE *initcheckfirebuttonslabels[] = { "Joystick","","",
	"Fire0Pressed", "Fire0Release", "Fire1Pressed", "Fire1Release",
	"Fire2Pressed", "Fire2Release", "Fire3Pressed", "Fire3Release",
	"",
	"RepeatFire3Press?\0RepeatFire2Press?\0RepeatFire1Press?\0RepeatFire0Press?" };
static UBYTE *yspeedanimlabels[] = { "UpThreshold", "UpAnim", "DownThreshold", "DownAnim", "NeutralAnim" };
static UBYTE *playsfxlabels[] = { "Effect"," AudioChannel" };
static UBYTE *oldplaysfxlabels[] = { "Effect"," AudioChannel" };
static UBYTE *initinertialhomerlabels[] = { "Target","","","","TrackNearestPlayer?",
	"XAccel","YAccel","MaxXSpd","MaxYSpd" };
static UBYTE *statprintscorelabels[] = { "XPos","YPos","Player" };
static UBYTE *oldinitinertialhominglabels[] = { "Target", "TargetPlayer", "MaxXSpd", "MaxYSpd", "XAccel", "YAccel" };
static UBYTE *timedelayexecutelabels[] = { "ActionList", "Delay", "Repeats (0=forever)" };
static UBYTE *followpathlabels[] = { "Path", "","Zero speeds at end?\0Ignore path StartPos?" };
static UBYTE *olddirectionmovelabels[] = { "SpeedMagnitude" };
static UBYTE *channelexecutelabels[] = { "ActionList", "Delay" };
static UBYTE *chdirectionmovelabels[] = { "SpeedMagnitude", "Delay" };
static UBYTE *olddir8animlabels[] = {
	"Anim0", "Startframe",
	"Anim1", "Startframe",
	"Anim2", "Startframe",
	"Anim3", "Startframe",
	"Anim4", "Startframe",
	"Anim5", "Startframe",
	"Anim6", "Startframe",
	"Anim7", "Startframe"};
static UBYTE *olddir16animlabels[] = {
	"Anim0", "Startframe",
	"Anim1", "Startframe",
	"Anim2", "Startframe",
	"Anim3", "Startframe",
	"Anim4", "Startframe",
	"Anim5", "Startframe",
	"Anim6", "Startframe",
	"Anim7", "Startframe",
	"Anim8", "Startframe",
	"Anim9", "Startframe",
	"Anim10", "Startframe",
	"Anim11", "Startframe",
	"Anim12", "Startframe",
	"Anim13", "Startframe",
	"Anim14", "Startframe",
	"Anim15", "Startframe" };
static UBYTE *olddir32animlabels[] = {
	"Anim0", "Startframe",
	"Anim1", "Startframe",
	"Anim2", "Startframe",
	"Anim3", "Startframe",
	"Anim4", "Startframe",
	"Anim5", "Startframe",
	"Anim6", "Startframe",
	"Anim7", "Startframe",
	"Anim8", "Startframe",
	"Anim9", "Startframe",
	"Anim10", "Startframe",
	"Anim11", "Startframe",
	"Anim12", "Startframe",
	"Anim13", "Startframe",
	"Anim14", "Startframe",
	"Anim15", "Startframe",
	"Anim16", "Startframe",
	"Anim17", "Startframe",
	"Anim18", "Startframe",
	"Anim19", "Startframe",
	"Anim20", "Startframe",
	"Anim21", "Startframe",
	"Anim22", "Startframe",
	"Anim23", "Startframe",
	"Anim24", "Startframe",
	"Anim25", "Startframe",
	"Anim26", "Startframe",
	"Anim27", "Startframe",
	"Anim28", "Startframe",
	"Anim29", "Startframe",
	"Anim30", "Startframe",
	"Anim31", "Startframe"};
static UBYTE *olddir64animlabels[] = {
	"Anim0", "Startframe",
	"Anim1", "Startframe",
	"Anim2", "Startframe",
	"Anim3", "Startframe",
	"Anim4", "Startframe",
	"Anim5", "Startframe",
	"Anim6", "Startframe",
	"Anim7", "Startframe",
	"Anim8", "Startframe",
	"Anim9", "Startframe",
	"Anim10", "Startframe",
	"Anim11", "Startframe",
	"Anim12", "Startframe",
	"Anim13", "Startframe",
	"Anim14", "Startframe",
	"Anim15", "Startframe",
	"Anim16", "Startframe",
	"Anim17", "Startframe",
	"Anim18", "Startframe",
	"Anim19", "Startframe",
	"Anim20", "Startframe",
	"Anim21", "Startframe",
	"Anim22", "Startframe",
	"Anim23", "Startframe",
	"Anim24", "Startframe",
	"Anim25", "Startframe",
	"Anim26", "Startframe",
	"Anim27", "Startframe",
	"Anim28", "Startframe",
	"Anim29", "Startframe",
	"Anim30", "Startframe",
	"Anim31", "Startframe",
	"Anim32", "Startframe",
	"Anim33", "Startframe",
	"Anim34", "Startframe",
	"Anim35", "Startframe",
	"Anim36", "Startframe",
	"Anim37", "Startframe",
	"Anim38", "Startframe",
	"Anim39", "Startframe",
	"Anim40", "Startframe",
	"Anim41", "Startframe",
	"Anim42", "Startframe",
	"Anim43", "Startframe",
	"Anim44", "Startframe",
	"Anim45", "Startframe",
	"Anim46", "Startframe",
	"Anim47", "Startframe",
	"Anim48", "Startframe",
	"Anim49", "Startframe",
	"Anim50", "Startframe",
	"Anim51", "Startframe",
	"Anim52", "Startframe",
	"Anim53", "Startframe",
	"Anim54", "Startframe",
	"Anim55", "Startframe",
	"Anim56", "Startframe",
	"Anim57", "Startframe",
	"Anim58", "Startframe",
	"Anim59", "Startframe",
	"Anim60", "Startframe",
	"Anim61", "Startframe",
	"Anim62", "Startframe",
	"Anim63", "Startframe"};
static UBYTE *oldinitdir8imagelabels[] = {"Anim"};
static UBYTE *oldinitdir16imagelabels[] = {"Anim"};
static UBYTE *oldinitdir32imagelabels[] = {"Anim"};
static UBYTE *oldinitdir64imagelabels[] = {"Anim"};
static UBYTE *oldactionlistplayerlabels[] = {
	"Input Channel", "Up Action", "Down Action", "Left Action", "Right Action", "Flags" };
static UBYTE *spawnplayerbulletlabels[] = {"Image","XOffset","YOffset","ActionList"};
static UBYTE *spawnbaddudelabels[] = {"Image","XOffset","YOffset","ActionList"};
static UBYTE *spawnplayerlabels[] = {"Image","XOffset","YOffset","ActionList"};
static UBYTE *dir8actionlabels[] = {"Prog1","Prog2","Prog3","Prog4","Prog5","Prog6","Prog7","Prog8"};
static UBYTE *dir16actionlabels[] = {"Prog1","Prog2","Prog3","Prog4","Prog5","Prog6","Prog7","Prog8",
																		 "Prog9","Prog10","Prog11","Prog12","Prog13","Prog14","Prog15","Prog16"};
static UBYTE *dir32actionlabels[] = {"Prog1","Prog2","Prog3","Prog4","Prog5","Prog6","Prog7","Prog8",
																		 "Prog9","Prog10","Prog11","Prog12","Prog13","Prog14","Prog15","Prog16",
																		 "Prog17","Prog18","Prog19","Prog20","Prog21","Prog22","Prog23","Prog24",
																		 "Prog25","Prog26","Prog27","Prog28","Prog29","Prog30","Prog31","Prog32"};
static UBYTE *dir64actionlabels[] = {"Prog1","Prog2","Prog3","Prog4","Prog5","Prog6","Prog7","Prog8",
																		 "Prog9","Prog10","Prog11","Prog12","Prog13","Prog14","Prog15","Prog16",
																		 "Prog17","Prog18","Prog19","Prog20","Prog21","Prog22","Prog23","Prog24",
																		 "Prog25","Prog26","Prog27","Prog28","Prog29","Prog30","Prog31","Prog32",
																		 "Prog33","Prog34","Prog35","Prog36","Prog37","Prog38","Prog39","Prog40",
																		 "Prog41","Prog42","Prog43","Prog44","Prog45","Prog46","Prog47","Prog48",
																		 "Prog49","Prog50","Prog51","Prog52","Prog53","Prog54","Prog55","Prog56",
																		 "Prog57","Prog58","Prog59","Prog60","Prog61","Prog62","Prog63","Prog64"};
static UBYTE *oldstatprintscorelabels[] = {"X Pos","Y Pos","Player"};
static UBYTE *printglobalscorelabels[] = {"X Pos","Y POs","Flags"};
static UBYTE *givescorelabels[] = {"Score"};
static UBYTE *endlevellabels[] = {"Next Level"};
static UBYTE *rndxspeedlabels[] = {"Shifts"};
static UBYTE *rndyspeedlabels[] = {"Shifts"};
static UBYTE *oldasteroidsplayerlabels[] = {"Joystick", "TurnSpeed", "MaxSpeed", "Acceleration", "Drag", "DownActionlist"};
static UBYTE *setborderslabels[] = {"Left Border", "Left Type", "Right Border", "Right Type", "Top Border", "Top Type", "Bottom Border", "Bottom Type", "",
	"Left ViewRelative?\0Right ViewRelative?\0Top ViewRelative?\0Bottom ViewRelative?" };
static UBYTE *rnddirectionmove[] = {"Speed"};
static UBYTE *setvarlabels[] = {"Var", "Value"};
static UBYTE *addtovarlabels[] = {"Var", "Value"};
static UBYTE *subfromvarlabels[] = {"Var", "Value"};
static UBYTE *executeifvarcclabels[] = {"Var", "Value", "Condition - EQ,NE", "Action"};
static UBYTE *olddir8imagelabels[] = {"Anim","Offset"};
static UBYTE *olddir16imagelabels[] = {"Anim","",""};
static UBYTE *dir8imagelabels[] = {
	"0 ( Up )","1 (UpRt)","2 ( Rt )","3 (DnRt)",
	"4 ( Dn )","5 (DnLf)","6 ( Lf )","7 (UpLf)","Anim","","" };
static UBYTE *dir16imagelabels[] = {
	"0 ( Up )", "0:16", "1 (UpRt)", "1:16", "2 ( Rt )", "2:16", "3 (DnRt)", "3:16",
	"4 ( Dn )", "4:16", "5 (DnLf)", "5:16", "6 ( Lf )", "6:16", "7 (UpLf)", "7:16",
	"Anim","","" };
static UBYTE *dir32imagelabels[] = {"Anim","",""};
static UBYTE *dir64imagelabels[] = {"Anim","",""};
static UBYTE *rotateclockwiselabels[] = {"Amount"};
static UBYTE *rotateanticlockwiselabels[] = {"Amount"};
static UBYTE *xspeedfromlistlabels[] = {"Value","Value","Value","Value","Value","Value","Value","Value",
																				 "Value","Value","Value","Value","Value","Value","Value","Value",
																				 "Value","Value","Value","Value","Value","Value","Value","Value",
																				 "Value","Value","Value","Value","Value","Value","Value","Value" };
static UBYTE *yspeedfromlistlabels[] = {"Value","Value","Value","Value","Value","Value","Value","Value",
																				 "Value","Value","Value","Value","Value","Value","Value","Value",
																				 "Value","Value","Value","Value","Value","Value","Value","Value",
																				 "Value","Value","Value","Value","Value","Value","Value","Value" };
static UBYTE *setdirectionlabels[] = {"Direction 0 - 256"};
static UBYTE *attachlabels[] = {"Player", "XOffset", "YOffset"};
static UBYTE *setmiscstufflabels[] = {"Player HitRoutine","PlayerBullet HitRoutine","BadDude HitRoutine","Damage","Shields","Death Action","","" };
static UBYTE *operationwolfmovementlabels[] = {"Max X Speed", "Max Y Speed", "Input Channel", "Fire Flags - Bits for bays to fire", "Flags - l,r,t,b,m/a,s/f"};
static UBYTE *repellabels[] = {"X Shifts", "Y Shifts"};
static UBYTE *setilabels[] = {"Value", "Variable"};
static UBYTE *addilabels[] = {"Value", "Variable"};
static UBYTE *setvlabels[] = {"Source Var", "Dest Var"};
static UBYTE *addvlabels[] = {"Source Var", "Dest Var"};
static UBYTE *oldsetrndlabels[] = {"Shifts", "XY,XYS,XYM,XYA,XYD,SD,12"};
static UBYTE *oldaddrndlabels[] = {"Shifts", "XY,XYS,XYM,XYA,XYD,SD,12"};
static UBYTE *collisionpartnerexecutelabels[] = {"ActionList"};
static UBYTE *joystickanimationlabels[] = {"Input Channel", "Flags",
					"Neutral Anim", "Up Anim","Down Anim","Left Anim","Right Anim",
					"Up & Right Anim","Down & Right Anim","Up & Left Anim","Down & Left Anim",
					"Fire Anim", "Fire & Up Anim","Fire & Down Anim","Fire & Left Anim","Fire & Right Anim",
					"Fire, Up & Right Anim","Fire, Down & Right Anim","Fire, Up & Left Anim","Fire, Down & Left Anim" };

//static UBYTE *setobjectactionlabels[] = {"ActionList", "Desination Var - HP,HPB,HBD,Death" };
static UBYTE *setobjectactionlabels[] = {"ActionList", "Upon" };
static UBYTE *setcollisionslabels[] = {"Collisions" };
static UBYTE *playmodulelabels[] = { "Module Name" };
static UBYTE *actiononfirelabels[] = { "Input Channel", "Delay", "Action" };

static UBYTE *comparevariablelabels[] = { "ValueVar", "VarToCheck" };
static UBYTE *compareimediatelabels[] = { "Value", "VarToCheck" };
static UBYTE *ifequallabels[] = { "Actionlist" };
static UBYTE *ifnotequallabels[] = { "Actionlist" };
static UBYTE *ifgreaterthanlabels[] = { "Actionlist" };
static UBYTE *iflowerthanlabels[] = { "Actionlist" };
static UBYTE *backgroundpointcollisionlabels[] = { "Last Space Block", "Action List", "Shifts Right", "X Offset", "Y Offset" };
static UBYTE *simpleplatformplayerlabels[] = { "Input Channel", "XAccel", "Gravity", "Jump Speed", "Max X Spd", "Max Fall Spd", "Last Space Block", "Last Floor Block", "", "Turn during jump?" };
static UBYTE *executeonplayerlabels[] = { "Player", "Actionlist" };
static UBYTE *executeifonscreenlabels[] = {"Action List"};
static UBYTE *initattachtoparentlabels[] = {"XOffset","YOffset"};
static UBYTE *drawbarlabels[] = {"XPos","YPos","Width","MaxVal","Var" };
static UBYTE *floorsentrylabels[] = {"LeftSpd","RightSpd","EdgeOfTheRoad","HighestSpaceVal","Check Floor?\0Start Left?\0Set 'Direction'?" };
static UBYTE *ceilingsentrylabels[] = {"LeftSpd","RightSpd","EdgeOfTheRoad","HighestSpaceVal","Check Ceiling?\0Start Left?\0Set 'Direction'?" };
static UBYTE *initlinearplayerlabels[] = {	"Joystick", "XSpd", "YSpd",	"FireActionList",
	"","Fire to lock dir?\0FireAllWeaponBays?\0AutoFireRepeat?", "SetDirection" };


static UBYTE *bgpointcollisionlabels[] = {"XOffset","YOffset","EdgeOfTheRoad","HighestSpaceVal","","BoingActionList", "BounceSpd" };
static UBYTE *bgcollisionlabels[] = { "EdgeOfTheRoad","HighestSpaceVal","","BoingActionList", "BounceSpd" };

static UBYTE *trackplayerlabels[] = { "Player", "Speed", "TurnSpeed", "", "Direct tracking?" };
static UBYTE *directionmovelabels[] = { "SpeedMagnitude","","Add to current speeds?" };
static UBYTE *initjitterspeedlabels[] = { "XRndMin","XRndMax","MaxXSpeed",
	"YRndMin","YRndMax","MaxYSpeed", "WhichAxis","Rate (0=Full)" };
static UBYTE *setrndlabels[] = { "MinValue","MaxValue","Variable" };
static UBYTE *addrndlabels[] = { "MinValue","MaxValue","Variable" };
static UBYTE *scrollxcenterplayerslabels[] = { "MaxScrollSpeed","Track","","","","","","" };

static UBYTE *initdir8imagelabels[] = { "0 ( Up )","1 (UpRt)","2 ( Rt )","3 (DnRt)",
	"4 ( Dn )","5 (DnLf)","6 ( Lf )","7 (UpLf)","Anim","","" };

static UBYTE *initdir16imagelabels[] = {
	"0 ( Up )", "0:16", "1 (UpRt)", "1:16", "2 ( Rt )", "2:16", "3 (DnRt)", "3:16",
	"4 ( Dn )", "4:16", "5 (DnLf)", "5:16", "6 ( Lf )", "6:16", "7 (UpLf)", "7:16",
	"Anim","","" };

static UBYTE *initdir32imagelabels[] = { "Anim","","" };
static UBYTE *initdir64imagelabels[] = { "Anim","","" };

static UBYTE *initdir8animlabels[] = { "","Pause at zero speed?\0Null anims pause?\0OneShot anims?",
	"0 ( Up )","1 (UpRt)","2 ( Rt )","3 (DnRt)",
	"4 ( Dn )","5 (DnLf)","6 ( Lf )","7 (UpLf)" };
static UBYTE *initdir16animlabels[] = { "","Pause at zero speed?\0Null anims pause?\0OneShot anims?",
	"0 ( Up )", "0:16", "1 (UpRt)", "1:16", "2 ( Rt )", "2:16", "3 (DnRt)", "3:16",
	"4 ( Dn )", "4:16", "5 (DnLf)", "5:16", "6 ( Lf )", "6:16", "7 (UpLf)", "7:16" };

static UBYTE *initdir32animlabels[] = { "","Pause at zero speed?\0Null anims pause?\0OneShot anims?",
	"0 ( Up )", "0:8 ", "0:16", "0:24",
	"1 (UpRt)", "1:8 ", "1:16", "1:24",
	"2 ( Rt )", "2:8 ", "2:16", "2:24",
	"3 (DnRt)", "3:8 ", "3:16", "3:24",
	"4 ( Dn )", "4:8 ", "4:16", "4:24",
	"5 (DnLf)", "5:8 ", "5:16", "5:24",
	"6 ( Lf )", "6:8 ", "6:16", "6:24",
	"7 (UpLf)", "7:8 ", "7:16", "7:24" };

static UBYTE *initdir64animlabels[] = { "","Pause at zero speed?\0Null anims pause?\0OneShot anims?",
	"0 ( Up )", "0:4", "0:8 ", "0:12", "0:16", "0:20", "0:24", "0:28",
	"1 (UpRt)", "1:4", "1:8 ", "1:12", "1:16", "1:20", "1:24", "1:28",
	"2 ( Rt )", "2:4", "2:8 ", "2:12", "2:16", "2:20", "2:24", "2:28",
	"3 (DnRt)", "3:4", "3:8 ", "3:12", "3:16", "3:20", "3:24", "3:28",
	"4 ( Dn )", "4:4", "4:8 ", "4:12", "4:16", "4:20", "4:24", "4:28",
	"5 (DnLf)", "5:4", "5:8 ", "5:12", "5:16", "5:20", "5:24", "5:28",
	"6 ( Lf )", "6:4", "6:8 ", "6:12", "6:16", "6:20", "6:24", "6:28",
	"7 (UpLf)", "7:4", "7:8 ", "7:12", "7:16", "7:20", "7:24", "7:28" };

static UBYTE *restrictdirectionlabels[] = { "Restriction","Min","Max" };
static UBYTE *randomeventlabels[] = { "Probability (0..255)","ActionList","","" };
static UBYTE *initrandomeventlabels[] = { "Probability (0..255)","ActionList","","" };
//room for mousebutton stuff
static UBYTE *initmousepointerlabels[] = { "MousePort","","","Speed","AxisFreedom","","","","",
	"","","","","","","","","","","","","","","","","","","","","","","","","","" };
static UBYTE *scrollycenterplayerslabels[] = { "MaxScrollSpeed","Track","","","","","","" };

/************************/

static UBYTE *offoncycnames[] = { "Off","On",NULL };
static UBYTE *channelcycnames[] = { "Channel 0","Channel 1",
	"Channel 2","Channel 3","Channel 4",NULL };
static UBYTE *playercycnames[] = { "Player 0","Player 1","Player 2","Player 3", NULL };
static UBYTE *trackplayerscycnames[] = { "Player 0","Player 1","Player 2","Player 3", "All Players", NULL };
static UBYTE *objactioncycnames[] = { "HitPlayer","HitPlayerBullet","HitBadDude","Death",NULL };
static UBYTE *dampingcycnames[] = { "Stop","Full","Half","Quarter",NULL };
static UBYTE *bordertypecycnames[] = { "Off","Kill","Stop","Wrap","Bounce1","Bounce2", NULL };
static UBYTE *restrictdirectioncycnames[] = { "Min-Max", "Left/Right", "Up/Down", "4-Way", "8-Way", NULL };

static UBYTE *axisfreedomcycnames[] = { "X & Y", "X Only", "Y Only", NULL };
static UBYTE *setdircycnames[] = { "8-Way", "Left/Right", "Up/Down", "No", NULL };
static UBYTE *homingtargetcycnames[] = { "Player","Nearest Player","Nearest BadDude", NULL };

static UBYTE *asteroidsdragcycnames[] = { "Total","Lots & lots","Quite a bit","Medium","Wee bit","Least",NULL };
static UBYTE *accel5cycnames[] = { "Lowest","Low","Medium","High","Serious", NULL };
static UBYTE *audiochannelcycnames[] = { "0 (Left)","1 (Right)","2 (Right)","3 (Left)","Any", NULL };
static UBYTE *mouseportcycnames[] = { "Port 0", "Port 1", NULL };

static UBYTE *mousepointerspeedcycnames[] = { "Quarter","Half","Normal","Double","Ridiculous", NULL };

static APTR setcollisionsaux[] = { &offoncycnames };
static APTR setobjectactionaux[] = { &objactioncycnames };
static APTR bgpointcollisionaux[] = { &dampingcycnames };
static APTR setborderaux[] = { &bordertypecycnames, &bordertypecycnames,
	&bordertypecycnames, &bordertypecycnames };
static APTR setchannelaux[] = { &channelcycnames };
static APTR restrictdirectionaux[] = { &restrictdirectioncycnames };
static APTR initinertialplayeraux[] = { &setdircycnames, &axisfreedomcycnames };
static APTR initlinearplayeraux[] = { &setdircycnames };
static APTR oldinitinertialhomingaux[] = { &homingtargetcycnames };
static APTR initinertialhomeraux[] = { &playercycnames };
static APTR initasteroidsplayeraux[] = { &asteroidsdragcycnames };
static APTR initmouseplayeraux[] = { &mouseportcycnames, &accel5cycnames, &accel5cycnames, &axisfreedomcycnames };
static APTR playsfxaux[] = { &audiochannelcycnames };
static APTR initjitterspeedaux[] = { &axisfreedomcycnames };

static APTR scrollxcenterplayersaux[] = { &trackplayerscycnames };
static APTR initmousepointeraux[] = { &mouseportcycnames, &mousepointerspeedcycnames, &axisfreedomcycnames };
static APTR scrollycenterplayersaux[] = { &trackplayerscycnames };


/************************/

static struct OpPage OLDdir16animpages[] = {
	{ 0,9 }, {9,9}, { 18,9 }, { 27,9 },{ 36,9 },{ 45,6 } };

static struct OpPage inertialjoymovepages[] = {{ 0,7 }, { 7,2 }, {9,5} };

static struct OpPage olddir8animpages[] = {{ 0,8 }, { 8,8 }};
static struct OpPage olddir16animpages[] = {{ 0,8 }, { 8,8 }, { 16,8 }, { 24,8 }};
static struct OpPage olddir32animpages[] = {{ 0,8 }, { 8,8 }, { 16,8 }, { 24,8 },
																				 { 32,8 }, { 40,8 }, { 48,8 }, { 56,8 }};
static struct OpPage olddir64animpages[] =  {{ 0,8 }, { 8,8 }, { 16,8 }, { 24,8 },
																				 { 32,8 }, { 40,8 }, { 48,8 }, { 56,8 },
																				 { 64,8 }, { 72,8 }, { 80,8 }, { 88,8 },
																				 { 96,8 }, { 104,8 }, { 112,8 }, { 120,8 }};

static struct OpPage dir16actionpages[] = {{ 0,8 }, { 8,8 }};
static struct OpPage dir32actionpages[] = {{ 0,8 }, { 8,8 }, { 16,8 }, { 24,8 }};
static struct OpPage dir64actionpages[] =  {{ 0,8 }, { 8,8 }, { 16,8 }, { 24,8 },
																					 { 32,8 }, { 40,8 }, { 48,8 }, { 56,8 }};
static struct OpPage xspeedfromlistpages[] = { {0,8}, {8,8}, {16,8}, {24,8} };
static struct OpPage yspeedfromlistpages[] = { {0,8}, {8,8}, {16,8}, {24,8} };
static struct OpPage joystickanimationpages[] = { {0,10}, {10,10} };
static struct OpPage initdir16imagepages[] = {{ 0,8 }, { 8,11 }};
static struct OpPage dir16imagepages[] = {{ 0,8 }, { 8,11 }};
static struct OpPage initdir16animpages[] = {{ 0,10 }, { 10,8 }};
static struct OpPage initdir32animpages[] = {{ 0,10 }, { 10,8 }, {18,8}, {26,8} };
static struct OpPage initdir64animpages[] = { { 0,10 }, { 10,8 }, {18,8}, {26,8},
	{ 34,8 }, { 42,8 }, {50,8}, {58,8} };
static struct OpPage platformplayerpages[] = {{ 0,11 }, { 11,13 }};
static struct OpPage initinertialplayerpages[] = { {0,10}, {10,7} };
static struct OpPage initmouseplayerpages[] = { {0,8}, {8,5} };
static struct OpPage initactionlistplayerpages[] = { {0,11},{11,8},{19,2} };
static struct OpPage initcheckfirebuttonspages[] = { {0,11},{11,2} };

/*
ACTION CLASSES:

BIT/CLASS
1		Objects
2		Variables
3		Movement
4		Weapons
5		Images
6		ActionLists
9		Basic
16	Unclassified

*/

/* Template defs (case sensitive):																					*/
/* b=byte, w=word, l=longword, O=operator, P=program, A=anim, B=bobimage		*/
/* S=soundeffect, p=playerref, T=Path, s=String, W=weaponref								*/
/* V=varref, L=levelref, E = eotrref */
/* 0=pad byte (always saved as zero) */
/* 1=flag byte bit 0 */
/* 2=flag byte bits 0..1 (bit 1 is first gadget, then bit 0 ) */
/* ...                   */
/* 8=flag byte bits 0..7 (bit 7 is first gadget, then bit 6..0 ) */
/* f=fixed point (27:5) */
/* c=cycleword - cycle labels taken from Aux field */

#define NUMOFOPERATORS 147
UWORD	numofoperators = NUMOFOPERATORS;

struct OperatorDef operators[ NUMOFOPERATORS ] = {
	/* 0 */
	{ "END",							0x8000,OPTYPE_ACTION, 0, "", nulllabels, 1, NULL, NULL },
	{ "XSpeedAnim",			0x8000,OPTYPE_CHANNL, 8+(3*ANIMNAMESIZE), "fAfAA", xspeedanimlabels, 1, NULL, NULL },
	{ "LinearAccel",			0x8001,OPTYPE_CHANNL, 18, "ffff01", linearaccellabels, 1, NULL, NULL },
	{ "RotationalPlayer",			0x8000,OPTYPE_CHANNL, 2+16+2+PROGNAMESIZE+2, "wffffcP02", initasteroidsplayerlabels, 1, NULL, initasteroidsplayeraux },
	/* 4 */
	{ "InertialPlayer", 0x8000,OPTYPE_CHANNL, 2+(4*6)+PROGNAMESIZE+2+8+4, "wffffffP03ffcc", initinertialplayerlabels, 2, initinertialplayerpages, initinertialplayeraux },
	{ "*FireBullet",				0x8009,OPTYPE_ACTION, BOBNAMESIZE+8+PROGNAMESIZE, "BffP", firebulletlabels, 1, NULL, NULL },
	{ "Execute",					0x8120,OPTYPE_ACTION, PROGNAMESIZE, "P", executelabels, 1, NULL, NULL },
	{ "CyclicAnim",				0x8000,OPTYPE_CHANNL, ANIMNAMESIZE+4, "A1bw", cyclicanimlabels, 1, NULL, NULL },
	/* 8 */
	{ "*OLDDir16Anim",				0x8000,OPTYPE_CHANNL, (ANIMNAMESIZE+4)*17,
		"AwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAwwAww", OLDdir16animlabels, 6,
		OLDdir16animpages, NULL },
	{ "*BGCollision",			0x8001,OPTYPE_CHANNL, 4+PROGNAMESIZE, "wPw",obsbgcollisionlabels, 1, NULL, NULL },
	{ "*InertialJoyMove",	0x8001,OPTYPE_CHANNL, 24+6+(PROGNAMESIZE*4), "ffffffw88wPPPP", inertialjoymovelabels, 3, inertialjoymovepages, NULL },
	{ "Kill Object",					0x8101,OPTYPE_ACTION, 0, "",nulllabels, 1, NULL, NULL },
	/* 12 */
	{ "ScrollConstantXSpeed",	0x8100,OPTYPE_ACTION, (4*4)+PROGNAMESIZE+2, "ffffP00", scrollconstantxspeedlabels, 1, NULL, NULL },
	{ "*Directional Homer",0x8001,OPTYPE_CHANNL, 2+2+2+PROGNAMESIZE, "wpwP", directionalhomerlabels, 1, NULL, NULL },
	{ "*PlatformPlayer",		0x8000,OPTYPE_CHANNL, 0, "",nulllabels, 1, NULL, NULL },
	{ "StatPrintVar",		0x8002,OPTYPE_ACTION, 4+4+2, "ffV", statprintvarlabels, 1, NULL, NULL },
	/* 16 */
	{ "PlatformPlayer",		0x8000,OPTYPE_CHANNL, 2+(4*7)+EOTRNAMESIZE+2+PROGNAMESIZE+(4*4)+(8*ANIMNAMESIZE), "wfffffffE03PffffAAAAAAAA", platformplayerlabels, 2, platformplayerpages, NULL },
	{ "SetImage",					0x8110,OPTYPE_ACTION, BOBNAMESIZE, "B", setimagelabels, 1, NULL, NULL },
	{ "PushScrollTrackX",		0x8100,OPTYPE_ACTION, 14, "pfff", pushscrolltrackxlabels, 1, NULL, NULL },
	{ "PushScrollTrackY",		0x8100,OPTYPE_ACTION, 14, "pfff", pushscrolltrackylabels, 1, NULL, NULL },
	/* 20 */
	{ "InitScroll",				0x8100,OPTYPE_ACTION, 4*4, "ll00000000", initscrolllabels, 1, NULL, NULL },
	{ "Get Hurt",					0x8101,OPTYPE_ACTION, 0, "", nulllabels, 1, NULL, NULL },
	{ "FireWeaponBay",		0x8108,OPTYPE_ACTION, 2, "w", fireweaponbaylabels, 1, NULL, NULL },
	{ "PowerUpBay",				0x8008,OPTYPE_ACTION, 2+WEAPONNAMESIZE, "wW", powerupbaylabels, 1, NULL, NULL },
	/* 24 */
	{ "PowerDownBay",			0x8008,OPTYPE_ACTION, 4, "w01", powerdownbaylabels, 1, NULL, NULL },
	{ "SetWeaponBay",			0x8108,OPTYPE_ACTION, 2+WEAPONNAMESIZE, "wW", setweaponbaylabels, 1, NULL, NULL },
	{ "SetChannel",				0x8101,OPTYPE_ACTION, 2+4+2+PROGNAMESIZE, "cOwP", setchannellabels, 1, NULL, setchannelaux },
	{ "HighlightBob",			0x8011,OPTYPE_ACTION, 2, "w", highlightboblabels, 1, NULL, NULL },
	/* 28 */
	{ "*SetColour",				0x8000,OPTYPE_ACTION, 4, "bbbb", setcolourlabels, 1, NULL, NULL },
	{ "KillChannel",			0x8101,OPTYPE_ACTION, 2, "c", killchannellabels, 1, NULL, setchannelaux },
	{ "MousePlayer",			0x8000,OPTYPE_CHANNL, 6+8+8+PROGNAMESIZE+4+8, "cccffffP03cff",initmouseplayerlabels, 2, initmouseplayerpages, initmouseplayeraux },
	{ "ActionListPlayer",			0x8000,OPTYPE_CHANNL, 2+2+(16*PROGNAMESIZE)+2, "w00PPPPPPPPPPPPPPPP08",initactionlistplayerlabels, 3, initactionlistplayerpages, NULL },
	/* 32 */
	{ "CheckFireButtons",			0x8000,OPTYPE_CHANNL, 2+2+(8*PROGNAMESIZE)+2, "w00PPPPPPPP04",initcheckfirebuttonslabels, 2, initcheckfirebuttonspages, NULL },
	{ "YSpeedAnim",			0x8000,OPTYPE_CHANNL, 8+(3*ANIMNAMESIZE), "fAfAA", yspeedanimlabels, 1, NULL, NULL },
	{ "PlaySFX",				0x8000,OPTYPE_ACTION, SFXNAMESIZE+2, "Sc", playsfxlabels, 1, NULL, playsfxaux },
	{ "*OldPlaySFX",					0x8000,OPTYPE_ACTION, SFXNAMESIZE+2, "Sw", oldplaysfxlabels, 1, NULL, NULL },
	/* 36 */
	{ "BeSpritely",				0x8011,OPTYPE_ACTION, 0, "", nulllabels, 1, NULL, NULL },
	{ "BeBobish",					0x8011,OPTYPE_ACTION, 0, "", nulllabels, 1, NULL, NULL },
	{ "InertialHomer",				0x8000,OPTYPE_CHANNL, 6+(4*4), "c0001ffff", initinertialhomerlabels, 1, NULL, initinertialhomeraux },
	{ "*StatPrintScore",				0x8000,OPTYPE_ACTION, 10, "ffw", statprintscorelabels, 1, NULL, NULL },
	/* 40 */
	{ "*InertialHoming",			0x8000,OPTYPE_CHANNL, 20, "cpffff", oldinitinertialhominglabels, 1, NULL, oldinitinertialhomingaux },
	{ "Execute",		0x8000,OPTYPE_CHANNL, 4 + PROGNAMESIZE, "Pww",	timedelayexecutelabels, 1, NULL, NULL },
	{ "FollowPath",				0x8000,OPTYPE_CHANNL, PATHNAMESIZE+2, "T02", followpathlabels, 1, NULL, NULL },
	{ "*OLDDirectionMove",		0x8004,OPTYPE_ACTION, 4, "f", olddirectionmovelabels, 1, NULL, NULL },
	/* 44 */
	{ "*ChannelExecute",	0x8000,OPTYPE_CHANNL, PROGNAMESIZE+2, "Pw", channelexecutelabels, 1, NULL, NULL },
	{ "DirectionMove",		0x8000,OPTYPE_CHANNL, 4+2, "fw", chdirectionmovelabels, 1, NULL, NULL },

	{ "*Dir8Anim",					0x8000,OPTYPE_CHANNL,(ANIMNAMESIZE+2)*8,
		"AwAwAwAwAwAwAwAw", olddir8animlabels, 2, olddir8animpages, NULL },
	{ "*Dir16Anim",				0x8000,OPTYPE_CHANNL,(ANIMNAMESIZE+2)*16,
		"AwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAw", olddir16animlabels, 4, olddir16animpages, NULL },
	{ "*Dir32Anim",				0x8000,OPTYPE_CHANNL,(ANIMNAMESIZE+2)*32,
		"AwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAw",
		 olddir32animlabels, 8, olddir32animpages, NULL },
	{ "*Dir64Anim",				0x8000,OPTYPE_CHANNL,(ANIMNAMESIZE+2)*64,
		"AwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAwAw",
		 olddir64animlabels, 16, olddir64animpages, NULL },
		/* 50 */
	{ "*OldDir8Image",				0x8000,OPTYPE_CHANNL, ANIMNAMESIZE, "A", oldinitdir8imagelabels, 1, NULL, NULL },
	{ "*OldDir16Image",				0x8000,OPTYPE_CHANNL, ANIMNAMESIZE, "A", oldinitdir16imagelabels, 1, NULL, NULL },
	{ "*OldDir32Image",				0x8000,OPTYPE_CHANNL, ANIMNAMESIZE, "A", oldinitdir32imagelabels, 1, NULL, NULL },
	{ "*OldDir64Image",				0x8000,OPTYPE_CHANNL, ANIMNAMESIZE, "A", oldinitdir64imagelabels, 1, NULL, NULL },
		/* 54 */
	{ "*OldActionListPlayer",	0x8000,OPTYPE_CHANNL, (PROGNAMESIZE*4)+2+4, "wPPPPl", oldactionlistplayerlabels, 1, NULL, NULL },
	{ "SpawnPlayerBullet",0x8109,OPTYPE_ACTION, BOBNAMESIZE+8+PROGNAMESIZE, "BffP", spawnplayerbulletlabels, 1, NULL, NULL },
	{ "SpawnBadDude"		 ,0x8101,OPTYPE_ACTION, BOBNAMESIZE+8+PROGNAMESIZE, "BffP", spawnbaddudelabels, 1, NULL, NULL },
	{ "*SpawnPlayer"			 ,0x8001,OPTYPE_ACTION, BOBNAMESIZE+8+PROGNAMESIZE, "BffP", spawnplayerlabels, 1, NULL, NULL },
		/* 58 */
	{ "Dir8Action"			 ,0x8000,OPTYPE_CHANNL, PROGNAMESIZE*8, "PPPPPPPP", dir8actionlabels, 1, NULL, NULL },
	{ "Dir16Action"			 ,0x8000,OPTYPE_CHANNL, PROGNAMESIZE*16, "PPPPPPPPPPPPPPPP", dir16actionlabels, 2, dir16actionpages, NULL },
	{ "Dir32Action"			 ,0x8000,OPTYPE_CHANNL, PROGNAMESIZE*32, "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP", dir32actionlabels, 4, dir32actionpages, NULL },
	{ "Dir64Action"			 ,0x8000,OPTYPE_CHANNL, PROGNAMESIZE*64, "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP", dir64actionlabels, 8, dir64actionpages, NULL },
		/* 62 */
	{ "*StatPrintScore" ,0x8000,OPTYPE_ACTION, 6,"wwp",oldstatprintscorelabels, 1,NULL, NULL },
	{ "*StatPrintGlobalScore"	 ,0x8000,OPTYPE_ACTION, 6,"www",printglobalscorelabels, 1,NULL, NULL },
	{ "GiveScore"			 ,0x8000,OPTYPE_ACTION, 4,"l",givescorelabels, 1,NULL, NULL },
	{ "EndLevel"			 ,0x8100,OPTYPE_ACTION, LEVELNAMESIZE,"L",endlevellabels, 1,NULL, NULL },
		/* 66 */
	{ "Dir8Action"			 ,0x8020,OPTYPE_ACTION, PROGNAMESIZE*8, "PPPPPPPP", dir8actionlabels, 1, NULL, NULL },
	{ "Dir16Action"			 ,0x8020,OPTYPE_ACTION, PROGNAMESIZE*16, "PPPPPPPPPPPPPPPP", dir16actionlabels, 2, dir16actionpages, NULL },
	{ "Dir32Action"			 ,0x8020,OPTYPE_ACTION, PROGNAMESIZE*32, "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP", dir32actionlabels, 4, dir32actionpages, NULL },
	{ "Dir64Action"			 ,0x8020,OPTYPE_ACTION, PROGNAMESIZE*64, "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP", dir64actionlabels, 8, dir64actionpages, NULL },
		/* 70 */
	{ "NullChannel"			,0x8000,OPTYPE_CHANNL, 0,"",nulllabels,1,NULL, NULL },
	{ "*RNDXSpeed"			,0x8000,OPTYPE_ACTION, 2,"w",rndxspeedlabels,1,NULL, NULL },
	{ "*RNDYSpeed"			,0x8000,OPTYPE_ACTION, 2,"w",rndyspeedlabels,1,NULL, NULL },
	{ "*Asteroids Player"	,0x8000,OPTYPE_CHANNL, 2+2+4+4+4+PROGNAMESIZE,"wwlllP",oldasteroidsplayerlabels,1,NULL, NULL },
		/* 74 */
	{ "SetBorders"				,0x8001,OPTYPE_ACTION, (4*4)+(2*4)+2,"fcfcfcfc04",setborderslabels,1,NULL, setborderaux },
	{ "*RNDDirectionMove",0x8004,OPTYPE_ACTION, 4, "f",rnddirectionmove,1,NULL, NULL },
	{ "*Set Variable"		 ,0x8000,OPTYPE_ACTION, (2+4),"wl",setvarlabels,1,NULL, NULL },
	{ "*Add To Variable"  ,0x8000,OPTYPE_ACTION, (2+4),"wl",addtovarlabels,1,NULL, NULL },
		/* 78 */
	{ "*Sub From Variable",0x8000,OPTYPE_ACTION, (2+4),"wl",subfromvarlabels,1,NULL, NULL },
	{ "*Execute If Variable",0x8000,OPTYPE_ACTION, (2+4+2+PROGNAMESIZE),"wlwP",executeifvarcclabels,1,NULL, NULL },
	{ "*OldDir8Image",				0x8010,OPTYPE_ACTION, ANIMNAMESIZE+2, "Aw", olddir8imagelabels, 1, NULL, NULL },
	{ "*OldDir16Image",				0x8010,OPTYPE_ACTION, ANIMNAMESIZE+2, "A00", olddir16imagelabels, 1, NULL, NULL },
		/* 82 */
	{ "Dir32Image",				0x8010,OPTYPE_ACTION, ANIMNAMESIZE+2, "A00", dir32imagelabels, 1, NULL, NULL },
	{ "Dir64Image",				0x8010,OPTYPE_ACTION, ANIMNAMESIZE+2, "A00", dir64imagelabels, 1, NULL, NULL },
	{ "*Rotate ClockWise",	0x8000,OPTYPE_ACTION, 2, "w", rotateclockwiselabels, 1, NULL, NULL },
	{ "*RotateAntiClockWise", 0x8000,OPTYPE_ACTION, 2, "w", rotateanticlockwiselabels, 1, NULL, NULL },
		/* 86 */
	{ "*SetXSpdFromList32" ,0x8006,OPTYPE_ACTION, 4*32, "ffffffffffffffffffffffffffffffff", xspeedfromlistlabels, 4, xspeedfromlistpages, NULL },
	{ "*SetYSpdFromList32" ,0x8006,OPTYPE_ACTION, 4*32, "ffffffffffffffffffffffffffffffff", yspeedfromlistlabels, 4, yspeedfromlistpages, NULL },
	{ "*Set Direction" 	,0x8000,OPTYPE_ACTION, 2, "w", setdirectionlabels, 1, NULL, NULL },
	{ "*Attach"		,0x8000,OPTYPE_CHANNL, 2+8, "pff", attachlabels,1,NULL, NULL},
		/* 90 */
	{ "SetMiscStuff"	,0x8101,OPTYPE_ACTION, (PROGNAMESIZE*3)+2+2+PROGNAMESIZE+2, "PPPwwP00", setmiscstufflabels,1,NULL, NULL },
	{ "*OperationWolfMovement", 0x8000,OPTYPE_CHANNL, 14, "ffwww", operationwolfmovementlabels, 1, NULL, NULL},
	{ "Repel",					0x8004,OPTYPE_ACTION, 4, "ww", repellabels, 1, NULL, NULL },
	{ "SetImmediate",		0x8102,OPTYPE_ACTION, 6, "fV", setilabels, 1, NULL, NULL },
		/* 94 */
	{ "AddImmediate",		0x8102,OPTYPE_ACTION, 6, "fV", addilabels, 1, NULL, NULL },
	{ "SetVariable",		0x8002,OPTYPE_ACTION, 4, "VV", setvlabels, 1, NULL, NULL },
	{ "AddVariable",		0x8002,OPTYPE_ACTION, 4, "VV", addvlabels, 1, NULL, NULL },
	{ "*SetRND",				0x8002,OPTYPE_ACTION, 4, "wV", oldsetrndlabels, 1, NULL, NULL },
		/* 98 */
	{ "*AddRND",				0x8002,OPTYPE_ACTION, 4, "wV", oldaddrndlabels, 1, NULL, NULL },
	{ "ObjectOff",			0x8101,OPTYPE_ACTION, 0, "", nulllabels, 1, NULL, NULL },
	{ "CollisionPartnerExecute",	0x8020,OPTYPE_ACTION, PROGNAMESIZE, "P", collisionpartnerexecutelabels, 1, NULL, NULL },
	{ "CollisionPartnerOff", 0x8000,OPTYPE_ACTION, 0, "", nulllabels,1,NULL, NULL },
		/* 102 */
	{ "KillCollisionPartner", 0x8000,OPTYPE_ACTION, 0, "", nulllabels,1,NULL, NULL },
	{ "JoystickAnimation",	0x8000,OPTYPE_CHANNL, 4+(ANIMNAMESIZE*18), "wwAAAAAAAAAAAAAAAAAA", joystickanimationlabels,2, joystickanimationpages, NULL },
	{ "SetObjectAction", 0x8021,OPTYPE_ACTION, PROGNAMESIZE+2, "Pc",setobjectactionlabels, 1,NULL, setobjectactionaux },
	{ "SetCollisions", 0x8100,OPTYPE_ACTION, 2, "c",setcollisionslabels, 1,NULL, setcollisionsaux },
		/* 106 */
	{ "PlayModule", 0x8000,OPTYPE_ACTION, PROGNAMESIZE, "P",playmodulelabels, 1,NULL, NULL },
	{ "StopModule", 0x8000,OPTYPE_ACTION, 0, "",nulllabels, 1,NULL, NULL },
	{ "ActionOnFire", 0x8000,OPTYPE_CHANNL, 2+2+PROGNAMESIZE,"wwP",actiononfirelabels,1,NULL, NULL },
	{ "CompareVariable",0x8002,OPTYPE_ACTION,4,"VV",comparevariablelabels,1,NULL, NULL },
		/* 110 */
	{ "CompareImmediate",0x8002,OPTYPE_ACTION,6,"fV",compareimediatelabels,1,NULL, NULL },
	{ "IfEqual",0x8022,OPTYPE_ACTION,PROGNAMESIZE,"P",ifequallabels,1,NULL, NULL },
	{ "IfNotEqual",0x8022,OPTYPE_ACTION,PROGNAMESIZE,"P",ifnotequallabels,1,NULL, NULL },
	{ "IfGreaterThan",0x8022,OPTYPE_ACTION,PROGNAMESIZE,"P",ifgreaterthanlabels,1,NULL, NULL },
		/* 114*/
	{ "IfLessThan",0x8022,OPTYPE_ACTION,PROGNAMESIZE,"P",iflowerthanlabels,1,NULL, NULL },
	{ "*BackgroundPointCollision",0x8000,OPTYPE_CHANNL,2+PROGNAMESIZE+2+4+4,"wPwff",backgroundpointcollisionlabels,1,NULL, NULL },
	{ "*SimplePlatformPlayer",0x8000,OPTYPE_CHANNL,2+(5*4)+(3*2),"wfffffww01",simpleplatformplayerlabels,1,NULL, NULL },
	{ "ExecuteOnPlayer",0x8120,OPTYPE_ACTION,2+PROGNAMESIZE,"pP",executeonplayerlabels,1,NULL, NULL },
		/* 118 */
	{ "ExecuteIfOnScreen",0x8020,OPTYPE_ACTION,PROGNAMESIZE,"P",executeifonscreenlabels,1,NULL, NULL },
	{ "AttachToParent",0x8000,OPTYPE_CHANNL,8,"ff",initattachtoparentlabels,1,NULL, NULL },
	{ "DrawBar",0x8000,OPTYPE_CHANNL,12,"wwwfV",drawbarlabels,1,NULL, NULL },
	{ "FloorSentry",0x8000,OPTYPE_CHANNL,4+4+BOBNAMESIZE+2,"ffEb3",floorsentrylabels,1,NULL, NULL },
		/* 122 */
	{ "CeilingSentry",0x8000,OPTYPE_CHANNL,4+4+BOBNAMESIZE+2,"ffEb3",ceilingsentrylabels,1,NULL, NULL },
	{ "LinearPlayer",	0x8000,OPTYPE_CHANNL,2+8+PROGNAMESIZE+2+2 , "wffP03c", initlinearplayerlabels, 1, NULL, initlinearplayeraux },
	{ "BGPointCollision",	0x8000,OPTYPE_CHANNL,8+EOTRNAMESIZE+2+PROGNAMESIZE+2, "ffEb0Pc", bgpointcollisionlabels,1,NULL,bgpointcollisionaux },
	{ "BGCollision",	0x8000,OPTYPE_CHANNL,EOTRNAMESIZE+2+PROGNAMESIZE+2, "Eb0Pc", bgcollisionlabels,1,NULL,bgpointcollisionaux },
		/* 126 */
	{ "TrackPlayer",0x8000,OPTYPE_CHANNL,2+4+4+2,"pff01",trackplayerlabels,1,NULL, NULL },
	{ "DirectionMove", 0x8004,OPTYPE_ACTION, 4+2, "f01", directionmovelabels, 1, NULL, NULL },
	{ "JitterSpeed",0x8000,OPTYPE_CHANNL,4*6+2+2,"ffffffcw",initjitterspeedlabels,1,NULL, initjitterspeedaux },
	{ "SetRND", 0x8102,OPTYPE_ACTION, 4+4+2, "ffV", setrndlabels, 1, NULL, NULL },
		/* 130 */
	{ "AddRND", 0x8102,OPTYPE_ACTION, 4+4+2, "ffV", addrndlabels, 1, NULL, NULL },
	{ "ScrollXCenterPlayers", 0x8000,OPTYPE_ACTION, 4+4+4, "fc000000", scrollxcenterplayerslabels, 1, NULL, scrollxcenterplayersaux },
	{ "Dir8Image",0x8000,OPTYPE_CHANNL,(8*BOBNAMESIZE)+ANIMNAMESIZE+2,"BBBBBBBBA00",initdir8imagelabels,1,NULL, NULL },
	{ "Dir16Image",0x8000,OPTYPE_CHANNL,(16*BOBNAMESIZE)+ANIMNAMESIZE+2,"BBBBBBBBBBBBBBBBA00",initdir16imagelabels,2,initdir16imagepages, NULL },
		/* 134 */
	{ "Dir32Image",0x8000,OPTYPE_CHANNL,ANIMNAMESIZE+2,"A00",initdir32imagelabels,1,NULL, NULL },
	{ "Dir64Image",0x8000,OPTYPE_CHANNL,ANIMNAMESIZE+2,"A00",initdir64imagelabels,1,NULL, NULL },
	{ "Dir8Image",	0x8010,OPTYPE_ACTION, (BOBNAMESIZE*8)+ANIMNAMESIZE+2, "BBBBBBBBA00", dir8imagelabels, 1, NULL, NULL },
	{ "Dir16Image",	0x8010,OPTYPE_ACTION, (BOBNAMESIZE*16)+ANIMNAMESIZE+2, "BBBBBBBBBBBBBBBBA00", dir16imagelabels, 2, dir16imagepages, NULL },
		/* 138 */
	{ "Dir8Anim", 0x8000,OPTYPE_CHANNL, 2+(ANIMNAMESIZE*8), "03AAAAAAAA", initdir8animlabels, 1, NULL, NULL },
	{ "Dir16Anim", 0x8000,OPTYPE_CHANNL, 2+(ANIMNAMESIZE*16), "03AAAAAAAAAAAAAAAA", initdir16animlabels, 2, initdir16animpages, NULL },
	{ "Dir32Anim", 0x8000,OPTYPE_CHANNL, 2+(ANIMNAMESIZE*32), "03AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", initdir32animlabels, 4, initdir32animpages, NULL },
	{ "Dir64Anim", 0x8000,OPTYPE_CHANNL, 2+(ANIMNAMESIZE*64), "03AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", initdir64animlabels, 8, initdir64animpages, NULL },
		/* 142 */
	{ "RestrictDirection", 0x8000,OPTYPE_ACTION, 2+4+4, "cff", restrictdirectionlabels, 1, NULL, restrictdirectionaux },
	{ "RandomEvent", 0x8000,OPTYPE_ACTION, 2+PROGNAMESIZE+2, "wP00", randomeventlabels, 1, NULL, NULL },
	{ "RandomEvent", 0x8000,OPTYPE_CHANNL, 2+PROGNAMESIZE+2, "wP00", initrandomeventlabels, 1, NULL, NULL },
	{ "MousePointer", 0x8000,OPTYPE_CHANNL, (2*4)+(4*6)+2+4, "c00cc000000000000000000000000000000", initmousepointerlabels, 1, NULL, initmousepointeraux },
	{ "ScrollYCenterPlayers", 0x8000,OPTYPE_ACTION, 4+4+4, "fc000000", scrollycenterplayerslabels, 1, NULL, scrollycenterplayersaux },
};

/* ok to add more variable names here - no extra changes needed. */

static UBYTE *varrefnames[] = {
	"XPos", 	"YPos",
	"XSpd", 	"YSpd",
	"XMaxSpd","YMaxSpd",
	"XAccel",	"YAccel",
	"XDeccel","YDeccel",
	"Shields","Damage",
	"Data1",	"Data2",
	"Dir",
	"Global1","Global2",
	"Global3","Global4",
	"Global5",
	"Collision Types",
	"ScrollXPos","ScrollYPos",
	"ScrollSpeed","BaseSpeed",
	"ID",
	NULL
	};


/*******************************************************************/
// A requester for editing operator parameters using the hoopyvision
// layout system. Some operators have so many parameters that we
// split them up over a number of pages.
//

BOOL EditOperator( struct Window *parentwin, UBYTE *opdata )
{
	UWORD op, exitcode, currentpage=0;
	struct Window *win;
	struct HoopyObject *hob = NULL;
	BOOL nextbutton, prevbutton, newpage;
	UWORD	paramnum, specialcode;
	UBYTE title[64];

	op = *( (UWORD *)opdata );

	if( operators[op].od_Type == OPTYPE_ACTION )
		sprintf( title, "%s Params (Action)", operators[op].od_Name );
	else
		sprintf( title, "%s Params (ChRoutine)", operators[op].od_Name );

	if( strlen( operators[op].od_Template ) == 0 )
		return( TRUE );

	if( win = OpenWindowTags( NULL,
		WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
		WA_Top, parentwin->TopEdge + parentwin->BorderTop,
		WA_Width, 300,
		WA_Height, 200,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, parentwin->WScreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				title,	//"Edit Operator",
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			STRINGIDCMP|TEXTIDCMP|BUTTONIDCMP,
		TAG_DONE,				NULL) )
	{
		do
		{
			nextbutton=FALSE;
			prevbutton=FALSE;
			if( currentpage+1 < operators[op].od_PageCount ) nextbutton = TRUE;
			if( currentpage > 0 ) prevbutton = TRUE;

			hob = CreateOpEditLayout( opdata, currentpage, nextbutton, prevbutton );
			if( hob )
			{
				newpage = FALSE;
				while( !newpage && ( (exitcode = HO_DoReq( hob, win )) != 0xFFFF ) )
				{

					/* nextpage gad? */
					if( exitcode == 0xFFFE )
					{

						ReadOpParamsFromLayout( hob, opdata, currentpage );
						if( currentpage+1 < operators[op].od_PageCount )
						{
							currentpage++;
							newpage = TRUE;
						}
						exitcode = 0;
					}

					/* prevpage gad? */
					if( exitcode == 0xFFFD )
					{
						ReadOpParamsFromLayout( hob, opdata, currentpage );
						if( currentpage > 0 )
						{
							currentpage--;
							newpage = TRUE;
						}
						exitcode = 0;
					}

					if( exitcode >= 0x8000 && exitcode != 0xFFFF )
					{
						ReadOpParamsFromLayout( hob, opdata, currentpage );

						if( operators[ op ].od_PageCount == 1 )
							paramnum = exitcode & 0x00FF;
						else
						{
							paramnum = ( exitcode & 0x00FF ) +
								operators[ op ].od_PageLayout[ currentpage ].op_Start;
						}
						specialcode = ( exitcode & 0x7F00 ) >> 8;

						HandleSpecialGadget( win, opdata, paramnum, specialcode );
						newpage = TRUE;
					}
				}
				if( !newpage )
					ReadOpParamsFromLayout( hob, opdata, currentpage );
				FreeOpEditLayout( hob );
			}
			else
				exitcode = 0xFFFF;	// bail out...
		} while( exitcode != 0xFFFF );
		CloseWindow( win );
	}

	return( TRUE );
}


static VOID HandleSpecialGadget( struct Window *win, UBYTE *opbase,
	UWORD paramnum, UWORD specialcode )
{
	UWORD op, childop;
	UBYTE *opdata, *childopdata;
	APTR blockwinhandle;
	struct BobInfoNode *bin;
	struct AnimInfoNode *ain;
	struct Program *prog;
	WORD var;

	op = *((UWORD *)opbase);
	opdata = FindOpParam( opbase, paramnum );

	switch( operators[op].od_Template[paramnum] )
	{
		case 'P':				/* pickaprogram button */
			if( blockwinhandle = BlockWin( win ) )
			{
				/* pick a program and copy its name into the opdata */
				if( prog =
					RequestProgram( win, NULL, "Pick Actionlist...", "Actionlists" ) )
				{
					Mystrncpy( (STRPTR)opdata, prog->pg_Name, PROGNAMESIZE-1 );
				}
				UnblockWin( blockwinhandle );
			}
			break;

		case 'B':				/* pickabob button */
			if( blockwinhandle = BlockWin( win ) )
			{
				/* pick a bob and copy its name into the opdata */
				if( bin =
					RequestBob( win, "Pick Bob...", NULL ) )
				{
					Mystrncpy( (STRPTR)opdata, bin->bin_Name, BOBNAMESIZE-1 );
				}
				UnblockWin( blockwinhandle );
			}
			break;
		case 'A':				/* pickanim button */
			if( blockwinhandle = BlockWin( win ) )
			{
				/* pick a bob and copy its name into the opdata */
				if( ain =
					RequestAnim( win, "Pick Anim...", NULL ) )
				{
					Mystrncpy( (STRPTR)opdata, ain->ain_Name, ANIMNAMESIZE-1 );
				}
				UnblockWin( blockwinhandle );
			}
			break;

		case 'V':				/* variable */
			if( specialcode == 1 )
			{
				if( blockwinhandle = BlockWin( win ) )
				{
					var = RequestVar( win );
					if( var != -1 )
					{
						*((WORD *)opdata) = var;
					}
					UnblockWin( blockwinhandle );
				}
			}
			break;

		case 'O':				/* Operator */

			if( specialcode == 1 )
			{
				if( blockwinhandle = BlockWin( win ) )
				{
					childop = OpReq( win, OPTYPE_CHANNL );
					if( childop != 0xFFFF )
					{
						if( childopdata = AllocVec( CalcOpSize( childop ),
							MEMF_ANY|MEMF_CLEAR ) )
						{
							FreeOperatorData( *((UBYTE **)opdata) );
							*((UBYTE **)opdata) = childopdata;
							*((UWORD *)childopdata) = childop;
							EditOperator( win, *((UBYTE **)opdata) );
						}
					}
					UnblockWin( blockwinhandle );
				}
			}

			/* edit gadget */
			if( specialcode == 0 )
			{
				if( *((UBYTE **)opdata) )
				{
					if( blockwinhandle = BlockWin( win ) )
					{
						EditOperator( win, *((UBYTE **)opdata) );
						UnblockWin( blockwinhandle );
					}
				}
			}
			break;
	}
}



/*******************************************************************/
// Creates a hoopyobject layout array for entering/editing parameters.
// Some operators need so many parameters that we want to have multiple
// pages - hence the pagenum param for just creating a particular page.
// Pair with FreeOpEditLayout().
//

struct HoopyObject *CreateOpEditLayout( UBYTE *opdata, UWORD pagenum,
	BOOL nextbutton, BOOL prevbutton )
{
	APTR foo;
	APTR *aux;

	struct HoopyObject *hob;
	UWORD	op, i, paramcount;
	int bitnum;
	WORD	pagestart, pagecount;
	UBYTE *template, *flagnames;
	BOOL stop = FALSE;
	static struct HoopyObject headervgroup =
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,
			0,0,0,0, NULL, 0,0,0,0,0 };
	static struct HoopyObject hgroup =
		{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,
			0,0,0,0, NULL, 0,0,0,0,0 };
	static struct HoopyObject empty =
		{ 0, 100, 0,
			0,0,0,0, NULL, 0,0,0,0,0 };

	static UBYTE workbuffer[ 512 ];
	UBYTE *wkptr;

//	printf("CreateOpEditLayout(): page %d\n",pagenum );
	op = *((UWORD *)opdata);

	if( operators[ op ].od_PageLayout )
	{
		if( pagenum >= operators[ op ].od_PageCount )
			return( NULL );
		pagestart = operators[ op ].od_PageLayout[ pagenum ].op_Start;
		pagecount = operators[ op ].od_PageLayout[ pagenum ].op_Count;
	}
	else
	{
		pagestart = 0;
		pagecount = -1;				/* all params */
	}

	/* find correct position for aux pointer */
	template = operators[op].od_Template;
	aux = operators[op].od_Aux;
	for( i = 0; i < pagestart; i++ )
	{
		/* cyclewords ('c') only type that use aux at moment */
		if( *template++ == 'c' )
			aux++;
	}

	template = operators[op].od_Template + pagestart;

	/* misc work memory for items to (ab)use */
	wkptr = workbuffer;

	/* grab enough memory for a header VGROUP, however many objects the params */
	/* need, 4 for an next/previous group, 2 spaces , 4 for ok/cancel and 1 END */
	if( hob = AllocVec( sizeof( struct HoopyObject ) *
		( 1 + 4 + PageObjectCount( opdata, pagenum ) + 1 + 4 + 1 ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( opdata = FindOpParam( opdata, pagestart ) )
		{
//			printf( "  pagestart = %d, opdata = %ld\n",pagestart,(LONG)opdata );

			i=0;
			hob[i] = headervgroup;
			i++;

			hob[i] = hgroup;
			hob[i].ho_HOFlags = HOFLG_FREEWIDTH;
			hob[i].ho_Attr0 = 3;
			i++;

			hob[i].ho_Weight = 100;
			hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH|
				(prevbutton ? 0:HOFLG_DISABLED);
			hob[i].ho_Text = "< Prev";
			hob[i].ho_Flags = PLACETEXT_IN;
			hob[i].ho_Type = HOTYPE_BUTTON;
			hob[i].ho_GadgetID = 0xFFFD;
			i++;

			hob[i].ho_Weight = 100;
			hob[i].ho_HOFlags = HOFLG_FREEWIDTH|HOFLG_NOBORDER;
			hob[i].ho_Text = NULL;
			hob[i].ho_Flags = 0;
			hob[i].ho_Type = HOTYPE_SPACE;
			i++;

			hob[i].ho_Weight = 100;
			hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH|
				(nextbutton ? 0:HOFLG_DISABLED);
			hob[i].ho_Text = "Next >";
			hob[i].ho_Flags = PLACETEXT_IN;
			hob[i].ho_Type = HOTYPE_BUTTON;
			hob[i].ho_GadgetID = 0xFFFE;
			i++;

			paramcount = 0;
			while( *template && !stop )
			{
				hob[i].ho_Weight = 100;
				hob[i].ho_HOFlags = 0;
				hob[i].ho_Text =
					operators[ op ].od_TemplateLabels[ pagestart + paramcount ];
				hob[i].ho_Flags = PLACETEXT_RIGHT;
				hob[i].ho_GadgetID = 0;

				bitnum = 7;		/* just for flagbyte operators */
				switch( *template++ )
				{
					case '1':
						bitnum--;
					case '2':
						bitnum--;
					case '3':
						bitnum--;
					case '4':
						bitnum--;
					case '5':
						bitnum--;
					case '6':
						bitnum--;
					case '7':
						bitnum--;
					case '8':
						hob[i] = empty;
						hob[i].ho_Type = HOTYPE_VGROUP;
						hob[i].ho_Attr0 = bitnum+1;
						i++;

						flagnames =
							operators[ op ].od_TemplateLabels[ pagestart + paramcount ];

						while( bitnum >= 0 )
						{
//							printf("i=%d, flag bit: %d\n",i,bitnum);
							hob[i].ho_Weight = 100;
							hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
							hob[i].ho_Text = flagnames;
							hob[i].ho_Flags = PLACETEXT_RIGHT;
							hob[i].ho_Type = HOTYPE_CHECKBOX;
							if( (*((UBYTE *)opdata) >> bitnum ) & 1 )
								hob[i].ho_Value = (LONG)TRUE;
							else
								hob[i].ho_Value = (LONG)FALSE;
							hob[i].ho_GadgetID = 0;

							flagnames += strlen( flagnames ) + 1;
							i++;
							bitnum--;
						}
						i--;
						opdata += 1;
						break;
					case '0':
						/* KLUDGE */
						hob[i].ho_Type = HOTYPE_SPACE;
						hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
						opdata += 1;
						break;
					case 'b':
						hob[i].ho_Type = HOTYPE_INTEGER;
						hob[i].ho_Attr0 = 4;
						hob[i].ho_Value = (LONG)*((BYTE *)opdata);
						opdata += 1;
						break;
					case 'w':
						hob[i].ho_Type = HOTYPE_INTEGER;
						hob[i].ho_Attr0 = 6;
						hob[i].ho_Value = (LONG)*((WORD *)opdata);
						opdata += 2;
						break;
					case 'l':
						hob[i].ho_Type = HOTYPE_INTEGER;
						hob[i].ho_Attr0 = 11;
						hob[i].ho_Value = *((LONG *)opdata);
						opdata += 4;
						break;
					case 'f':
						hob[i].ho_Type = HOTYPE_FIXEDPOINT;
						hob[i].ho_Attr0 = 12;
						hob[i].ho_Value = *((LONG *)opdata);
						opdata += 4;
						break;
					case 'c':			/* CycleWord */
						hob[i].ho_Type = HOTYPE_CYCLE;
						hob[i].ho_HOFlags &= ~HOFLG_FREEWIDTH;
						hob[i].ho_Attr0 = (LONG)*aux++;
						hob[i].ho_Value = (LONG)*((WORD *)opdata);
						opdata += 2;
						break;
					case 'V':
						hob[i] = hgroup;
						hob[i].ho_Attr0 = 2;
						i++;

						/* a stringgadget */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
						hob[i].ho_Text =
							operators[ op ].od_TemplateLabels[ pagestart + paramcount ];
						hob[i].ho_Flags = PLACETEXT_RIGHT;
						hob[i].ho_Type = HOTYPE_STRING;
						hob[i].ho_Attr0 = (LONG)wkptr;
						Mystrncpy( wkptr, varnodes[ *((WORD *)opdata) ].ln_Name, VARNAMESIZE-1 );
						wkptr += VARNAMESIZE;
						hob[i].ho_Attr1 = VARNAMESIZE-1;
						hob[i].ho_GadgetID = 0x8000 | paramcount;
						i++;

						/* a var-selection button */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER;
						hob[i].ho_Text = "?";
						hob[i].ho_Flags = PLACETEXT_IN;
						hob[i].ho_Type = HOTYPE_BUTTON;
						hob[i].ho_GadgetID = 0x8100 | paramcount;
						opdata += 2;
						break;
					case 'p':
						hob[i].ho_Type = HOTYPE_HSLIDER;
						hob[i].ho_Attr0 = 0;
						hob[i].ho_Attr1 = 3;
						hob[i].ho_Value = (LONG)*((WORD *)opdata);
						opdata += 2;
						break;
					case 'B':
						hob[i] = hgroup;
						hob[i].ho_Attr0 = 2;
						i++;

						/* a stringgadget */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
						hob[i].ho_Text =
							operators[ op ].od_TemplateLabels[ pagestart + paramcount ];
						hob[i].ho_Flags = PLACETEXT_RIGHT;
						hob[i].ho_Type = HOTYPE_PACKEDSTRING;
						hob[i].ho_Attr0 = (LONG)opdata;
						hob[i].ho_Attr1 = BOBNAMESIZE-1;
						i++;

						/* a bob-selection button */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER;
						hob[i].ho_Text = "?";
						hob[i].ho_Flags = PLACETEXT_IN;
						hob[i].ho_Type = HOTYPE_BUTTON;
						hob[i].ho_GadgetID = 0x8000 | paramcount;

						opdata += BOBNAMESIZE;
						break;
					case 'W':
					case 'T':
					case 'L':
						hob[i].ho_Type = HOTYPE_PACKEDSTRING;
						hob[i].ho_Attr0 = (ULONG)opdata;
						hob[i].ho_Attr1 = GENERICNAMESIZE-1;
						opdata += GENERICNAMESIZE;
						break;
					case 'A':
						hob[i] = hgroup;
						hob[i].ho_Attr0 = 2;
						i++;

						/* a stringgadget */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
						hob[i].ho_Text =
							operators[ op ].od_TemplateLabels[ pagestart + paramcount ];
						hob[i].ho_Flags = PLACETEXT_RIGHT;
						hob[i].ho_Type = HOTYPE_PACKEDSTRING;
						hob[i].ho_Attr0 = (LONG)opdata;
						hob[i].ho_Attr1 = ANIMNAMESIZE-1;
						i++;

						/* a selection button */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER;
						hob[i].ho_Text = "A";
						hob[i].ho_Flags = PLACETEXT_IN;
						hob[i].ho_Type = HOTYPE_BUTTON;
						hob[i].ho_GadgetID = 0x8000 | paramcount;

						opdata += ANIMNAMESIZE;
						break;
					case 'P':			/* Actionlist */
						hob[i] = hgroup;
						hob[i].ho_Attr0 = 2;
						i++;

						/* a stringgadget */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
						hob[i].ho_Text =
							operators[ op ].od_TemplateLabels[ pagestart + paramcount ];
						hob[i].ho_Flags = PLACETEXT_RIGHT;
						hob[i].ho_Type = HOTYPE_PACKEDSTRING;
						hob[i].ho_Attr0 = (LONG)opdata;
						hob[i].ho_Attr1 = PROGNAMESIZE-1;
						i++;

						/* a program-selection button */
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER;
						hob[i].ho_Text = "?";
						hob[i].ho_Flags = PLACETEXT_IN;
						hob[i].ho_Type = HOTYPE_BUTTON;
						hob[i].ho_GadgetID = 0x8000 | paramcount;

						opdata += PROGNAMESIZE;
						break;

					case 'S':			/* sound effect */
						hob[i].ho_Type = HOTYPE_PACKEDSTRING;
						hob[i].ho_Attr0 = (ULONG)opdata;
						hob[i].ho_Attr1 = SFXNAMESIZE-1;
						opdata += SFXNAMESIZE;
						break;
					case 'E':			/* EdgeOfTheRoad */
						hob[i].ho_Type = HOTYPE_PACKEDSTRING;
						hob[i].ho_Attr0 = (ULONG)opdata;
						hob[i].ho_Attr1 = EOTRNAMESIZE-1;
						opdata += EOTRNAMESIZE;
						break;
					case 's':			/* string (nullterminated) */
						hob[i].ho_HOFlags = HOFLG_FREEWIDTH;
						if( hob[i].ho_Attr0 = (LONG)AllocVec( 256,MEMF_ANY ) )
						{
							Mystrncpy( (STRPTR)hob[i].ho_Attr0, *((STRPTR *)opdata), 256-1 );
							hob[i].ho_Attr1 = 256-1;
						}
						else
							hob[i].ho_Attr1 = 0;
						hob[i].ho_Type = HOTYPE_STRING;
						opdata += 4;
						break;
					case 'O':			/* operator (ptr to other opdata) */
						hob[i] = hgroup;
						hob[i].ho_Attr0 = 3;
						i++;

						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
						hob[i].ho_Text =
							operators[ op ].od_TemplateLabels[ pagestart + paramcount ];
						hob[i].ho_Flags = PLACETEXT_RIGHT;
						hob[i].ho_Type = HOTYPE_TEXT;
						foo = *((APTR *)opdata);
//						hob[i].ho_Attr0 = (ULONG)operators[ *((UWORD *)(*((UBYTE **)opdata ))) ].od_Name;
						if( foo )
							hob[i].ho_Attr0 = (ULONG)operators[ *((UWORD *)foo ) ].od_Name;
						else
							hob[i].ho_Attr0 = (ULONG)"None";
						i++;
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER;
						hob[i].ho_Text = "Edit";
						hob[i].ho_Flags = PLACETEXT_IN;
						hob[i].ho_Type = HOTYPE_BUTTON;
						hob[i].ho_GadgetID = 0x8000 | paramcount;
						i++;
						hob[i].ho_Weight = 100;
						hob[i].ho_HOFlags = HOFLG_NOBORDER;
						hob[i].ho_Text = "Replace";
						hob[i].ho_Flags = PLACETEXT_IN;
						hob[i].ho_Type = HOTYPE_BUTTON;
						hob[i].ho_GadgetID = 0x8100 | paramcount;
						opdata += 4;
						break;
//					default:
//						printf("CreateEditOpLayout(): unknown template field...\n");
//						break;
				}
				i++;
				paramcount++;
				if( pagecount != -1 )
					if( paramcount >= pagecount ) stop = TRUE;
			}

			hob[i].ho_Weight = 100;
			hob[i].ho_HOFlags = HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT|HOFLG_NOBORDER;
			hob[i].ho_Text = NULL;
			hob[i].ho_Flags = 0;
			hob[i].ho_Type = HOTYPE_SPACE;
			i++;

			hob[i] = hgroup;
			hob[i].ho_HOFlags = HOFLG_FREEWIDTH;
			hob[i].ho_Attr0 = 3;
			i++;

			hob[i].ho_Weight = 100;
			hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
			hob[i].ho_Text = "OK";
			hob[i].ho_Flags = PLACETEXT_IN;
			hob[i].ho_Type = HOTYPE_BUTTON;
			hob[i].ho_GadgetID = 0xFFFF;
			i++;

			hob[i].ho_Weight = 200;
			hob[i].ho_HOFlags = HOFLG_FREEWIDTH|HOFLG_NOBORDER;
			hob[i].ho_Text = NULL;
			hob[i].ho_Flags = 0;
			hob[i].ho_Type = HOTYPE_SPACE;
			i++;

//			hob[i].ho_Weight = 100;
//			hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
//			hob[i].ho_Text = "Cancel";
//			hob[i].ho_Flags = PLACETEXT_IN;
//			hob[i].ho_Type = HOTYPE_BUTTON;
//			hob[i].ho_GadgetID = 0xFFFF;
//			i++;

			hob[i].ho_Weight = 100;
			hob[i].ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
			hob[i].ho_Text = NULL;
			hob[i].ho_Flags = 0;
			hob[i].ho_Type = HOTYPE_SPACE;
			hob[i].ho_GadgetID = 0xFFFF;
			i++;

			hob[0].ho_Attr0 = paramcount + 1 + 1 + 1;

			hob[i].ho_Type = HOTYPE_END;
		}
		else
		{
			FreeVec(hob);
			hob = NULL;
		}
	}

	return( hob );
}




/*******************************************************************/

VOID FreeOpEditLayout( struct HoopyObject *hob )
{
	FreeVec( hob );
}

/*******************************************************************/
// count how many hoopyobjects needed for the params on a given page

static UWORD PageObjectCount( UBYTE *opdata, UWORD pagenum )
{
	UWORD op, hobcount = 0;
	WORD paramcount = -1;
	UBYTE *template;

//	printf("PageObjectCount()\n");

	op = *((UWORD *)opdata);
	template = operators[op].od_Template;

	if( operators[op].od_PageLayout )
	{
//		printf("paged...");
//		printf("%d:%s",op,operators[op].od_Name );
		opdata =
			FindOpParam( opdata, operators[op].od_PageLayout[ pagenum ].op_Start );
		template += operators[op].od_PageLayout[ pagenum ].op_Start;
		paramcount = operators[op].od_PageLayout[ pagenum ].op_Count;
	}
	else
		opdata += 2;

	while( *template && paramcount )
	{
		switch( *template++ )
		{
			case '1':
				hobcount += 2;
				opdata += 1;
				break;
			case '2':
				hobcount += 3;
				opdata += 1;
				break;
			case '3':
				hobcount += 4;
				opdata += 1;
				break;
			case '4':
				hobcount += 5;
				opdata += 1;
				break;
			case '5':
				hobcount += 6;
				opdata += 1;
				break;
			case '6':
				hobcount += 7;
				opdata += 1;
				break;
			case '7':
				hobcount += 8;
				opdata += 1;
				break;
			case '8':
				hobcount += 9;
				opdata += 1;
				break;
			case '0':
				hobcount++;		/* kludge space gadget */
				opdata += 1;
				break;
			case 'b':
				hobcount++;
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'p':
				hobcount++;
				opdata += 2;
				break;
			case 'V':			/* variable */
				hobcount += 3;
				opdata += 2;
				break;
			case 'l':
			case 'f':
				hobcount++;
				opdata += 4;
				break;
			case 'A':
			case 'B':
				hobcount += 3;
				opdata += GENERICNAMESIZE;
				break;
			case 'T':
			case 'W':
			case 'L':
				hobcount++;
				opdata += GENERICNAMESIZE;
				break;
			case 'P':
				hobcount += 3;
				opdata += PROGNAMESIZE;
				break;
			case 'E':			/* EdgeOfTheRoad */
			case 'S':			/* sound effect */
				hobcount++;
				opdata += SFXNAMESIZE;
				break;
			case 's':			/* string (nullterminated) */
				hobcount++;
				opdata += 4;
				break;
			case 'O':			/* operator (ptr to other opdata) */
				hobcount +=4 ;
				opdata += 4;
				break;
//			default:
//				printf("PageObjectCount: unknown template field...\n");
//				break;
		}
		if( paramcount != -1 ) paramcount--;
	}


	return( hobcount );
}



/*******************************************************************/
// Returns a pointer to the nth parameter of an operator
//

UBYTE *FindOpParam( UBYTE *opdata, UWORD num )
{
	UBYTE	*template;
	UWORD op;

	op = *((UWORD *)opdata);
//	printf("FindOpParam(), %d:%s\n",op,operators[op].od_Name);
	opdata += 2;
	template = operators[op].od_Template;

	while( num-- )
	{
		switch( *template++ )
		{
			case '\0':
				return( NULL );
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'b':
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'V':
			case 'p':
				opdata += 2;
				break;
			case 'f':
			case 'l':
			case 's':						/* string (nullterminated) */
			case 'O':						/* operator */
				opdata += 4;
				break;
			case 'B':						/* bob image */
			case 'W':						/* weaponref */
			case 'L':						/* levelref */
			case 'T':						/* path */
			case 'A':						/* bob anim */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
			case 'P':						/* program */
				opdata += GENERICNAMESIZE;
				break;
			default:
				D(bug("FindOpParam(): unknown template field...\n") );
				num = 0;
				opdata = NULL;
				break;
		}
	}

	return( opdata );
}



/**********************************************************************/
// ReadOpParamsFromLayout()
//
// Read a set of parameters from a hoopyobject layout array which was created
// with CreateOpEditLayout(). Some parameters, such as strings, are
// automatically updated during HO_DoReq().

VOID ReadOpParamsFromLayout( struct HoopyObject *hob, UBYTE *opdata, UWORD pagenum )
{
	UWORD op;
	WORD pagestart,pagecount;
	UBYTE *template;
	int bitnum;
	int i;

//	printf("ReadOpParamsFromLayout()\n");


	op = *((UWORD *)opdata);

	if( operators[ op ].od_PageCount > 1 )
	{
		if( pagenum >= operators[ op ].od_PageCount )
			return;
		pagestart = operators[ op ].od_PageLayout[ pagenum ].op_Start;
		pagecount = operators[ op ].od_PageLayout[ pagenum ].op_Count;
	}
	else
	{
		pagestart = 0;
		pagecount = -1;				/* all params */
	}

//	printf("  pagestart: %d, pagecount: %d\n", pagestart, pagecount );

	template = operators[ op ].od_Template + pagestart;

	if( opdata = FindOpParam( opdata, pagestart ) )
	{
		hob += 5;			// skip vgroup and next/prev group.
		while( pagecount && *template )
		{
			bitnum = 7;
			switch( *template++ )
			{
				case '1':
					bitnum--;
				case '2':
					bitnum--;
				case '3':
					bitnum--;
				case '4':
					bitnum--;
				case '5':
					bitnum--;
				case '6':
					bitnum--;
				case '7':
					bitnum--;
				case '8':
					hob++;			/* skip vgroup */

					while( bitnum >= 0 )
					{
						if( hob->ho_Value )
							*((UBYTE *)opdata) |= (1<<bitnum);
						else
							*((UBYTE *)opdata) &= ~(1<<bitnum);
						hob++;			/* next gadget */
						bitnum--;
					}
					opdata += 1;
					break;
				case '0':
					hob++;									/* skip kludge gadget */
					*((UBYTE *)opdata) = 0;	/* just to be on safe side */
					opdata += 1;
					break;
				case 'b':
					if((LONG)hob->ho_Value < 0)								//signed?
						*((BYTE *)opdata) = (BYTE)hob->ho_Value;
					else
						*((UBYTE *)opdata) = (UBYTE)hob->ho_Value;
					hob++;
					opdata += 1;
					break;
				case 'w':
				case 'c':			/* CycleWord */
				case 'p':
					if((LONG)hob->ho_Value < 0)								//signed?
						*((WORD *)opdata) = (WORD)hob->ho_Value;
					else
						*((UWORD *)opdata) = (UWORD)hob->ho_Value;
					hob++;
					opdata += 2;
					break;
				case 'l':
				case 'f':
					*((ULONG *)opdata) = (ULONG)hob->ho_Value;
					hob++;
					opdata += 4;
					break;
				case 'V':		/* variable */
					hob++;		/* skip hgroup */
					i = FindVariable( (STRPTR)hob->ho_Attr0 );
					if( i != -1 )
						*((UWORD *)opdata) = (UWORD)i;
					hob++;		/* skip string */
					hob++;		/* skip button */
					opdata += 2;
					break;
				case 'B':
				case 'A':
					/* string already updated - make sure there are no crap chars */
					MakePackable( (STRPTR)opdata );
					hob++;	/* skip hgroup */
					hob++;	/* skip string */
					hob++;	/* skip button */
					opdata += GENERICNAMESIZE;
					break;
				case 'W':
				case 'T':
				case 'L':
				case 'E':						/* EdgeOfTheRoad */
					MakePackable( (STRPTR)opdata );
					hob++;
					opdata += GENERICNAMESIZE;
					break;
				case 'P':
					MakePackable( (STRPTR)opdata );
					hob++;	/* skip hgroup */
					hob++;	/* skip string */
					hob++;	/* skip button */
					opdata += PROGNAMESIZE;
					break;
				case 'S':			/* sound effect */
					MakePackable( (STRPTR)opdata );
					hob++;
					opdata += SFXNAMESIZE;
					break;
				case 's':			/* string (nullterminated) */
					FreeVec( *((STRPTR *)opdata) );
					*((STRPTR *)opdata) = MyStrDup( (STRPTR)hob->ho_Attr0 );
					FreeVec( (APTR)hob->ho_Attr0 );
					hob++;
					opdata += 4;
					break;
				case 'O':			/* operator (ptr to other opdata) */
					/* automagically updated */
					hob++;			// skip hgroup
					hob++;
					hob++;
					hob++;
					opdata += 4;
					break;
//				default:
//					printf("unknown template field...\n");
//					break;
			}

			if( pagecount != -1 ) --pagecount;
		}
	}
}


/**********************************************************/
//
// Free memory used for an operator and its parameters.
// (recursive - frees and child operators/params as well )
//

VOID FreeOperatorData( UBYTE *opdatbase )
{
	UBYTE *template, *opdata;

	if( opdata=opdatbase )
	{
		template = operators[ *((UWORD *)opdata) ].od_Template;
//		printf("free:	%s\n", operators[ *((UWORD *)opdata) ].od_Name );
		opdata += 2;

		while( *template )
		{
			switch( *template )
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case 'b':			/* byte */
					opdata += 1;
					break;
				case 'w':			/* word */
				case 'c':			/* CycleWord */
				case 'V':			/* var */
				case 'p':			/* player ref */
					opdata += 2;
					break;
				case 'l':			/* long word */
				case 'f':			/* fixedpoint */
					opdata += 4;
					break;
				case 's':			/* string (nullterminated) */
					FreeVec( *((STRPTR *)opdata) );
					opdata += 4;
					break;
				case 'A':			/* anim */
				case 'B':			/* bob image */
				case 'T':			/* path */
				case 'W':			/* weapons */
				case 'P':			/* program */
				case 'S':			/* sound effect */
				case 'E':			/* EdgeOfTheRoad */
				case 'L':			/* levelref */
					opdata += GENERICNAMESIZE;
					break;
				case 'O':			/* operator (recursive) */
					FreeOperatorData( *((UBYTE **)opdata) );
					opdata += 4;
					break;

//				default:
//					printf("FreeOperatorData(): unknown template field: '%c'\n", *template );
//					break;
			}
			template++;
		}
		FreeVec( opdatbase );
	}
}


/**********************************************************/
//

UBYTE *CopyOperator( UBYTE *src )
{
	UBYTE *template, *dest, *destbase;
	ULONG size;
	UWORD op;

	dest = NULL;

	if( src )
	{
		op = *( (UWORD *)src );

		size = CalcOpSize( op );

		if( destbase = AllocVec( size, MEMF_ANY ) )
		{
			dest = destbase;
			template = operators[ op ].od_Template;

			/* write op token */
			*((UWORD *)dest) = op;
			dest += 2;
			src += 2;

			while( *template )
			{
				switch( *template++ )
				{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case 'b':			/* byte */
						*dest++ = *src++;
						break;
					case 'w':			/* word */
					case 'c':			/* CycleWord */
					case 'V':			/* var */
					case 'p':			/* player ref */
						*dest++ = *src++;
						*dest++ = *src++;
						break;
					case 'l':			/* long word */
					case 'f':			/* fixedpoint */
						*dest++ = *src++;
						*dest++ = *src++;
						*dest++ = *src++;
						*dest++ = *src++;
						break;
					case 's':			/* string (nullterminated) */
						*((STRPTR *)dest) = MyStrDup( *((STRPTR *)src) );
						src += 4;
						dest += 4;
						break;
					case 'B':			/* bob image */
					case 'T':			/* path */
					case 'W':			/* weapon */
					case 'L':			/* levelref */
					case 'A':			/* bob anim */
					case 'P':			/* program */
					case 'S':			/* sound effect */
					case 'E':			/* EdgeOfTheRoad */
						CopyMem( src, dest, GENERICNAMESIZE );
						src += GENERICNAMESIZE;
						dest += GENERICNAMESIZE;
						break;
					case 'O':			/* operator (recursive) */
						*((UBYTE **)dest) = CopyOperator( *((UBYTE **)src) );
						src += 4;
						dest += 4;
						break;

					default:
						D(bug("CopyOperator(): unknown template field...\n") );
						break;
				}
			}
		}
	}

	return( destbase );
}

/**********************************************************/

ULONG CalcOpSize( UWORD op )
{
	UBYTE *template;
	ULONG size = 2;		/* reserve a word for the operator itself */

	template = operators[op].od_Template;

	while( *template )
	{
		switch( *template++ )
		{
			case '1':
				size ++;
			case '2':
				size ++;
			case '3':
				size ++;
			case '4':
				size ++;
			case '5':
				size ++;
			case '6':
				size ++;
			case '7':
				size ++;
			case '8':
				size ++;
				break;
			case '0':
			case 'b':			/* byte */
				size += 1;
				break;
			case 'w':			/* word */
			case 'c':			/* CycleWord */
			case 'V':			/* var */
			case 'p':			/* player ref */
				size += 2;
				break;
			case 'f':			/* fixedpoint */
			case 'l':			/* long word */
				size += 4;
				break;
			case 's':			/* string (nullterminated) */
				size += 4;
				break;
			case 'B':			/* bob image */
			case 'T':			/* path */
			case 'W':			/* weapon */
			case 'L':			/* levelref */
			case 'A':			/* bob anim */
			case 'P':			/* program */
			case 'S':			/* sound effect */
			case 'E':			/* EdgeOfTheRoad */
				size += GENERICNAMESIZE;
				break;
			case 'O':			/* operator (recursive) */
				size += 4;
				break;
			default:
				D(bug("CalcOpSize(): unknown template field...\n") );
				break;
		}
	}
	return( size );
}


/**********************************************************/
// Insert an operator and params into a program at the given position
// ( 0 = head of list )
//

BOOL InsertOperator( struct Program *prog, UWORD op, WORD pos )
{
	struct OpNode *on, *noddy;
	BOOL success = FALSE;
//	UBYTE *template;

/*	printf("InsertOperator() at pos %d\n",pos ); */

	if( --pos >= 0)
		noddy = (struct OpNode *)FindNthNode( (struct List *)&prog->pg_OpList, pos );
	else
		noddy = (struct OpNode *)&prog->pg_OpList.mlh_Head;

	if( noddy )
	{
		if( on = AllocVec( sizeof( struct OpNode ), MEMF_ANY|MEMF_CLEAR ) )
		{
			if( on->on_OpData = AllocVec( CalcOpSize( op ), MEMF_ANY|MEMF_CLEAR ) )
			{
				*((UWORD *)on->on_OpData) = op;
				if( InitOpData( on->on_OpData ) )
				{
					Insert( (struct List *)&prog->pg_OpList, (struct Node *)on,
						(struct Node *)noddy );
					success = TRUE;
				}
				else
				{
					FreeVec( on->on_OpData );
					FreeVec( on );
				}
			}
			else
				FreeVec( on );
		}
	}

	return( success );
}


/**********************************************************/


struct OpNode *AllocOperator( UWORD op )
{
	struct OpNode *on;
	BOOL success = FALSE;

	if( on = AllocVec( sizeof( struct OpNode ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( on->on_OpData = AllocVec( CalcOpSize( op ), MEMF_ANY|MEMF_CLEAR ) )
		{
			*((UWORD *)on->on_OpData) = op;
			if( InitOpData( on->on_OpData ) )
			{
				success = TRUE;
			}
			else
			{
				FreeVec( on->on_OpData );
				FreeVec( on );
			}
		}
		else
			FreeVec( on );
	}

	if( !success )
		on = NULL;

	return( on );
}



/*****************************************************/
static BOOL InitOpData( UBYTE *opdata )
{
	UWORD op;
	UBYTE *template;
	BOOL allok;

	op = *((UWORD *)opdata);
	opdata += 2;
	template = operators[op].od_Template;

	allok = TRUE;

	while( *template && allok )
	{
		switch( *template++ )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'b':
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'V':			/* var */
			case 'p':
				opdata += 2;
				break;
			case 'f':			/* fixedpoint */
			case 'l':
				opdata += 4;
				break;
			case 'B':						/* bob image */
			case 'T':						/* path */
			case 'W':						/* weapon */
			case 'L':						/* levelref */
			case 'A':						/* anim */
			case 'P':						/* program */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
				opdata += GENERICNAMESIZE;
				break;
			case 's':						/* string (nullterminated) */
				if( !( *((STRPTR *)opdata) = MyStrDup( "" ) ) )
					allok = FALSE;
				opdata += 4;
				break;
			case 'O':						/* operator (we'll assume a channelroutine) */
				if( *((UBYTE **)opdata) = AllocVec( 2, MEMF_ANY ) )
				{
					/* 70 = Null channel routine */
					**((UWORD **)opdata) = 70;
				}
				else
					allok = FALSE;
				opdata += 4;
				break;
			default:
				D(bug("InitOpData(): unknown template field...\n") );
				break;
		}
	}

	return( allok );
}


/*****************************************************/
// Calculate the size of an operator as it will be stored on disk -
// ie, with the recursive operators and strings inserted.
//

ULONG CalcOpDiskSize( UBYTE *opdata )
{
	UWORD op;
	UBYTE *template;
	ULONG size;

	op = *((UWORD *)opdata);
	opdata += 2;
	size = 2;
	template = operators[op].od_Template;

	while( *template )
	{
		switch( *template++ )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'b':
				size += 1;
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'V':			/* var */
			case 'p':
				size += 2;
				opdata += 2;
				break;
			case 'l':
			case 'f':			/* fixedpoint */
				size += 4;
				opdata += 4;
				break;
			case 'B':						/* bob image */
			case 'T':						/* path */
			case 'W':						/* weapon */
			case 'L':						/* levelref */
			case 'A':						/* anim */
			case 'P':						/* program */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
				size += GENERICNAMEPACKSIZE;
				opdata += GENERICNAMESIZE;
				break;
			case 's':						/* string (nullterminated) */
				size += strlen( *((STRPTR *)opdata) )+1;
				/* pad to even length */
				if( size & 1 )
					size++;
				opdata += 4;
				break;
			case 'O':						/* operator */
				size += CalcOpDiskSize( *((UBYTE **)opdata) );
				opdata += 4;
				break;
//			default:
//				printf("CalcOpDiskSize(): unknown template field...\n");
//				break;
		}
	}
	return( size );
}


/*****************************************************/
// Calculate the size that an operator will end up when it is cooked
// by Lame.

ULONG CalcOpLameSize( UBYTE *opdata )
{
	UWORD op;
	UBYTE *template;
	ULONG size;

	op = *((UWORD *)opdata);
	opdata += 2;
	size = 4;			/* Lame replaces operator identifiers with routine pointers */
	template = operators[op].od_Template;

	while( *template )
	{
		switch( *template++ )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'b':
				size += 1;
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'V':			/* var */
				size += 2;
				opdata += 2;
				break;
			case 'f':			/* fixedpoint */
			case 'l':
				size += 4;
				opdata += 4;
				break;
			case 'p':						/* player ref */
				size += 4;
				opdata += 2;
				break;
			case 'B':						/* bob image */
			case 'T':						/* path */
			case 'W':						/* weapon */
			case 'A':						/* anim */
			case 'P':						/* program */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
				size += 4;
				opdata += GENERICNAMESIZE;
				break;
			case 'L':			/* levelref */
				size += GENERICNAMEPACKSIZE;	/* Lame leaves it in packed format */
				opdata += GENERICNAMESIZE;
				break;
			case 's':						/* string (nullterminated) */
				size += strlen( *((STRPTR *)opdata) )+1;
				/* pad to even length */
				if( size & 1 )
					size++;
				opdata += 4;
				break;
			case 'O':						/* operator */
				size += CalcOpLameSize( *((UBYTE **)opdata) );
				opdata += 4;
				break;
			default:
				D(bug("CalcOpLameSize(): unknown template field...\n") );
				break;
		}
	}
	return( size );
}


/*****************************************************/
// Write an operator to an open IFFHandle, into the current chunk.
//
//


BOOL WriteOperator( struct IFFHandle *iff, UBYTE *opdata )
{
	UWORD op, temp;
	UBYTE *template;
	BOOL allok = TRUE;
	UBYTE compbuf[32];		/* for compressing names */

	op = *((UWORD *)opdata);
	opdata += 2;
	template = operators[op].od_Template;

	/* output the operator identifier */
	if( WriteChunkBytes( iff, &op, 2 ) != 2 ) allok = FALSE;

	while( *template && allok )
	{
		switch( *template++ )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'b':
				if( WriteChunkBytes( iff, opdata, 1 ) != 1 ) allok = FALSE;
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'V':			/* var */
			case 'p':
				if( WriteChunkBytes( iff, opdata, 2 ) != 2 ) allok = FALSE;
				opdata += 2;
				break;
			case 'l':
			case 'f':			/* fixedpoint */
				if( WriteChunkBytes( iff, opdata, 4 ) != 4 ) allok = FALSE;
				opdata += 4;
				break;
			case 'B':						/* bob image */
			case 'T':						/* path */
			case 'W':						/* weapon */
			case 'L':						/* levelref */
			case 'A':						/* anim */
			case 'P':						/* program */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
				PackASCII( opdata, compbuf, GENERICNAMESIZE-1 );
				if( WriteChunkBytes( iff, compbuf, GENERICNAMEPACKSIZE ) != GENERICNAMEPACKSIZE )
					allok = FALSE;
				opdata += GENERICNAMESIZE;
				break;
			case 's':						/* string (nullterminated) */
				temp = strlen( *((STRPTR *)opdata) )+1;

				if( WriteChunkBytes( iff, *((STRPTR *)opdata), temp ) == temp )
				{
					if( temp & 1)
					{
						/* pad to even length */
						if( WriteChunkBytes( iff, "", 1 ) != 1 )
							allok = FALSE;
					}
				}
				else
					allok = FALSE;
				opdata += 4;
				break;
			case 'O':						/* operator */
				allok = WriteOperator( iff, *((UBYTE **)opdata) );
				opdata += 4;
				break;
			default:
				D(bug("WriteOperator(): unknown template field...\n") );
				break;
		}
	}
	return( allok );
}



/**************************************************************/

UBYTE *FindItemInOperator( UBYTE *opdata, UBYTE itemtype )
{
	UWORD op;
	UBYTE *match, *template;

	op = *((UWORD *)opdata);
	opdata += 2;

	match = NULL;
	template = operators[op].od_Template;

	while( *template && !match )
	{
		if( *template == itemtype )
			match = opdata;
		else
		{
			switch( *template++ )
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case 'b':
					opdata += 1;
					break;
				case 'w':
				case 'c':			/* CycleWord */
				case 'V':			/* var */
				case 'p':						/* player ref */
					opdata += 2;
					break;
				case 'l':
				case 'f':			/* fixedpoint */
				case 's':						/* string (nullterminated) */
					opdata += 4;
					break;
				case 'B':						/* bob image */
				case 'T':						/* path */
				case 'W':						/* weapon */
				case 'L':						/* levelref */
				case 'A':						/* anim */
				case 'P':						/* program */
				case 'S':						/* sound effect */
				case 'E':						/* EdgeOfTheRoad */
					opdata += GENERICNAMESIZE;
					break;
				case 'O':						/* operator */
					match = FindItemInOperator( *((UBYTE **)opdata), itemtype );
					opdata += 4;
					break;
				default:
					D(bug("FindItemInOperator(): unknown template field...\n") );
					break;
			}
		}
	}

	return( match );
}


/**************************************************************/

struct BobInfoNode *FindImageFromOperator( UBYTE *opdata, LONG *offsetbuffer,
	BOOL enterprogs )
{
	UWORD op;
	UBYTE *template;
	struct BobInfoNode *bin;
	struct AnimInfoNode *ain;
	struct Program *prog;

	op = *((UWORD *)opdata);
	opdata += 2;

	bin = NULL;
	template = operators[op].od_Template;

	while( *template && !bin )
	{
		switch( *template++ )
		{
			case 'B':						/* bob image */
				offsetbuffer[0] = 0;		/* x offset */
				offsetbuffer[1] = 0;		/* y offset */
				if( *opdata != '\0' )
					bin = FindBob( opdata );
				opdata += GENERICNAMESIZE;
				break;
			case 'A':						/* anim */
				if( *opdata != '\0' )
				{
					if( ain = FindAnim( opdata ) )
					{
						bin = AnimFirstFrame( ain, offsetbuffer );
					}
				}
				opdata += GENERICNAMESIZE;
				break;
			case 'P':						/* program */
				if( enterprogs )
				{
					/* search this program, but no deeper */
					if( prog = GlobalFindProg( opdata ) )
						bin = FindImageFromProgram( prog, offsetbuffer, FALSE );
				}
				opdata += GENERICNAMESIZE;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'b':
				opdata += 1;
				break;
			case 'w':
			case 'c':			/* CycleWord */
			case 'V':			/* var */
			case 'p':						/* player ref */
				opdata += 2;
				break;
			case 'l':
			case 'f':			/* fixedpoint */
			case 's':						/* string (nullterminated) */
				opdata += 4;
				break;
			case 'T':						/* path */
			case 'W':						/* weapon */
			case 'L':						/* levelref */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
				opdata += GENERICNAMESIZE;
				break;
			case 'O':						/* operator */
				if( *((UBYTE **)opdata) )
					bin = FindImageFromOperator( *((UBYTE **)opdata), offsetbuffer, enterprogs );
				opdata += 4;
				break;
			default:
				D(bug("FindImageFromOperator(): unknown template field...\n") );
				break;
		}
	}

	return( bin );
}


/********************************************************/

VOID DescribeOperator( STRPTR str, UBYTE *opdata )
{
	UWORD op, count;
	UBYTE *template, numbuf[16], *auxdata;
	APTR *aux;
	BOOL addcomma;
	int numbits;

	op = *((UWORD *)opdata);
	opdata += 2;
	template = operators[op].od_Template;
	aux = operators[op].od_Aux;

	/* output operator name */
	str = stpcpy( str, operators[op].od_Name );

	addcomma = FALSE;

	for( count = 0; ( count <= 20 ) && *template; count++ )
	{
		if( addcomma )
			str = stpcpy( str, "," );
		else
			str = stpcpy( str, " " );

		addcomma = TRUE;
		numbits = 0;

		switch( *template++ )
		{
			case '0':
				opdata += 1;
				addcomma = FALSE;
				break;
			case '8':
				numbits++;
			case '7':
				numbits++;
			case '6':
				numbits++;
			case '5':
				numbits++;
			case '4':
				numbits++;
			case '3':
				numbits++;
			case '2':
				numbits++;
			case '1':
				numbits++;

				while( numbits-- )
				{
					if( *((UBYTE *)opdata) & (1<<numbits) )
						*str++ = 'Y';
					else
						*str++ = 'N';
//					numbits--;
				}
				*str++ = '\0';
				opdata += 1;
				break;
			case 'b':
				sprintf( numbuf,"%ld",(LONG)(*((UBYTE *)opdata)) );
				str = stpcpy( str, numbuf );
				opdata += 1;
				break;
			case 'w':
				sprintf( numbuf,"%ld",(LONG)(*((WORD *)opdata)) );
				str = stpcpy( str, numbuf );
				opdata += 2;
				break;
			case 'c':			/* CycleWord */
				str = stpcpy( str, ((STRPTR *)(*aux++))[ *((WORD *)opdata) ] );
				opdata += 2;
				break;
			case 'p':						/* player ref */
				sprintf( numbuf,"%ld",(LONG)(*((WORD *)opdata)) );
				str = stpcpy( str, numbuf );
				opdata += 2;
				break;
			case 'l':
				sprintf( numbuf,"%ld",(LONG)(*((LONG *)opdata)) );
				str = stpcpy( str, numbuf );
				opdata += 4;
				break;
			case 'f':			/* fixedpoint */
				FixedPointToString( (LONG)(*((LONG *)opdata)), numbuf );
				str = stpcpy( str, numbuf );
				opdata += 4;
				break;
			case 'V':						/* var */
				str = stpcpy( str, varrefnames[ *((WORD *)opdata) ] );
				opdata += 2;
				break;
			case 's':						/* string (nullterminated) */
				if( *((STRPTR *)opdata) )
					str = stpcpy( str, *((STRPTR *)opdata) );
				opdata += 4;
				break;
			case 'B':						/* bob image */
			case 'T':						/* path */
			case 'W':						/* weapon */
			case 'L':						/* levelref */
			case 'A':						/* anim */
			case 'P':						/* program */
			case 'S':						/* sound effect */
			case 'E':						/* EdgeOfTheRoad */
				str = stpcpy( str, opdata );
				opdata += GENERICNAMESIZE;
				break;
			case 'O':						/* operator */
				auxdata = *((UBYTE **)opdata);
				if( auxdata )
				{
					op = *((UWORD *)auxdata);
					str = stpcpy( str, operators[op].od_Name );
				}
				opdata += 4;
				break;
			default:
				D(bug("DescribeOperator(): unknown template field...\n") );
				break;
		}
	}

}



/********************************************************/

BOOL MakeOperatorList( struct Program *prog, struct List *oplist )
{
	BOOL allok = TRUE;
	struct OpNode *on;
	struct DataNode *dn;
	UBYTE buf[512];

//	printf( "MakeOperatorList( %s )\n",prog->pg_Name );

	if( !IsListEmpty( oplist ) )
		FreeOperatorList( oplist );

	for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		on->on_Node.mln_Succ && allok;
		on = (struct OpNode *)on->on_Node.mln_Succ )
	{
		if( dn = AllocVec( sizeof(struct DataNode), MEMF_ANY|MEMF_CLEAR ) )
		{
			DescribeOperator( buf, on->on_OpData );
			dn->dn_Node.ln_Name = MyStrDup( buf );
			dn->dn_Data = on;
			AddTail( oplist, &dn->dn_Node );
		}
		else
			allok = FALSE;
	}

	return( allok );
}

/********************************************************/

VOID FreeOperatorList( struct List *oplist )
{
	struct DataNode *dn;

	while( !IsListEmpty( oplist ) )
	{
		dn = (struct DataNode *)oplist->lh_Head;
		Remove( (struct Node *)dn );
		if( dn->dn_Node.ln_Name )
			FreeVec( dn->dn_Node.ln_Name );
		FreeVec( dn );
	}
}

/***************************************************************/
// Converts an operator and params from disk form into our internal
// storage form. This involves copying out all embedded variable length
// data (like other operators) and replacing them with pointers.
//
// Inputs:
//	**opdata - pointer to a pointer to an operator. This is so opdata
//	can be updated as the process proceeds, and will end up pointing
//	to the data directly after the operator params.
//
// Result:
//	A pointer to the newly created internal-format operator.
//	NULL for error.

UBYTE *CookOperator( UBYTE **opdata )
{
	UBYTE *input, *output, *outbase, *template;
	UWORD op;
	ULONG temp;

	input = *opdata;											/* easier to handler than double pointers */
	op = *(UWORD *)input;									/* get operator */
	input += 2;
	template = operators[op].od_Template;	/* get its template */

	/* allocate enough memory to hold the cooked operator and params */
	if( outbase = AllocVec( 2 + operators[op].od_ParamSize, MEMF_ANY ) )
	{
		output = outbase;					/* remember base address for return */

		/* write out the operator word itself... */
		*((UWORD *)output) = op;
		output += 2;

//		printf("CookOperator: %s\n",operators[op].od_Name );

		/* go through and cook parameters one at a time using template */
		while( *template )
		{
//			printf( "%c",*template );
			switch( *template )
			{
/*				case 'D':		 dude??????? */
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case 'b':
//					printf("  Byte\n");
					*output++ = *input++;
					break;
				case 'w':			/* word */
				case 'c':			/* CycleWord */
				case 'V':			/* var */
//					printf("  Word\n");
					*output++ = *input++;
					*output++ = *input++;
					break;
				case 'f':			/* fixedpoint */
				case 'l':			/* long word */
//					printf("  Long\n");
					*output++ = *input++;
					*output++ = *input++;
					*output++ = *input++;
					*output++ = *input++;
					break;
				case 'p':			/* player ref */
//					printf("  PlayerRef\n");
					*output++ = *input++;
					*output++ = *input++;
					break;
				case 's':			/* string (nullterminated) */
//					printf("  String\n");
					/* replace variable length string with a pointer to a copy */
					*((STRPTR *)output) = MyStrDup( (STRPTR)input );
					output += 4;

					temp = strlen( (STRPTR)input ) + 1;
//					printf("%s\ntemp = %d\n",(STRPTR)input,temp);
					input += temp;
					/* odd length? If yep, skip pad. */
					if( temp & 1 )
					{
//						printf("Skip pad\n");
						input++;
					}
					break;
				case 'B':			/* bob image */
					UnpackASCII( input, output, BOBNAMESIZE-1 );
//					printf("  Bob: %s\n",output );
					input += BOBNAMEPACKSIZE;
					output += BOBNAMESIZE;
					break;
				case 'T':			/* path */
				case 'W':			/* weapon */
				case 'L':			/* levelref */
				case 'E':			/* EdgeOfTheRoad */
					UnpackASCII( input, output, GENERICNAMESIZE-1 );
					input += GENERICNAMEPACKSIZE;
					output += GENERICNAMESIZE;
					break;

				case 'A':			/* anim */
					UnpackASCII( input, output, ANIMNAMESIZE-1 );
//					printf("  Anim: %s\n",output );
					input += ANIMNAMEPACKSIZE;
					output += ANIMNAMESIZE;
					break;
				case 'P':			/* program */
					UnpackASCII( input, output, PROGNAMESIZE-1 );
//					printf("  Program: %s\n",output );
					input += PROGNAMEPACKSIZE;
					output += PROGNAMESIZE;
					break;
				case 'S':			/* sound effect */
					UnpackASCII( input, output, SFXNAMESIZE-1 );
//					printf("  Program: %s\n",output );
					input += SFXNAMEPACKSIZE;
					output += SFXNAMESIZE;
					break;
				case 'O':			/* operator (recursive) */
					*(UBYTE **)output = CookOperator( &input );
//					printf("  Operator\n" );
					output += 4;
					break;

				default:
					D(bug("CookOperator(): unknown template field: '%c'\n",*template) );
					break;
			}
			template++;
		}
	}

	*opdata = input;				/* need to update the ptr before returning */
	return( outbase );
}







#ifndef NDEBUG

/*****************************************************/
//
// Only used if NDEBUG is off.

VOID VerifyOpParamSize( VOID )
{
	UBYTE *template;
	UWORD i, size;
	LONG totsize;
	int j;

	printf("VerifyOpParamSize()...Checking %ld operators\n",
		(ULONG)numofoperators );

	totsize=0;

	for( i=0; i<numofoperators; i++ )
	{
		template = operators[i].od_Template;
		size = 0;

//		if( operators[i].od_Type == OPTYPE_ACTION )
//			printf("\nAction: " );
//		else
//			printf("\nChannelRoutine: " );

//		printf("%s\n", operators[i].od_Name );

		j=0;
		while( *template )
		{
			/* b=byte, w=word, l=longword, O=operator, P=program, A=anim, B=bobimage		*/
			/* S=soundeffect, p=playerref, T=Path, s = String			*/

//			printf("%c: %s\n", *template, operators[i].od_TemplateLabels[j++] );


			switch( *template++ )
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case 'b':
					size++;
					break;
				case 'w':
				case 'c':			/* CycleWord */
				case 'V':			/* var */
					size += 2;
					break;
				case 'f':			/* fixedpoint */
				case 'l':
					size += 4;
					break;
				case 'O':
					size += 4;
					break;
				case 'p':
					size += 2;
					break;
				case 'T':
				case 'W':
				case 'L':						/* levelref */
				case 'P':
				case 'A':
				case 'B':
				case 'S':
				case 'E':						/* EdgeOfTheRoad */
					size += GENERICNAMESIZE;
					break;
				case 's':
					size += 4;
					break;
			}
		}

		totsize += size;
		if( size != operators[i].od_ParamSize )
		{
			printf("WARNING: operator '%s'\n",operators[i].od_Name );
			printf("           ParamSize: %ld\n",(ULONG)operators[i].od_ParamSize );
			printf("  Expected ParamSize: %ld\n",(ULONG)size );
		}

	}
	printf("VerifyOpParamSize()...Done - totalsize = %ld\n", totsize );
}

#endif

