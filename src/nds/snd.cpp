#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspaudio.h>

#include "burnint.h"
#include "psp.h"
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
static SceUID sound_thread_id;
static short sound_active = 0;

static int sound_thread(SceSize args, void *argp)
{
	short * currentMixBuf; 
	while (sound_active) {
		if(mixbufidDiff < 0)
		{
			
			sceKernelDelayThread(100000);

			continue;
		}
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
		sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, currentMixBuf);


	}
	sceKernelExitThread(0);
	return 0;
}

int sound_start()
{
	nInterpolation = 0;
	pBurnSoundOut = NULL;
	nBurnSoundRate = SND_RATE;
	nBurnSoundLen = SND_FRAME_SIZE;

	memset(mixbuf, 0, SND_FRAME_SIZE * 2 * 8*2);
	sceAudioSRCChRelease();
	
	int aures = sceAudioSRCChReserve( SND_FRAME_SIZE, SND_RATE, 2 );
	if ( aures ) return aures;
	
	sound_thread_id = sceKernelCreateThread("sound_thread", sound_thread, 0x0F, 0x400, 0, NULL);
	if (sound_thread_id < 0) {
		sceAudioSRCChRelease();
		return -1;
	}
	
	mixbufid = 0;
	mixbufidDiff= 0;
	pBurnSoundOut = &mixbuf[0];
	
	sound_active = 1;
	sceKernelStartThread(sound_thread_id, 0, 0);

	return 0;
}

int sound_stop()
{

	if (sound_thread_id >= 0) {
		sound_active = 0;
		sceKernelWaitThreadEnd(sound_thread_id, NULL);
		sceKernelDeleteThread(sound_thread_id);
		sound_thread_id = -1;
		sceAudioSRCChRelease();
	}

	return 0;
}

void sound_next()
{	
	mixbufid ++;
	pBurnSoundOut = pmixbuf[mixbufid & 0x7];
	//sceKernelDelayThread(1);

}

void sound_pause()
{
	mixbufidDiff= -1;
	sceAudioSRCChRelease();
	memset(mixbuf, 0, SND_FRAME_SIZE * 2 * 8*2);
}

void sound_continue()
{
	mixbufidDiff= 0;
	sceAudioSRCChReserve( SND_FRAME_SIZE, SND_RATE, 2 );
}




