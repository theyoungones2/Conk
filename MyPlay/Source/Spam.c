//#include <stdio.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <devices/audio.h>
#include <graphics/gfxbase.h>
#include <hardware/custom.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
//#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>

#include "Spam.h"

static struct IOAudio *audioreq[4] = {NULL,NULL,NULL,NULL};		/* one play request for each channel */
static struct IOAudio *repeatreq[4] = {NULL,NULL,NULL,NULL};	/* one repeat request for each channel */
static BYTE	channelstate[4] = {0,0,0,0};		/* 1 means channel is busy */
static struct MsgPort *audioport = NULL;
static BYTE chanpri[4] = {0,0,0,0};
static ULONG clock;
static struct SpamParam	*samp_spamparam;
static UBYTE						*samp_data;
static UWORD						 samp_channel;
struct Task *playertask = NULL;
char *playertaskname = "PlayerTask";

UWORD	playertaskcommand = 0;
#define PTC_KillSpam 1
#define PTC_PlaySample 2
#define PTC_FinishAllSamples 3

extern struct GfxBase *GfxBase;
extern struct SpamParam MySpamParam;
extern struct Task *OurTaskBase;
extern struct Custom custom;

void __asm PlayerTask(void);
void __asm UpdateModulePlayer(void);

/*************************   StartPlayerSubTask()   ***********************/
BOOL __asm StartPlayerSubTask(void)
{
	BOOL ret = FALSE;
	ULONG sigbits;

	/* Init The Module Player SpamParam Thingy */
	SetSignal(0, SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
	if(playertask = CreateTask(playertaskname, 54, PlayerTask, 4096))
	{
		sigbits = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
		if(sigbits & SIGBREAKF_CTRL_C)
		{
			/* We Started Ok */
			ret = TRUE;
		}
		if(sigbits & SIGBREAKF_CTRL_D)
		{
			/* We Didn't Start Ok */
			printf("SubTask Didn't Start\n");
			ret = FALSE;
		}
	}
	else
		printf("CreateTask() Failed\n");

	return(ret);
}

void __asm EndPlayerSubTask(void)
{
	if(playertask)
	{
		SetSignal(0,SIGBREAKF_CTRL_C);
		playertaskcommand = PTC_KillSpam;
		Signal(playertask, SIGBREAKF_CTRL_D);	/* Tell The Sub Task To Kill Spam */
		Wait(SIGBREAKF_CTRL_C);					/* Wait for Spam to end */
		WaitTOF();											/* Wait just a little bit more */
		WaitTOF();											/* just to be on the safe side. */
		Forbid();
		DeleteTask(playertask);
		Permit();
	}
}

void __asm PlayerTask(void)
{
	ULONG sigbit,sigbits;
	UBYTE *ptr;
	struct IOAudio *ioa;
	UWORD chan;

	if(!Spam_Init())
	{
		Signal(OurTaskBase,SIGBREAKF_CTRL_C);
		sigbit = 1<<audioport->mp_SigBit;
		while(TRUE)
		{
			sigbits = Wait(sigbit | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D );
			if(audioport && (sigbits & sigbit))
			{
				while( ptr = (UBYTE *)GetMsg(audioport) )
				{
					/* Change Ptr to Start of IOAudio struct */
					/* ptr -= (sizeof(struct IOAudio) - sizeof(struct Message)); */
					ioa = (struct IOAudio *)ptr;
					for(chan = 0;
						 (chan <= 3) &&
						 (ioa < audioreq[chan]) && (ioa >= (audioreq[chan]+1)) &&
						 (ioa < repeatreq[chan]) && (ioa >= (repeatreq[chan]+1));
							chan++);
					if(chan <= 3)
					{
						if(channelstate > 0)
							channelstate[chan] -= 1;
					}
				}
			}
			if(sigbits & SIGBREAKF_CTRL_C)
			{
				SetSignal(0,SIGBREAKF_CTRL_C);
				UpdateModulePlayer();
			}
			if(sigbits & SIGBREAKF_CTRL_D)
			{
				SetSignal(0,SIGBREAKF_CTRL_D);
				switch(playertaskcommand)
				{
					case PTC_KillSpam:
						Spam_Die();
						sigbit = SIGBREAKF_CTRL_D;	/* Don't Wait for a dead Message Port */
						break;
					case PTC_PlaySample:
						Spam_PlaySound(samp_spamparam, samp_data, samp_channel);
						break;
					case PTC_FinishAllSamples:
						Spam_FinishAllSamples();
						break;
				}
			}
		}
	}
	Signal(OurTaskBase,SIGBREAKF_CTRL_D);
	playertask = NULL;
}

/*************************   PlaySample()  ****************************/
//
//  Call this routine to play a sample from the Parent Process.

void __asm PlaySample( register __a0 struct SpamParam *spp,
	register __a1 UBYTE *sampledata, register __d0 UWORD chan )
{
	samp_spamparam = spp;
	samp_data = sampledata;
	samp_channel = chan;
	playertaskcommand = PTC_PlaySample;
	Signal(playertask, SIGBREAKF_CTRL_D);
}

/*************************   FinishAllSamples()  ****************************/
//
//  Call this routine to FinishAllCurrentlyPlaying Samples

void __asm FinishAllSamples(void)
{
	playertaskcommand = PTC_FinishAllSamples;
	Signal(playertask, SIGBREAKF_CTRL_D);
}


/**************************************************************************/
//
// Spam_Init()
//

UWORD __asm Spam_Init( VOID )
{
	UWORD status;
	UWORD i;
	UBYTE channelcombo;

	status = SPAMERR_ALLOK;
	channelcombo = 15;

	/* set up a messageport for talking to the audio device */
	if( audioport = CreatePort( NULL, 0 ) )
	{
		/* Create an IO request struct for each play channel */
		for( i = 0; i <= 3; i++ )
		{
			audioreq[i] = (struct IOAudio *)CreateExtIO( audioport,
				sizeof( struct IOAudio ) );
			if( !audioreq[i] )
				status = SPAMERR_MEM;
		}
		/* Create an IO request struct for each repeat channel */
		for( i = 0; i <= 3; i++ )
		{
			repeatreq[i] = (struct IOAudio *)CreateExtIO( audioport,
				sizeof( struct IOAudio ) );
			if( !audioreq[i] )
				status = SPAMERR_MEM;
		}

		if( status == SPAMERR_ALLOK )
		{
			/* open the audio device and allocate channels */
			audioreq[0]->ioa_Request.io_Message.mn_Node.ln_Pri = 127;
			audioreq[0]->ioa_Request.io_Command = ADCMD_ALLOCATE;
			audioreq[0]->ioa_Request.io_Flags = ADIOF_NOWAIT;
			audioreq[0]->ioa_AllocKey = 0;
			audioreq[0]->ioa_Data = &channelcombo;
			audioreq[0]->ioa_Length = 1;

			if( !OpenDevice( AUDIONAME, 0, (struct IORequest *)audioreq[0], 0 ) )
			{
				/* make copies of the opening request */
				CopyMem( audioreq[0], repeatreq[0], sizeof( struct IOAudio ) );
				for( i=1; i<=3; i++ )
				{
					CopyMem( audioreq[0], audioreq[i], sizeof( struct IOAudio ) );
					CopyMem( audioreq[0], repeatreq[i], sizeof( struct IOAudio ) );
				}

				/* Execute a CMD_RESET command on each request to get around the				*/
				/* problem where CheckIO() can hang if used on a request which hasn't		*/
				/* be used.	Anyway, it probably don't hurt to reset the audio hardware.	*/
				/* Also, we set the io_Unit field for future use. */

				for( i=0; i<=3; i++ )
				{
					audioreq[i]->ioa_Request.io_Unit = (APTR)(1<<i);
					audioreq[i]->ioa_Request.io_Flags = 0;
					audioreq[i]->ioa_Request.io_Command = CMD_RESET;
					audioreq[i]->ioa_Length = 0;
					BeginIO( (struct IORequest *)audioreq[i] );
					WaitIO( (struct IORequest *)audioreq[i] );
					repeatreq[i]->ioa_Request.io_Unit = (APTR)(1<<i);
					repeatreq[i]->ioa_Request.io_Flags = 0;
					repeatreq[i]->ioa_Request.io_Command = CMD_RESET;
					repeatreq[i]->ioa_Length = 0;
					BeginIO( (struct IORequest *)repeatreq[i] );
					WaitIO( (struct IORequest *)repeatreq[i] );
				}

				/* check PAL/NTSC and adjust the clock constant accordingly */
				if( GfxBase->DisplayFlags & PAL )
					clock = 3546895L;
				else
					clock = 3579545L;
			}
			else
				status = SPAMERR_AUDIODEVICE;
		}
	}
	else
		status = SPAMERR_MEM;

	if( status != SPAMERR_ALLOK )
	{
		/* clean up if there were any fuckups */
		for( i=0; i<4; i++ )
		{
			if( audioreq[i] )
				DeleteExtIO( (struct IORequest *)audioreq[i] );
			audioreq[i] = NULL;
			if( repeatreq[i] )
				DeleteExtIO( (struct IORequest *)repeatreq[i] );
			repeatreq[i] = NULL;
		}
		if( audioport )
			DeletePort( audioport );
		audioport = NULL;
	}
	return( status );
}

/**************************************************************************/

VOID __asm Spam_Die( VOID )
{
	UWORD i;

	if( audioport )
	{
		/* finish any pending IO */
		for( i = 0; i < 4; i++ )
		{
			if( !CheckIO( (struct IORequest *)audioreq[ i ] ) )
			{
				AbortIO( (struct IORequest *)audioreq[ i ] );
				WaitIO( (struct IORequest *)audioreq[ i ] );
			}
			if( !CheckIO( (struct IORequest *)repeatreq[ i ] ) )
			{
				AbortIO( (struct IORequest *)repeatreq[ i ] );
				WaitIO( (struct IORequest *)repeatreq[ i ] );
			}
		}
		CloseDevice( (struct IORequest *)audioreq[0] );

		for( i = 0; i < 4; i++ )
		{
			DeleteExtIO( (struct IORequest *)audioreq[i] );
			DeleteExtIO( (struct IORequest *)repeatreq[i] );
		}
		DeletePort( audioport );
		audioport = NULL;
	}
	Signal(OurTaskBase,SIGBREAKF_CTRL_C);
}

/**************************************************************************/

BOOL __asm Spam_PlaySound( register __a0 struct SpamParam *spp,
	register __a1 UBYTE *sampledata, register __d0 UWORD chan )
{
	BOOL play;
	UWORD i;
	BYTE lowest;

	play = TRUE;

	if( !audioport )
		return( FALSE );

	if( chan == 4 )
	{
		for( i=0; i<4 && chan==4; i++ )
		{
			if( CheckIO( (struct IORequest *)audioreq[ i ] ) )
				chan = i;
		}
		/* all busy? find lowest pri */
		if( chan == 4 )
		{
			lowest = 127;
			for( i=0; i<4; i++ )
			{
				if( chanpri[ i ] <=lowest )
				{
					chan = i;
					lowest = chanpri[ i ];
				}
			}
			// chan = 0;
		}
	}

	if( !CheckIO( (struct IORequest *)audioreq[ chan ] ) )
	{
		/* there is a sample already playing - see if we should override */
		if( spp->spam_Pri >= chanpri[ chan ] )
		{
			/* kill currently-playing sample */
			AbortIO( (struct IORequest *)audioreq[ chan ] );
			WaitIO( (struct IORequest *)audioreq[ chan ] );
		}
		else
			play = FALSE;
	}
	if( !CheckIO( (struct IORequest *)repeatreq[ chan ] ) )
	{
		/* there is a sample already playing - see if we should override */
		if( spp->spam_Pri >= chanpri[ chan ] )
		{
			/* kill currently-playing sample */
			AbortIO( (struct IORequest *)repeatreq[ chan ] );
			WaitIO( (struct IORequest *)repeatreq[ chan ] );
		}
		else
			play = FALSE;
	}

	if( play )
	{
		/* set sample going */
		audioreq[chan]->ioa_Request.io_Command = CMD_WRITE;
		audioreq[chan]->ioa_Request.io_Flags = ADIOF_PERVOL|IOF_QUICK;
		audioreq[chan]->ioa_Data = sampledata;
		audioreq[chan]->ioa_Length = spp->spam_Length;
		audioreq[chan]->ioa_Period = (spp->spam_Flags & SPAMF_ISPERIOD) ? spp->spam_SampleRate : clock/spp->spam_SampleRate;
		audioreq[chan]->ioa_Volume = spp->spam_Volume;
		audioreq[chan]->ioa_Cycles = 1;
		BeginIO( (struct IORequest *)audioreq[chan] );
		channelstate[chan] = 1;
		if((spp->spam_Repeats != 0) && spp->spam_RepeatLength)
		{
			repeatreq[chan]->ioa_Request.io_Command = CMD_WRITE;
			repeatreq[chan]->ioa_Request.io_Flags = IOF_QUICK; /* ADIOF_PERVOL|IOF_QUICK; */
			repeatreq[chan]->ioa_Data = sampledata + spp->spam_RepeatStart;
			repeatreq[chan]->ioa_Length = spp->spam_RepeatLength;
			repeatreq[chan]->ioa_Period = (spp->spam_Flags & SPAMF_ISPERIOD) ? spp->spam_SampleRate : clock/spp->spam_SampleRate;
			repeatreq[chan]->ioa_Volume = spp->spam_Volume;
			repeatreq[chan]->ioa_Cycles = (spp->spam_Repeats == -1) ? 0 : spp->spam_Repeats;
			BeginIO( (struct IORequest *)repeatreq[chan] );
			channelstate[chan] += 1;
		}
		chanpri[chan] = spp->spam_Pri;
	}
	return( play );
}

void __asm Spam_FinishAllSamples( void )
{
	UWORD chan;

	for(chan = 0; chan < 4; chan++)
	{
		if(channelstate[chan] != 0)
		{
			audioreq[chan]->ioa_Request.io_Command = ADCMD_FINISH;
			audioreq[chan]->ioa_Request.io_Flags = IOF_QUICK;
			BeginIO( (struct IORequest *)audioreq[chan] );
			channelstate[chan] = 0;
		}
	}
}
