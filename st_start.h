
//**************************************************************************
//**
//** st_start.h : Heretic 2 : Raven Software, Corp.
//**
//** $Revision: 543 $
//** $Date: 2010-01-11 20:44:55 +0200 (Mon, 11 Jan 2010) $
//**
//**************************************************************************

#ifndef __ST_START__
#define __ST_START__

extern void ST_Init(void);
extern void ST_Done(void);

extern void ST_Progress(void);
extern void ST_NetProgress(void);
extern void ST_NetDone(void);

/* Maximum size of a debug message */
#define	MAX_ST_MSG		256

/* These two doesn't add a '\n' to the message, the caller must add it by himself */
extern void ST_Message(const char *message, ...);
extern void ST_RealMessage(const char *message, ...);

#endif	/* __ST_START__ */

