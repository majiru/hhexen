/* i_system.c */

#include "h2stdinc.h"
#include "h2def.h"

#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"

//#include "g_game.h"
//#include "m_misc.h"

int mb_used = 32;	/* 32MB heap */

void I_Init (void)
{
	S_Init();
	I_MouseEnable(1);
}

byte* I_ZoneBase (int *size)
{
	*size = mb_used*1024*1024;
	return (byte *) malloc(*size);
}

/* returns time in 1/70th second tics */
int I_GetTime (void)
{
	return (int)((nsec()*TICRATE)/1000000000);
}

static ticcmd_t emptycmd;
ticcmd_t* I_BaseTiccmd (void)
{
	return &emptycmd;
}

extern void G_CheckDemoStatus(void);

void I_Quit (void)
{
	D_QuitNetGame ();
	I_ShutdownSound();
	M_SaveDefaults ();
	I_ShutdownGraphics();
	postnote(PNGROUP, getpid(), "I_Quit");
	exits(nil);
}

byte* I_AllocLow (int length)
{
	byte *mem;
        
	mem = (byte *)malloc (length);
	memset (mem,0,length);
	return mem;
}

void I_Tactile(int on, int off, int total)
{
	USED(on, off, total);
}

//
// I_Error
//
extern boolean demorecording;

void I_Error (char *error, ...)
{
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();

    exits("I_Error");
}

int I_FileExists (char *filepath)
{
	return access(filepath, AEXIST) == 0;
}

int I_Open (char *filepath)
{
	return open(filepath, OREAD);
}

void I_Close (int handle)
{
	close (handle);
}

int I_Seek (int handle, int n)
{
	return seek(handle, n, 0);
}

int I_Read (int handle, void *buf, int n)
{
	return read(handle, buf, n);
}

void I_CheckExternDriver (void)
{
}

static char* strip(char *s)
{
	char *p;

	if(p = strstr(s, ".wad"))
		*p = '\0';
	if(p = strrchr(s, '/'))
		return p;
	return s;
}

static char bpd[512];
static char wd[512];

void I_SetupPath(char **wads)
{
	char **s;
	char *tmp;
	char *cfg, *data;
	char buf[512];

	data = cfg = nil;
	for(s = wads; *s; s++){
		if(!strstr(*s, ".wad"))
			continue;
		if(data == nil)
			data = *s;
		cfg = *s;
	}

	strcpy(buf, data);
	snprint(wd, sizeof wd, "/sys/games/lib/%s/", strip(buf));

	strcpy(buf, cfg);
	tmp = getenv("home");
	snprint(bpd, sizeof bpd, "%s/lib/%s/", tmp, strip(buf));
	free(tmp);

	// would suck if someone only found out after 2 hours
	// we could default back to cwd, but since this is our
	// write location, I'd rather not take chances.
	if(access(bpd, AEXIST) != 0)
		sysfatal("user directory %s not available %r", bpd);

	basePath = bpd;
	waddir = wd;
}
