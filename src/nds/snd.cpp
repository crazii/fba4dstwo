#include <ds2io.h>

#include "burnint.h"
#include "nds.h"
static short mixbuf[SND_FRAME_SIZE * 2 * 8];
static short * pmixbuf[] = {
	&mixbuf[SND_FRAME_SIZE * 2 * 0],
	&mixbuf[SND_FRAME_SIZE * 2 * 1],
	&mixbuf[SND_FRAME_SIZE * 2 * 2],
	&mixbuf[SND_FRAME_SIZE * 2 * 3],
	&mixbuf[SND_FRAME_SIZE * 2 * 4],
	&mixbuf[SND_FRAME_SIZE * 2 * 5],
	&mixbuf[SND_FRAME_SIZE * 2 * 6],
	&mixbuf[SND_FRAME_SIZE * 2 * 7]
};
static unsigned int mixbufid = 0;
static unsigned int mixbufidPlay = 0;
int mixbufidDiff = 0;
unsigned char monoSound=0;
static short sound_active = 0;

static void ds2_play_sound()
{
	short * currentMixBuf; 
	if(mixbufidDiff < 0 || ds2_checkAudiobuff() >= 4)
		return;
	
	mixbufidDiff = mixbufid-mixbufidPlay;
	if(mixbufidDiff>1)
	{
		mixbufidPlay++;
	}
	
	currentMixBuf=pmixbuf[mixbufidPlay & 0x7];
	if(monoSound)
	{
		for(int i=0;i<SND_FRAME_SIZE*2;i=i+2)
		{
			 currentMixBuf[i+1]=currentMixBuf[i];
		}
	}
	
	
	unsigned short* ds2_aubuff = (unsigned short*)ds2_getAudiobuff();
	if (ds2_aubuff)
	{
		if(monoSound)
		{
			for(int i = 0; i < SND_FRAME_SIZE; ++i)
				ds2_aubuff[i+SND_FRAME_SIZE] = ds2_aubuff[i] = currentMixBuf[i*2];
		}
		else
		{
			for(int i = 0; i < SND_FRAME_SIZE; ++i)
			{
				ds2_aubuff[i] = currentMixBuf[i*2];
				ds2_aubuff[i+SND_FRAME_SIZE] = currentMixBuf[i*2+1];
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
	
	mixbufid = 0;
	mixbufidDiff= 0;
	pBurnSoundOut = &mixbuf[0];
	
	sound_active = 1;
	return 0;
}

int sound_stop()
{
	sound_active = 0;
	return 0;
}

void sound_next()
{	
	mixbufid ++;
	pBurnSoundOut = pmixbuf[mixbufid & 0x7];
}

void sound_pause()
{
	mixbufidDiff= -1;
	memset(mixbuf, 0, SND_FRAME_SIZE * 2 * 8*2);
}

void sound_continue()
{
	mixbufidDiff= 0;
}




