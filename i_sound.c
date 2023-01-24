//**************************************************************************
//**
//** i_soundpi.c: unix sound driver using a plugin interface
//**
//** $Revision: 512 $
//** $Date: 2009-06-04 18:00:34 +0300 (Thu, 04 Jun 2009) $
//**
//**************************************************************************


#include "h2stdinc.h"
#include "h2def.h"
#include "sounds.h"
#include "i_sound.h"
#include "audio_plugin.h"

#define SAMPLE_ZERO	0
#define SAMPLE_RATE	11025	/* Hz */
#define SAMPLE_CHANNELS	2
#define TARGET_RATE	44100

#define SAMPLE_TYPE	short

/*
 *	SOUND HEADER & DATA
 */

int snd_Channels;
int snd_MaxVolume,		/* maximum volume for sound */
	snd_MusicVolume;	/* maximum volume for music */
boolean snd_MusicAvail,		/* whether music is available */
	snd_SfxAvail;		/* whether sfx are available */

/*
 *	SOUND FX API
 */

typedef struct
{
	unsigned char	*begin;		/* pointers into Sample.firstSample */
	unsigned char	*end;

	SAMPLE_TYPE	*lvol_table;	/* point into vol_lookup */
	SAMPLE_TYPE	*rvol_table;

	unsigned int	pitch_step;
	unsigned int	step_remainder;	/* 0.16 bit remainder of last step. */

	int		pri;
	unsigned int	time;
} Channel;

#pragma pack on
typedef struct
{
/* Sample data is a lump from a wad: byteswap the a, freq
 * and the length fields before using them		*/
	short		a;		/* always 3	*/
	short		freq;		/* always 11025	*/
	int32_t		length;		/* sample length */
	unsigned char	firstSample;
} Sample;
#pragma pack off

static int	audio_exit_thread = 1;
//static pthread_t	audio_thread;


#define CHAN_COUNT	8
static Channel	channel[CHAN_COUNT];

#define MAX_VOL		64	/* 64 keeps our table down to 16Kb */
static SAMPLE_TYPE	vol_lookup[MAX_VOL * 256];

static int	steptable[256];		/* Pitch to stepping lookup */

#define BUF_LEN		(256 * 2 * 4)

static int audiofd;

static QLock audiolk;

boolean mus_paused = false;

static int mpfd[2] = {-1, -1};

static void audioproc(void)
{
	Channel* chan;
	Channel* cend;
	char buf[BUF_LEN];
	SAMPLE_TYPE *begin;
	SAMPLE_TYPE *end;
	unsigned int sample;
	register int dl, ml;
	register int dr, mr;
	int i;

	end = (SAMPLE_TYPE *) (buf + BUF_LEN);
	cend = channel + CHAN_COUNT;

	procsetname("hexen audioproc");
	for(;;){
		memset(buf, 0, sizeof buf);
		if(mpfd[0]>=0 && !mus_paused && readn(mpfd[0], buf, sizeof buf) < 0){
			fprint(2, "I_UpdateSound: disabling music: %r\n");
			I_ShutdownMusic();
		}
		begin = (SAMPLE_TYPE *) buf;
		while (begin < end){
			// Mix all the channels together.
			dl = SAMPLE_ZERO;
			dr = SAMPLE_ZERO;

			qlock(&audiolk);
			chan = channel;
			for ( ; chan < cend; chan++){
				if(!chan->begin)
					continue;
				// Get the sample from the channel.
				sample = *chan->begin;

				// Adjust volume accordingly.
				dl += chan->lvol_table[sample];
				dr += chan->rvol_table[sample];

				// Increment sample pointer with pitch adjustment.
				chan->step_remainder += chan->pitch_step;
				chan->begin += chan->step_remainder >> 16;
				chan->step_remainder &= 65535;

				// Check whether we are done.
				if (chan->begin >= chan->end)
				{
					chan->begin = NULL;
				//printf ("  channel done %d\n", chan);
				}
			}
			qunlock(&audiolk);
			for(i=0; i < TARGET_RATE/SAMPLE_RATE; i++){
				ml = dl + *begin;
				if (ml > 0x7fff)
					ml = 0x7fff;
				else if (ml < -0x8000)
					ml = -0x8000;
				*begin++ = ml;

				mr = dr + *begin;
				if (mr > 0x7fff)
					mr = 0x7fff;
				else if (mr < -0x8000)
					mr = -0x8000;
				*begin++ = mr;
			}
		}
		write(audiofd, buf, BUF_LEN);
	}
}


