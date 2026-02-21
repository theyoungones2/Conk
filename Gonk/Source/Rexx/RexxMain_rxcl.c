/*
 * Source generated with ARexxBox 1.12 (May 18 1993)
 * which is Copyright (c) 1992,1993 Michael Balzer
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <rexx/storage.h>

#define NO_GLOBALS
#include "RexxMain.h"

#define RESINDEX(stype) (((long) &((struct stype *)0)->res) / sizeof(long))

char	RexxPortBaseName[80] = "GONK";
char	*rexx_extension = "GONK";

struct rxs_command rxs_commandlist[] =
{
	{ "ACTIVATEWINDOW", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_activatewindow, 1 },
	{ "ACTIVEWINDOW", NULL, "WINDOWID", RESINDEX(rxd_activewindow), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_activewindow, 1 },
	{ "CHANGEWINDOW", "LEFTEDGE/N,TOPEDGE/N,WIDTH/N,HEIGHT/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_changewindow, 1 },
	{ "CLEAR", "FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_clear, 1 },
	{ "CLOSE", "FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_close, 1 },
	{ "FINDWINDOW", "NAME", "WINDOWID", RESINDEX(rxd_findwindow), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_findwindow, 1 },
	{ "MOVEWINDOW", "LEFTEDGE/N,TOPEDGE/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_movewindow, 1 },
	{ "NEW", NULL, "WINDOWID", RESINDEX(rxd_new), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_new, 1 },
	{ "OPEN", "FILENAME/K", "WINDOWID", RESINDEX(rxd_open), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_open, 1 },
	{ "QUIT", "FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_quit, 1 },
	{ "SAVE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_save, 1 },
	{ "SAVEAS", "NAME/K", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_saveas, 1 },
	{ "SELECTWINDOW", "WINDOWID", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_selectwindow, 1 },
	{ "SIZEWINDOW", "WIDTH/N,HEIGHT/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_sizewindow, 1 },
	{ "UNZOOMWINDOW", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_unzoomwindow, 1 },
	{ "WINDOWTOBACK", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_windowtoback, 1 },
	{ "WINDOWTOFRONT", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_windowtofront, 1 },
	{ "ZOOMWINDOW", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_zoomwindow, 1 },
	{ NULL, NULL, NULL, NULL, NULL }
};

int		command_cnt = 18;

static struct arb_p_link link0[] = {
	{"ZOOMWINDOW", 1}, {"WINDOWTO", 2}, {"UNZOOMWINDOW", 5}, {"S", 6}, {"QUIT", 11}, {"OPEN", 12},
	{"NEW", 13}, {"MOVEWINDOW", 14}, {"FINDWINDOW", 15}, {"C", 16}, {"ACTIV", 21}, {NULL, 0} };

static struct arb_p_link link2[] = {
	{"FRONT", 3}, {"BACK", 4}, {NULL, 0} };

static struct arb_p_link link6[] = {
	{"IZEWINDOW", 7}, {"ELECTWINDOW", 8}, {"AVE", 9}, {NULL, 0} };

static struct arb_p_link link9[] = {
	{"AS", 10}, {NULL, 0} };

static struct arb_p_link link16[] = {
	{"L", 17}, {"HANGEWINDOW", 20}, {NULL, 0} };

static struct arb_p_link link17[] = {
	{"OSE", 18}, {"EAR", 19}, {NULL, 0} };

static struct arb_p_link link21[] = {
	{"EWINDOW", 22}, {"ATEWINDOW", 23}, {NULL, 0} };

struct arb_p_state arb_p_state[] = {
	{-1, link0}, {17, NULL}, {15, link2}, {16, NULL}, {15, NULL},
	{14, NULL}, {10, link6}, {13, NULL}, {12, NULL}, {10, link9},
	{11, NULL}, {9, NULL}, {8, NULL}, {7, NULL}, {6, NULL},
	{5, NULL}, {2, link16}, {3, link17}, {4, NULL}, {3, NULL},
	{2, NULL}, {0, link21}, {1, NULL}, {0, NULL}  };

