
#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <Global.h>
#include <ExternVars.h>

#define LMB 1
#define MMB 2
#define RMB 4

static BOOL MoreMouseEvents(struct Window *win,
														struct IntuiMessage *imsg);
static void SubTaskHandler(void);
static void SubTaskProjMessageHandler(struct ProjectWindow *pw,struct IntuiMessage *imsg);
static void PassOnMessageAndReply(struct IntuiMessage *msg);

/*************************************************************************/
/******************************  SUB TASK STUFF **************************/
/*************************************************************************/
/*
				for(msg2 = (struct IntuiMessage *)subtaskport->mp_MsgList.lh_Head;
						msg2->ExecMessage.mn_Node.ln_Succ;
						msg2 = (struct IntuiMessage *)msg2->ExecMessage.mn_Node.ln_Succ)
				{
					if(msg2->Class == IDCMP_MOUSEBUTTONS)
						printf("Mouse Button: %s\n", (msg2->Code & IECODE_UP_PREFIX) ? "UP" : "DOWN");
				}
*/
/**************  SubTaskStart()  *********************/
//
//  This is the main() routine, for the sub-task, hence the __saveds.

void __saveds SubTaskStart()
{
	//Forbid();

	//printf("SubTask: CreateMsgPort()\n");

	/* Try to create our own message port */
	if( !(subtaskport = CreateMsgPort()) )
	{
		/* If failed, signal the main code to that fact */
		//Signal(&taskbase->pr_Task,SIGF_BAD);
		//Permit();
		return;
	}

	/* Wow, we got a port, cool, well signal the main code that we're up
		and away */
	//printf("SubTask: Signal(taskbase,SIGF_GOOD) \n");
	//Signal(&taskbase->pr_Task,SIGF_GOOD);
	//Permit();

	/* Call up our message handler, this will exit when it gets a message
		from our main code to do so */
	SubTaskHandler();
	
	MyDeleteMsgPort(subtaskport);
	subtaskport = NULL;
	/* Signal the main task, that we're done */
	Signal(&taskbase->pr_Task,SIGF_GOOD);
}




/***************  SubTaskHandler()  *********************/
//
//  This is the main sub task message handler bit. From here messages go out
// to other more refined message handlers.
//  If we get a SIGF_BAD signal, it means that we're to bugger off.

static void SubTaskHandler(void)
{
	struct	IntuiMessage		*msg;
	struct	SuperWindow			*sw;
	ULONG		sigset,subtasksignal;
	BOOL		done=FALSE, altpaging = FALSE;
	struct Project *pj;
	UWORD button;

	subtasksignal = 1L << subtaskport->mp_SigBit;
	subtaskgoing = TRUE;

	while(!done)
	{
		altpaging = FALSE;
		if(lastprojectwindow)
		{
			pj = lastprojectwindow->pw_Project;
			if( ((pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES)) &&
							 altdown && toolstate && (currentbrush->br_ID == BR_ANIMBRUSH) )
			{
				altpaging = TRUE;
				if(toolstate & MMB)
					button = IECODE_MBUTTON;
				if(toolstate & RMB)
					button = IECODE_RBUTTON;
				if(toolstate & LMB)
					button = IECODE_LBUTTON;
				Tool_Down(lastprojectwindow, lastprojectwindow->pw_SelX0, lastprojectwindow->pw_SelY0, button);
			}
		}
		if(!altpaging)
			sigset = Wait( subtasksignal | SIGF_BAD);
		else
			sigset = subtasksignal;

		if(sigset & subtasksignal)
		{
			while(msg = (struct IntuiMessage *)GetMsg(subtaskport) )
			{
				sw = (struct SuperWindow *)msg->IDCMPWindow->UserData;

				if( (sw) && (sw->sw_Type == SWTY_PROJECT) )
					SubTaskProjMessageHandler((struct ProjectWindow *)sw,msg);
				else
					ReplySubTaskMsg((struct Message *)msg);
			}
		}
		else
			done = TRUE;
	}
}

/*************************  SubTaskProjMessageHandler() ******************/
//
//  This handles all drawing stuff to do with the project Window.

