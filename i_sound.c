/* i_sound.c */

#include "i_system.h"
#include "i_sound.h"
//#include "w_wad.h"	// W_GetNumForName()
//#include "z_zone.h"
//#include "m_argv.h"

/* The number of internal mixing channels,
**  the samples calculated for each mixing step,
**  the size of the 16bit, 2 hardware channel (stereo)
**  mixing buffer, and the samplerate of the raw data.
*/

/* Needed for calling the actual sound output. */
#define	AUDFREQ		44100
#define	SFXFREQ		11025
#define	SAMPLECOUNT	(AUDFREQ/TICRATE)
#define	NUM_CHANNELS	8

/* The actual lengths of all sound effects. */
int	lengths[NUMSFX];

/* The actual output device. */
static int audio_fd = -1;

/* The global mixing buffer.
** Basically, samples from all active internal channels
**  are modified and added, and stored in the buffer
**  that is submitted to the audio device.
*/
uchar mixbuf[SAMPLECOUNT*4];

/* The channel step amount... */
uint	channelstep[NUM_CHANNELS];
/* ... and a 0.16 bit remainder of last step. */
uint	channelstepremainder[NUM_CHANNELS];

/* The channel data pointers, start and end. */
uchar*	channels[NUM_CHANNELS];
uchar*	channelsend[NUM_CHANNELS];

/* Time/gametic that the channel started playing,
**  used to determine oldest, which automatically
**  has lowest priority.
** In case number of active sounds exceeds
**  available channels.
*/
int	channelstart[NUM_CHANNELS];

/* The sound in channel handles,
**  determined on registration,
**  might be used to unregister/stop/modify,
**  currently unused.
*/
int	channelhandles[NUM_CHANNELS];

/* SFX id of the playing sound effect.
** Used to catch duplicates (like chainsaw).
*/
int	channelids[NUM_CHANNELS];

/* Pitch to stepping lookup, unused. */
int	steptable[256];

/* Volume lookups. */
int	vol_lookup[128*256];

/* Hardware left and right channel volume lookup. */
int*	channelleftvol_lookup[NUM_CHANNELS];
int*	channelrightvol_lookup[NUM_CHANNELS];

int snd_MaxVolume;
int UpdateState;
int snd_Channels;
int snd_MusicVolume;

void I_StartupSound(void)
{
}

void I_InitSound(void)
{
}

/* This function loops all active (internal) sound
**  channels, retrieves a given number of samples
**  from the raw sound data, modifies it according
**  to the current (internal) channel parameters,
**  mixes the per-channel samples into the global
**  mixbuffer, clamping it to the allowed range,
**  and sets up everything for transferring the
**  contents of the mixbuffer to the (two)
**  hardware channels (left and right, that is).
**
** This function currently supports only 16bit.
*/
void I_UpdateSound(void)
{
}

void I_ShutdownSound(void)
{
}

void I_SetChannels(int channels)
{
}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
	return 0;
}

int I_StartSound(int id, void *data, int vol, int sep, int pitch, int)
{
	return 0;
}

void I_StopSound(int handle)
{
	USED(handle);
//	printf("PORTME i_sound.c I_StopSound\n");
}

int I_SoundIsPlaying(int handle)
{
	return 1;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	/* I fail to see that this is used.
	** Would be using the handle to identify
	**  on which channel the sound might be active,
	**  and resetting the channel parameters.
	*/
	USED(handle, vol, sep, pitch);
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{
}

void I_SetMusicVolume(int)
{
}

void I_PauseSong(int)
{
}

void I_ResumeSong(int)
{
}

void I_PlaySong(int handle, int loop)
{
}

void I_StopSong(int)
{
	I_ShutdownMusic();
}

int I_RegisterSong(void *data)
{
	return 0;
}

void I_UnRegisterSong(int handle)
{
}

int I_RegisterExternalSong(char *name)
{
	return 0;
}
