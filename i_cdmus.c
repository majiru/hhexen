
//**************************************************************************
//**
//** i_cdmus.c
//**
//** $Revision: 562 $
//** $Date: 2010-10-20 15:45:52 +0300 (Wed, 20 Oct 2010) $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2stdinc.h"
#include "h2def.h"
#include "i_cdmus.h"

// MACROS ------------------------------------------------------------------

#define MAX_AUDIO_TRACKS	25

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int cdaudio;	/* boolean: enabled or disabled */

boolean i_CDMusic;
int i_CDTrack;
int i_CDCurrentTrack;
int i_CDMusicLength;
int oldTic;

int cd_Error;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

#if 0 /* nothing is here yet */
static int cd_FirstTrack;
static int cd_LastTrack;
static char cd_dev[64] = "/dev/cdrom";
#endif
static int cdfile = -1;

// CODE --------------------------------------------------------------------

static int I_CDGetDiskInfo(void)
{
	return 0;
}

//==========================================================================
//
// I_CDMusInit
//
// Initializes the CD audio system.  Must be called before using any
// other I_CDMus functions.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusInit(void)
{
	//open CD device
	I_CDGetDiskInfo ();
	return -1;	// not implemented yet
}

//==========================================================================
//
// I_CDMusPlay
//
// Play an audio CD track.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusPlay(int)
{
	return 0;
}

//==========================================================================
//
// I_CDMusStop
//
// Stops the playing of an audio CD.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusStop(void)
{
	return 0;
}

//==========================================================================
//
// I_CDMusResume
//
// Resumes the playing of an audio CD.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusResume(void)
{
	return 0;
}

//==========================================================================
//
// I_CDMusSetVolume
//
// Sets the CD audio volume (0 - 255).
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusSetVolume(int)
{
	return 0;
}

//==========================================================================
//
// I_CDMusFirstTrack
//
// Returns: the number of the first track.
//
//==========================================================================

int I_CDMusFirstTrack(void)
{
	return 0;
}

//==========================================================================
//
// I_CDMusLastTrack
//
// Returns: the number of the last track.
//
//==========================================================================

int I_CDMusLastTrack(void)
{
	return 0;
}

//==========================================================================
//
// I_CDMusShutDown
//
//==========================================================================

void I_CDMusShutdown(void)
{
	if (cdfile != -1)
		close(cdfile);
	cdfile = -1;
}

//==========================================================================
//
// I_CDMusUpdate
//
//==========================================================================

void I_CDMusUpdate(void)
{

}

