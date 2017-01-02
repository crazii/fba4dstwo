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
			short* src = mixbuf;
			for(int i = 0; i < SND_FRAME_SIZE; i+=16, src+=32,ds2_aubuff+=16,ds2_aubuff2+=16)
			{
				ds2_aubuff2[0] = ds2_aubuff[0] = src[0];
				ds2_aubuff2[1] = ds2_aubuff[1] = src[2];
				ds2_aubuff2[2] = ds2_aubuff[2] = src[4];
				ds2_aubuff2[3] = ds2_aubuff[3] = src[6];
				ds2_aubuff2[4] = ds2_aubuff[4] = src[8];
				ds2_aubuff2[5] = ds2_aubuff[5] = src[10];
				ds2_aubuff2[6] = ds2_aubuff[6] = src[12];
				ds2_aubuff2[7] = ds2_aubuff[7] = src[14];

				ds2_aubuff2[8] = ds2_aubuff[8] = src[16];
				ds2_aubuff2[9] = ds2_aubuff[9] = src[18];
				ds2_aubuff2[10] = ds2_aubuff[10] = src[20];
				ds2_aubuff2[11] = ds2_aubuff[11] = src[22];
				ds2_aubuff2[12] = ds2_aubuff[12] = src[24];
				ds2_aubuff2[13] = ds2_aubuff[13] = src[26];
				ds2_aubuff2[14] = ds2_aubuff[14] = src[28];
				ds2_aubuff2[15] = ds2_aubuff[15] = src[30];
			}
		}
		else
		{
			unsigned int* mixbuf2 = (unsigned int*)mixbuf;
			for(int i = 0; i < SND_FRAME_SIZE; i+=16, mixbuf2+=16,ds2_aubuff+=16,ds2_aubuff2+=16)
			{
				register unsigned int s;

				s = mixbuf2[0];
				ds2_aubuff[0] = s;
				ds2_aubuff2[0] = s >> 16;
				
				s = mixbuf2[1];
				ds2_aubuff[1] = s;
				ds2_aubuff2[1] = s >> 16;
				
				s = mixbuf2[2];
				ds2_aubuff[2] = s;
				ds2_aubuff2[2] = s >> 16;
				
				s = mixbuf2[3];
				ds2_aubuff[3] = s;
				ds2_aubuff2[3] = s >> 16;
				
				s = mixbuf2[4];
				ds2_aubuff[4] = s;
				ds2_aubuff2[4] = s >> 16;
				
				s = mixbuf2[5];
				ds2_aubuff[5] = s;
				ds2_aubuff2[5] = s >> 16;
				
				s = mixbuf2[6];
				ds2_aubuff[6] = s;
				ds2_aubuff2[6] = s >> 16;
				
				s = mixbuf2[7];
				ds2_aubuff[7] = s;
				ds2_aubuff2[7] = s >> 16;

				s = mixbuf2[8];
				ds2_aubuff[8] = s;
				ds2_aubuff2[8] = s >> 16;

				s = mixbuf2[9];
				ds2_aubuff[9] = s;
				ds2_aubuff2[9] = s >> 16;

				s = mixbuf2[10];
				ds2_aubuff[10] = s;
				ds2_aubuff2[10] = s >> 16;

				s = mixbuf2[11];
				ds2_aubuff[11] = s;
				ds2_aubuff2[11] = s >> 16;

				s = mixbuf2[12];
				ds2_aubuff[12] = s;
				ds2_aubuff2[12] = s >> 16;

				s = mixbuf2[13];
				ds2_aubuff[13] = s;
				ds2_aubuff2[13] = s >> 16;

				s = mixbuf2[14];
				ds2_aubuff[14] = s;
				ds2_aubuff2[14] = s >> 16;

				s = mixbuf2[15];
				ds2_aubuff[15] = s;
				ds2_aubuff2[15] = s >> 16;
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