static void SubTaskProjMessageHandler(struct ProjectWindow *pw,struct IntuiMessage *imsg)
{
	struct Project *pj;
	WORD mx,my,bmxoff,bmyoff;
	UWORD code;
	struct Window *win;
	WORD oldwinwidth,oldwinheight;

	pj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;
	mx = imsg->MouseX;
	my = imsg->MouseY;
	code = imsg->Code;

	GetBitMapOffsets(mx,my,&bmxoff,&bmyoff,pw);

	/* Clear the Break Signal, this is because we only want it to stop drawing
		if it gets a message _during_ a draw */
	SetSignal(0L,SIGF_BREAK);

	switch(imsg->Class)
	{
		/* We check that there are no more mousemove events, so that we don't bother
			processing mousemoves events that are just going to be overridden later */
		case IDCMP_MOUSEMOVE:
			if( (pj->pj_Edable) && (!MoreMouseEvents(pw->pw_sw.sw_Window,imsg)) )
				Tool_Move( pw, bmxoff, bmyoff );
			ReplySubTaskMsg((struct Message *)imsg);
			break;
		case IDCMP_MOUSEBUTTONS:
			if(pj->pj_Edable)
			{
				if(code & IECODE_UP_PREFIX)
					Tool_Up( pw, bmxoff, bmyoff, code & ~IECODE_UP_PREFIX);
				else
				{
					if( (mx >= win->BorderLeft) && (mx < win->Width - win->BorderRight ) &&
						 (my >= win->BorderTop) && (my < win->Height - win->BorderBottom ) )
						Tool_Down( pw, bmxoff, bmyoff, code);
				}
			}
			ReplySubTaskMsg((struct Message *)imsg);
			break;
		case IDCMP_INACTIVEWINDOW:
			ReplySubTaskMsg((struct Message *)imsg);
			EraseSelector(pw);
			break;
		case IDCMP_CHANGEWINDOW:
			ReplySubTaskMsg((struct Message *)imsg);
			oldwinwidth = pw->pw_sw.sw_Width;
			oldwinheight = pw->pw_sw.sw_Height;
			GetTrueWindowSize(pw);
			if( (pw->pw_sw.sw_Width != oldwinwidth) || (pw->pw_sw.sw_Height != oldwinheight) )
			{
				/* New Size */
				pw->pw_SelState = FALSE;
			}
			if(currenttool == TL_MAGNIFY)
				SetMagnifyBoxSize(pw);
			break;
		default:
			ReplySubTaskMsg((struct Message *)imsg);
			break;
	}
}

/**************************  MoreMouseEvents() *********************/
//
// Returns:	TRUE, if there are more IDCMP_MOUSEMOVE events qued
//					FALSE, if no more IDCMP_MOUSEMOVE events qued

static BOOL MoreMouseEvents(struct Window *win,
														struct IntuiMessage *imsg)
{
	struct Node *succ;

	if(imsg = (struct IntuiMessage *)imsg->ExecMessage.mn_Node.ln_Succ)
	{
		while( succ =  imsg->ExecMessage.mn_Node.ln_Succ )
		{
			if( (imsg->IDCMPWindow == win) && (imsg->Class == IDCMP_MOUSEMOVE) )
				return(TRUE);
			imsg = (struct IntuiMessage *)succ;
		}
	}
	return(FALSE);
}

/**************************  WaitForSubTaskDone() *******************/
//
// Wait till the sub task has processed all the messages sent to it.
//
// Parameters: mp = Reply port for the sub task (mainmsgport).
//

void WaitForSubTaskDone(struct MsgPort *mp)
{
	while(lastsubtaskmessage)
		Delay(5);
	WaitBlit();
}


/********************   FilterSubTaskMessages()   ************************/
//
//  This takes care of all the messages to/from the subtask.
// Returns: TRUE = Has been processed
//					FALSE = Not processed.

BOOL FilterSubTaskMessages(struct IntuiMessage *msg)
{
	struct	SuperWindow			*sw;
	struct	ProjectWindow		*pw;
	WORD		bmxoff,bmyoff;
	BOOL		ret = FALSE;

	sw = (struct SuperWindow *)msg->IDCMPWindow->UserData;

	/* If the Message is not for a project Window, then the sub-task has
		nothing to do with it, so we just pass it onto the normal handler */
	if(sw->sw_Type == SWTY_PROJECT)
	{
		pw = (struct ProjectWindow *)sw;
		pw->pw_MouseX = msg->MouseX;
		pw->pw_MouseY = msg->MouseY;

		if( (!pw->pw_Project->pj_Disabled) && (pw->pw_Project->pj_Edable) )
		{
			switch(msg->Class)
			{
				case IDCMP_MOUSEMOVE:
					GetBitMapOffsets(pw->pw_MouseX,pw->pw_MouseY,&bmxoff,&bmyoff,pw);

					/* If the mouse hasn't actually moved a 'True' pixel, then we don't
						want to send the subtask a new message as it will just slow
						things down more */
					if( (bmxoff == pw->pw_BMMouseX) && (bmyoff == pw->pw_BMMouseY) )
					{
						ReplyMsg((struct Message *)msg);
						ret = TRUE;
						break;
					}
					else
					{
						/* If mouse has moved, then store the new co-ords, and do the
							same stuff as for IDCMP_MOUSEBUTTONS */
						pw->pw_BMMouseX = bmxoff;
						pw->pw_BMMouseY = bmyoff;
					}
					if(lastsubtaskmessage)
						Signal(&subtaskproc->pr_Task,SIGF_BREAK);
					PassOnMessageAndReply(msg);
					ret = TRUE;
					break;
				case IDCMP_MOUSEBUTTONS:
					if(lastsubtaskmessage)
						Signal(&subtaskproc->pr_Task,SIGF_BREAK);
					PassOnMessageAndReply(msg);
					ret = TRUE;
					break;
				case IDCMP_CHANGEWINDOW:
					PassOnMessageAndReply(msg);
					ret = TRUE;
					break;
				case IDCMP_INACTIVEWINDOW:
					PassOnMessageAndReply(msg);
					ret = TRUE;
					break;
				case IDCMP_MENUVERIFY:
					SortOutProjMenuVerify(pw,msg);
					ret = TRUE;
					break;
				default:
					break;              
			}
		}
		else
		{
			/*  If the project is Disabled or not Edable then just
			 * reply to the message and say it's been processed */

			if( (msg->Class != IDCMP_CLOSEWINDOW) &&
					(msg->Class != IDCMP_MENUPICK) &&
					(msg->Class != IDCMP_REFRESHWINDOW) )
			{
				ReplyMsg((struct Message *)msg);
				ret = TRUE;
			}
		}
	}
	return(ret);
}