void I_SetSfxVolume(int volume)
{
	USED(volume);
}

// Gets lump nums of the named sound.  Returns pointer which will be
// passed to I_StartSound() when you want to start an SFX.  Must be
// sure to pass this to UngetSoundEffect() so that they can be
// freed!

int I_GetSfxLumpNum(sfxinfo_t *sound)
{
	return W_GetNumForName(sound->lumpname);
}


// Id is unused.
// Data is a pointer to a Sample structure.
// Volume ranges from 0 to 127.
// Separation (orientation/stereo) ranges from 0 to 255.  128 is balanced.
// Pitch ranges from 0 to 255.  Normal is 128.
// Priority looks to be unused (always 0).

int I_StartSound(int id, void *data, int vol, int sep, int pitch, int priority)
{
	// Relative time order to find oldest sound.
	static unsigned int soundTime = 0;
	int chanId;
	Sample *sample;
	Channel *chan;
	int oldest;
	int i;

	USED(id);
	// Find an empty channel, the oldest playing channel, or default to 0.
	// Currently ignoring priority.

	chanId = 0;
	oldest = soundTime;
	for (i = 0; i < CHAN_COUNT; i++)
	{
		if (! channel[ i ].begin)
		{
			chanId = i;
			break;
		}
		if (channel[ i ].time < oldest)
		{
			chanId = i;
			oldest = channel[ i ].time;
		}
	}

	sample = (Sample *) data;
	chan = &channel[chanId];

	I_UpdateSoundParams(chanId + 1, vol, sep, pitch);

	// begin must be set last because the audio thread will access the channel
	// once it is non-zero.  Perhaps this should be protected by a mutex.
	chan->pri = priority;
	chan->time = soundTime;
	chan->end = &sample->firstSample + LONG(sample->length);
	chan->begin = &sample->firstSample;

	soundTime++;

#if 0
	printf ("I_StartSound %d: v:%d s:%d p:%d pri:%d | %d %d %d %d\n",
		id, vol, sep, pitch, priority,
		chanId, chan->pitch_step, SHORT(sample->a), SHORT(sample->freq));
#endif

	return chanId + 1;
}

void I_StopSound(int handle)
{
	handle--;
	handle &= 7;
	channel[handle].begin = NULL;
}

int I_SoundIsPlaying(int handle)
{
	handle--;
	handle &= 7;
	return (channel[ handle ].begin != NULL);
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	int lvol, rvol;
	Channel *chan;

	qlock(&audiolk);
	// Set left/right channel volume based on seperation.
	sep += 1;	// range 1 - 256
	lvol = vol - ((vol * sep * sep) >> 16);	// (256*256);
	sep = sep - 257;
	rvol = vol - ((vol * sep * sep) >> 16);

	// Sanity check, clamp volume.
	if (rvol < 0)
	{
	//	printf ("rvol out of bounds %d, id %d\n", rvol, handle);
		rvol = 0;
	}
	else if (rvol > 127)
	{
	//	printf ("rvol out of bounds %d, id %d\n", rvol, handle);
		rvol = 127;
	}

	if (lvol < 0)
	{
	//	printf ("lvol out of bounds %d, id %d\n", lvol, handle);
		lvol = 0;
	}
	else if (lvol > 127)
	{
	//	printf ("lvol out of bounds %d, id %d\n", lvol, handle);
		lvol = 127;
	}

	// Limit to MAX_VOL (64)
	lvol >>= 1;
	rvol >>= 1;

	handle--;
	handle &= 7;
	chan = &channel[handle];
	chan->pitch_step = steptable[pitch];
	chan->step_remainder = 0;
	chan->lvol_table = &vol_lookup[lvol * 256];
	chan->rvol_table = &vol_lookup[rvol * 256];
	qunlock(&audiolk);
}


/*
 *	SOUND STARTUP STUFF
 */

// inits all sound stuff
void I_StartupSound (void)
{
	snd_SfxAvail = false;

	if (M_CheckParm("--nosound") || M_CheckParm("-s") || M_CheckParm("-nosound"))
	{
		ST_Message("I_StartupSound: Sound Disabled.\n");
		return;
	}

	audiofd = open("/dev/audio", OWRITE);
	if(audiofd < 0){
		ST_Message("I_StartupSound: /dev/audio could not be opened\n");
		return;
	}

	snd_SfxAvail = true;

	if(rfork(RFPROC|RFMEM) == 0){
		audioproc();
		exits(nil);
	}
}

