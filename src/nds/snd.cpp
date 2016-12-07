#include <ds2io.h>

#include "burnint.h"
#include "nds.h"
static short mixbuf[SND_FRAME_SIZE * 2];
static unsigned int mixbufid = 0;
static unsigned int mixbufidPlay = 0;
int mixbufidDiff = 0;
static short sound_active = 0;
static short sound_paused = 0;
unsigned char monoSound=0;

static void ds2_play_sound()
{
	if(ds2_checkAudiobuff() >= 4 || !sound_active || sound_paused)
		return;	
	
	unsigned short* ds2_aubuff = (unsigned short*)ds2_getAudiobuff();
	if (ds2_aubuff)
	{
		if(monoSound)
		{
			for(int i = 0; i < SND_FRAME_SIZE; ++i)
				ds2_aubuff[i+SND_FRAME_SIZE] = ds2_aubuff[i] = mixbuf[i*2];
		}
		else
		{
			for(int i = 0; i < SND_FRAME_SIZE; ++i)
			{
				ds2_aubuff[i] = mixbuf[i*2];
				ds2_aubuff[i+SND_FRAME_SIZE] = mixbuf[i*2+1];
			}
		}
		//Update audio  
		ds2_updateAudio();
   }
}

int sound_start()
{
	nInterpolation = 0;
	pBurnSoundOut = NULL;
	nBurnSoundRate = SND_RATE;
	nBurnSoundLen = SND_FRAME_SIZE;

	memset(mixbuf, 0, SND_FRAME_SIZE * 2 * 8*2);
	pBurnSoundOut = &mixbuf[0];
	
	sound_active = 1;
	sound_paused = 0;
	return 0;
}

int sound_stop()
{
	sound_active = 0;
	return 0;
}

void sound_next()
{	
	ds2_play_sound();
}

void sound_pause()
{
	sound_paused = 1;
}

void sound_continue()
{
	sound_paused = 0;
}




