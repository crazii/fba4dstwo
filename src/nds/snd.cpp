#include <ds2io.h>

#include "burnint.h"
#include "nds.h"
static short mixbuf[SND_FRAME_SIZE * 2];
static short sound_active = 0;
static short sound_paused = 0;
signed char soundMode=2;

static void ds2_play_sound()
{
	if(ds2_checkAudiobuff() >= 4 || soundMode == 0)
		return;	
	
	unsigned short* ds2_aubuff = (unsigned short*)ds2_getAudiobuff();
	unsigned short* ds2_aubuff2 = ds2_aubuff + SND_FRAME_SIZE;
	if (ds2_aubuff)
	{
		if(soundMode == 1)
		{
			for(int i = 0; i < SND_FRAME_SIZE; ++i)
			{
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1]; ++i;
				ds2_aubuff2[i] = ds2_aubuff[i] = mixbuf[i<<1];
			}
		}
		else
		{
			unsigned int* mixbuf2 = (unsigned int*)mixbuf;
			for(int i = 0; i < SND_FRAME_SIZE; ++i)
			{
				register unsigned int s;

				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
				++i;
				
				s = mixbuf2[i];
				ds2_aubuff[i] = s;
				ds2_aubuff2[i] = s >> 16;
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

	memset(mixbuf, 0, SND_FRAME_SIZE * 2 * sizeof(short));
	if(soundMode != 0)
		pBurnSoundOut = &mixbuf[0];
	
	sound_active = 1;
	sound_paused = 0;
	return 0;
}

int sound_stop()
{
	sound_active = 0;
	sound_paused = 0;
	return 0;
}

void sound_next()
{
	if(sound_active && !sound_paused)
		ds2_play_sound();
	
	if(soundMode != 0)
		pBurnSoundOut = &mixbuf[0];
	else
		pBurnSoundOut = NULL;
}

void sound_pause()
{
	sound_paused = 1;
}

void sound_continue()
{
	sound_paused = 0;
}




