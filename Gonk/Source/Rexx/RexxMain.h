/*
 * Source generated with ARexxBox 1.12 (May 18 1993)
 * which is Copyright (c) 1992,1993 Michael Balzer
 */

#ifndef _RexxMain_H
#define _RexxMain_H

#define RXIF_INIT   1
#define RXIF_ACTION 2
#define RXIF_FREE   3

#define ARB_CF_ENABLED     (1L << 0)

#define ARB_HF_CMDSHELL    (1L << 0)
#define ARB_HF_USRMSGPORT  (1L << 1)

struct RexxHost
{
	struct MsgPort *port;
	char portname[ 80 ];
	long replies;
	struct RDArgs *rdargs;
	long flags;
	APTR userdata;
};

struct rxs_command
{
	char *command, *args, *results;
	long resindex;
	void (*function)( struct RexxHost *, void **, long, struct RexxMsg * );
	long flags;
};

struct arb_p_link
{
	char	*str;
	int		dst;
};

struct arb_p_state
{
	int		cmd;
	struct arb_p_link *pa;
};

#ifndef NO_GLOBALS
extern char RexxPortBaseName[80];
extern struct rxs_command rxs_commandlist[];
extern struct arb_p_state arb_p_state[];
extern int command_cnt;
extern char *rexx_extension;
#endif

void ReplyRexxCommand( struct RexxMsg *rxmsg, long prim, long sec, char *res );
void FreeRexxCommand( struct RexxMsg *rxmsg );
struct RexxMsg *CreateRexxCommand( struct RexxHost *host, char *buff, BPTR fh );
struct RexxMsg *CommandToRexx( struct RexxHost *host, struct RexxMsg *rexx_command_message );
struct RexxMsg *SendRexxCommand( struct RexxHost *host, char *buff, BPTR fh );

void CloseDownARexxHost( struct RexxHost *host );
struct RexxHost *SetupARexxHost( char *basename, struct MsgPort *usrport );
struct rxs_command *FindRXCommand( char *com );
char *ExpandRXCommand( struct RexxHost *host, char *command );
char *StrDup( char *s );
void ARexxDispatch( struct RexxHost *host );

void DoShellCommand( struct RexxHost *host, char *comline, BPTR fhout );
void CommandShell( struct RexxHost *host, BPTR fhin, BPTR fhout, char *prompt );

/* rxd-Strukturen dürfen nur AM ENDE um lokale Variablen erweitert werden! */

struct rxd_activatewindow
{
	long rc, rc2;
};

void rx_activatewindow( struct RexxHost *, struct rxd_activatewindow **, long, struct RexxMsg * );

struct rxd_activewindow
{
	long rc, rc2;
	struct {
		char *var, *stem;
	} arg;
	struct {
		char *windowid;
	} res;
};

void rx_activewindow( struct RexxHost *, struct rxd_activewindow **, long, struct RexxMsg * );

struct rxd_changewindow
{
	long rc, rc2;
	struct {
		long *leftedge;
		long *topedge;
		long *width;
		long *height;
	} arg;
};

void rx_changewindow( struct RexxHost *, struct rxd_changewindow **, long, struct RexxMsg * );

struct rxd_clear
{
	long rc, rc2;
	struct {
		long force;
	} arg;
};

void rx_clear( struct RexxHost *, struct rxd_clear **, long, struct RexxMsg * );

struct rxd_close
{
	long rc, rc2;
	struct {
		long force;
	} arg;
};

void rx_close( struct RexxHost *, struct rxd_close **, long, struct RexxMsg * );

struct rxd_findwindow
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *name;
	} arg;
	struct {
		char *windowid;
	} res;
};

void rx_findwindow( struct RexxHost *, struct rxd_findwindow **, long, struct RexxMsg * );

struct rxd_movewindow
{
	long rc, rc2;
	struct {
		long *leftedge;
		long *topedge;
	} arg;
};

void rx_movewindow( struct RexxHost *, struct rxd_movewindow **, long, struct RexxMsg * );

struct rxd_new
{
	long rc, rc2;
	struct {
		char *var, *stem;
	} arg;
	struct {
		char *windowid;
	} res;
};

void rx_new( struct RexxHost *, struct rxd_new **, long, struct RexxMsg * );

struct rxd_open
{
	long rc, rc2;
	struct {
		char *var, *stem;
		char *filename;
	} arg;
	struct {
		char *windowid;
	} res;
};

void rx_open( struct RexxHost *, struct rxd_open **, long, struct RexxMsg * );

struct rxd_quit
{
	long rc, rc2;
	struct {
		long force;
	} arg;
};

void rx_quit( struct RexxHost *, struct rxd_quit **, long, struct RexxMsg * );

struct rxd_save
{
	long rc, rc2;
};

void rx_save( struct RexxHost *, struct rxd_save **, long, struct RexxMsg * );

struct rxd_saveas
{
	long rc, rc2;
	struct {
		char *name;
	} arg;
};

void rx_saveas( struct RexxHost *, struct rxd_saveas **, long, struct RexxMsg * );

struct rxd_selectwindow
{
	long rc, rc2;
	struct {
		char *windowid;
	} arg;
};

void rx_selectwindow( struct RexxHost *, struct rxd_selectwindow **, long, struct RexxMsg * );

struct rxd_sizewindow
{
	long rc, rc2;
	struct {
		long *width;
		long *height;
	} arg;
};

void rx_sizewindow( struct RexxHost *, struct rxd_sizewindow **, long, struct RexxMsg * );

struct rxd_unzoomwindow
{
	long rc, rc2;
};

void rx_unzoomwindow( struct RexxHost *, struct rxd_unzoomwindow **, long, struct RexxMsg * );

struct rxd_windowtoback
{
	long rc, rc2;
};

void rx_windowtoback( struct RexxHost *, struct rxd_windowtoback **, long, struct RexxMsg * );

struct rxd_windowtofront
{
	long rc, rc2;
};

void rx_windowtofront( struct RexxHost *, struct rxd_windowtofront **, long, struct RexxMsg * );

struct rxd_zoomwindow
{
	long rc, rc2;
};

void rx_zoomwindow( struct RexxHost *, struct rxd_zoomwindow **, long, struct RexxMsg * );

#endif