// shuts down all sound stuff
void I_ShutdownSound (void)
{
	snd_SfxAvail = false;

	I_ShutdownMusic();
}

void I_SetChannels(int channels)
{
	int v, j;
	int *steptablemid;

	// We always have CHAN_COUNT channels.
	USED(channels);

	for (j = 0; j < CHAN_COUNT; j++)
	{
		channel[j].begin = NULL;
		channel[j].end   = NULL;
		channel[j].time = 0;
	}

	// This table provides step widths for pitch parameters.
	steptablemid = steptable + 128;
	for (j = -128; j < 128; j++)
	{
		steptablemid[j] = (int) (pow(2.0, (j/64.0)) * 65536.0);
	}

	// Generate the volume lookup tables.
	for (v = 0; v < MAX_VOL; v++)
	{
		for (j = 0; j < 256; j++)
		{
		//	vol_lookup[v*256+j] = 128 + ((v * (j-128)) / (MAX_VOL-1));

		// Turn the unsigned samples into signed samples.

			vol_lookup[v*256+j] = (v * (j-128) * 256) / (MAX_VOL-1);
		//	printf ("vol_lookup[%d*256+%d] = %d\n", v, j, vol_lookup[v*256+j]);
		}
	}
}


/*
 *	SONG API
 */

static int didgen = 0;

static void genmidi(void)
{
	int fd, n, sz;
	char name[64];
	uchar *gm;

	n = W_GetNumForName("GENMIDI");
	sz = W_LumpLength(n);
	gm = (uchar *)W_CacheLumpNum(n, PU_STATIC);
	snprint(name, sizeof(name), "/tmp/genmidi.%d", getpid());
	if((fd = create(name, ORDWR|ORCLOSE, 0666)) < 0)
		sysfatal("create: %r");
	if(write(fd, gm, sz) != sz)
			sysfatal("write: %r");
	Z_Free(gm);
}

void I_ShutdownMusic(void)
{
	if(mpfd[0] >= 0){
		close(mpfd[0]);
		mpfd[0] = -1;
		waitpid();
	}
}


/* In theory this allows register step allows 
 * the use of external files in place of internal ones. */
static void *currentsong = nil;
static int currentsize = 0;

int I_RegisterSong(void *data, int siz)
{
	if(!didgen){
		genmidi();
		didgen++;
	}
	if(currentsong != nil)
		return 0;

	currentsong = data;
	currentsize = siz;
	return 1;
}

int I_RegisterExternalSong(const char *nm)
{
	USED(nm);
	return 0;
}

void I_UnRegisterSong(int handle)
{
	USED(handle);
	currentsong = nil;
}

void I_PauseSong(int handle)
{
	if(handle <= 0)
		return;
	mus_paused = true;
}

void I_ResumeSong(int handle)
{
	if(handle <= 0)
		return;
	mus_paused = false;
}

void I_SetMusicVolume(int volume)
{
	USED(volume);
}

int I_QrySongPlaying(int handle)
{
	USED(handle);
	return 0;
}

// Stops a song.  MUST be called before I_UnregisterSong().
void I_StopSong(int handle)
{
	if(handle <= 0)
		return;
	I_ShutdownMusic();
}

void I_PlaySong(int handle, boolean loop)
{
	char name[64];
	int n;

	if(M_CheckParm("-nomusic") || audiofd < 0 || handle <= 0)
		return;

	I_ShutdownMusic();
	if(pipe(mpfd) < 0)
		return;
	switch(rfork(RFPROC|RFFDG|RFNAMEG)){
	case -1:
		fprint(2, "I_PlaySong: %r\n");
		break;
	case 0:
		dup(mpfd[1], 1);
		for(n=3; n<20; n++) close(n);
		close(0);
		snprint(name, sizeof(name), "/tmp/hexen.%d", getpid());
		if(create(name, ORDWR|ORCLOSE, 0666) != 0)
			sysfatal("create: %r");
		if(write(0, currentsong, currentsize) != currentsize)
			sysfatal("write: %r");
		if(seek(0, 0, 0) != 0)
			sysfatal("seek: %r");
		if(bind("/fd/1", "/dev/audio", MREPL) == -1)
			sysfatal("bind: %r");
		while(loop && fork() > 0){
			if(waitpid() < 0 || write(1, "", 0) < 0)
				exits(nil);
		}
		execl("/bin/dmus", "dmus", name, nil);
		execl("/bin/play", "play", name, nil);
		sysfatal("execl: %r");
	default:
		close(mpfd[1]);
	}
}

