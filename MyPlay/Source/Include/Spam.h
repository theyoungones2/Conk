
#define SFXNAMEPACKSIZE 10

struct SpamParam
{
	UBYTE	spam_PackedName[ SFXNAMEPACKSIZE ];
	BYTE	spam_Pri;
	UBYTE	spam_Volume;
	UWORD	spam_SampleRate;		/* in samples/sec */
	ULONG	spam_Length;				/* length of sampledata in bytes */
	LONG	spam_RepeatStart;		/* Start offset of the repeat data */
	ULONG	spam_RepeatLength;	/* Repeat Data Length ( 0 = No Repeat ) */
	UWORD	spam_Flags;					/* See Defines Below */
	WORD spam_Repeats;				/* How many times to play the repeatsection */
														/* -1=forever */
	UWORD	spam_Reserved[4];
};

/* If IsPeriod == 0 then spam_SampleRate is the actual Sample Rate, else it's
		the Period to play the sample at. */
#define SPAMB_ISPERIOD 0
#define SPAMF_ISPERIOD 1<<SPAMB_ISPERIOD

#define SPAMERR_ALLOK					0
#define SPAMERR_MEM						1
#define SPAMERR_AUDIODEVICE		2

BOOL __asm StartPlayerSubTask(void);
void __asm EndPlayerSubTask(void);
void __asm PlaySample( register __a0 struct SpamParam *spp,
	register __a1 UBYTE *sampledata, register __d0 UWORD chan );
void __asm FinishAllSamples(void);

UWORD __asm Spam_Init( VOID );
VOID __asm Spam_Die( VOID );

BOOL __asm Spam_PlaySound( register __a0 struct SpamParam *sp,
	register __a1 UBYTE *sampledata, register __d0 UWORD chan );
void __asm Spam_FinishAllSamples( void );