/************************  PassOnMessageAndReply()  ***************************/
//
//  Pass on the given message to the subtask, and update anything that needs
// to be updated.

static void PassOnMessageAndReply(struct IntuiMessage *msg)
{
	struct IntuiMessage *newmsg;

	Forbid();
	if(newmsg = AllocVec(sizeof(struct IntuiMessage), MEMF_PUBLIC|MEMF_CLEAR))
	{
		*newmsg = *msg;
		ReplyMsg((struct Message *)msg);
		newmsg->ExecMessage.mn_Node.ln_Succ = NULL;
		newmsg->ExecMessage.mn_Node.ln_Pred = NULL;
		newmsg->ExecMessage.mn_ReplyPort = NULL; //mainmsgport;
		newmsg->ExecMessage.mn_Length = sizeof(struct IntuiMessage);
		lastsubtaskmessage = newmsg;
		//printf("PutMsg(): %lx\n",newmsg);
		PutMsg(subtaskport,(struct Message *)newmsg);
	}
	else
		ReplyMsg((struct Message *)msg);
	Permit();
}


/***********************  SortOutProjMenuVerify()  *****************************/
//
//  Handle the menu verify for the project window.

void	SortOutProjMenuVerify(struct ProjectWindow *pw,struct IntuiMessage *imsg)
{
	UWORD	code;
	struct IntuiMessage *newmsg;
	struct Window *win;
	UWORD qual;

	code = imsg->Code;
	qual = imsg->Qualifier;

	if( (code == MENUHOT) && (!pw->pw_Project->pj_Disabled) && (pw->pw_Project->pj_Edable) && (qual & IEQUALIFIER_RBUTTON) )
	{
		/* the menu event came from this window, so we can cancel it
		 * if we feel the inclination. */

		win = pw->pw_sw.sw_Window;

		/* check mouse position against window boundaries */
		if( (imsg->MouseX >= win->BorderLeft) && (imsg->MouseX < (win->Width-win->BorderRight)) &&
			(imsg->MouseY >= win->BorderTop) && (imsg->MouseY < (win->Height-win->BorderBottom)) )
		{
			/* RMB down within window - cancel the menu, */
			/* and send the event on as a SELECTDOWN */
			imsg->Code = MENUCANCEL;
			ReplyMsg((struct Message *)imsg);

			Forbid();
			if(newmsg = AllocVec(sizeof(struct IntuiMessage),MEMF_PUBLIC|MEMF_CLEAR))
			{
				newmsg->ExecMessage.mn_Node.ln_Succ = NULL;
				newmsg->ExecMessage.mn_Node.ln_Pred = NULL;
				newmsg->ExecMessage.mn_ReplyPort = NULL; //mainmsgport;
				newmsg->ExecMessage.mn_Length = sizeof(struct IntuiMessage);
				newmsg->Class = IDCMP_MOUSEBUTTONS;
				newmsg->Code = IECODE_RBUTTON;
				newmsg->MouseX = pw->pw_MouseX;
				newmsg->MouseY = pw->pw_MouseY;
				newmsg->IDCMPWindow = pw->pw_sw.sw_Window;
				lastsubtaskmessage = newmsg;
				//printf("PutMsg(): %lx\n",newmsg);
				PutMsg(subtaskport,(struct Message *)newmsg);
			}
			Permit();
		}
		else
		{
			/* mouse outside window - let the menu come up as normal */
			ReplyMsg((struct Message *)imsg);
		}
	}
	else
	{
		/* menu event came from some other window - just let it go */
		ReplyMsg((struct Message *)imsg);
	}
}

void ReplySubTaskMsg(struct Message *msg)
{
	Forbid();
	if(msg == (struct Message *)lastsubtaskmessage)
		lastsubtaskmessage = NULL;
	FreeVec(msg);
	Permit();
}
