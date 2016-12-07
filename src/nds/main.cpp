#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "burnint.h"
#include "font.h"
#include "nds.h"
#include "UniCache.h"

#define VIDEO_BUFFER_WIDTH 512
#define VIDEO_BUFFER_HEIGHT 512

short skipFrame=0;
int nGameStage = 0;
int bGameRunning = 0;
unsigned int debugValue[2]={0,};

static unsigned short* videoBuffer = NULL;
static bool p2pFrameStatus=false;

void returnToMenu()
{
	ds2_setCPUclocklevel(10);
	setGameStage(1);
	sound_pause();
	draw_ui_main();
}

static unsigned int HighCol16(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (b << 8) & 0x7C00;
	t |= (g << 3) & 0x03E0;
	t |= (r >> 3) & 0x001F;
	return t;
}

static void swapBuffer();

char szAppCachePath[256];

void chech_and_mk_dir(const char * dir)
{
	DIR* d = opendir(dir);
	if (d)
		closedir(d);
	else
		mkdir(dir, 0777);
}

int main(int argc, char** argv) {
	
	unsigned int autoFireButtons=0;

	loadDefaultInput();
	
	strcpy(szAppRomPath, "/FBA4DSTWO/roms");
	chech_and_mk_dir( szAppRomPath );
	strcat(szAppRomPath, "/");
	
	strcpy(szAppCachePath, "/FBA4DSTWO/CACHE");
	chech_and_mk_dir( szAppCachePath );
	strcat(szAppCachePath, "/");

	setGameStage (1);
	init_gui();

	BurnLibInit();
	nBurnDrvSelect = ~0U;
	bBurnUseASMCPUEmulation = false;
	
	sound_start();
	
	nBurnBpp = sizeof(unsigned short);
	nBurnPitch  = VIDEO_BUFFER_WIDTH * nBurnBpp;
	BurnHighCol = HighCol16;
	
	videoBuffer = (unsigned short*)malloc(VIDEO_BUFFER_WIDTH*VIDEO_BUFFER_HEIGHT*nBurnBpp);
	if(!videoBuffer)
		return -1;
	
	pBurnDraw = (unsigned char *)videoBuffer;
	
	draw_ui_main();
	bGameRunning = 1;
	nCurrentFrame = 0;

#ifdef SHOW_FPS
	u64 ctk, ptk;
	int nframes = 0,nTicksCountInSec=0;
	char fps[32] = {0, };
	ptk = ctk;
#endif

	int ndsinput = 0;
	while( bGameRunning ) {
GAME_RUNNING:
		
#ifdef SHOW_FPS
		//sceRtcGetCurrentTick( &ctk );
		nTicksCountInSec=ctk - ptk;
		if ( nTicksCountInSec>= 1000000 ) {
			ptk += 1000000;
			sprintf( fps, "%2d FPS, debugValue:0x%X,0x%X",  nframes,debugValue[0],debugValue[1]);
			nframes = 0;
			nTicksCountInSec=0;
		}
		nframes ++;
#endif

		struct key_buf pad;
		ds2_getrawInput(&pad);

		if ( nGameStage ) {
			
			do_ui_key( pad.key );
			update_gui();
			//swapBuffer();

		} else {
			
			if ( (pad.key & (KEY_L|KEY_R|KEY_START)) == (KEY_L|KEY_R|KEY_START) ) 
			{
				returnToMenu();								
				continue;
			}		
						
			if((nCurrentFrame&0x3)<2)
			{
				autoFireButtons=autoFireButtons&pad.key;
				if(pad.key&KEY_SELECT)
				{
					autoFireButtons=autoFireButtons|KEY_SELECT;
				}
				if ( pad.key & KEY_R ) 
				{
					{
						autoFireButtons=autoFireButtons|(pad.key&(~KEY_R));
					}
				}
			}else
				pad.key=pad.key&(~autoFireButtons);
			
			nCurrentFrame++;
			InpMake(pad.key);
			
			//TODO:
			const bool skip = false;
			#if 0
			if(mixbufidDiff<3 && skipFrame<gameSpeedCtrl && skip)
			{
				skipFrame++;
			}else
			#endif
			{
				skipFrame=0;
				pBurnDraw = (unsigned char *)videoBuffer;
			}
#ifdef SHOW_FPS			
			drawString(fps, (unsigned short*)((unsigned int)up_screen_addr, 11, 11, R8G8B8_to_B5G6R5(0x404040));
			drawString(fps, (unsigned short*)((unsigned int)up_screen_addr, 10, 10, R8G8B8_to_B5G6R5(0xffffff));
#endif
			BurnDrvFrame();
			
			if(pBurnDraw)
			{
				swapBuffer();
				//update_gui();
			}
			//pBurnDraw = NULL;
			sound_next();
		}
		
	}

	ds2_setCPUclocklevel(10);
	
	sound_stop();
	exit_gui();
	
	DrvExit();
	BurnLibExit();
	InpExit();
	
	free(videoBuffer);
}

void resetGame()
{
	setGameStage(0);
	sound_continue();
	nCurrentFrame=0;
	InpMake(0x80000000);
	nCurrentFrame++;
	InpMake(0x80000000);
	p2pFrameStatus=false;
}

#define _r(c) ((c)&0x1F)
#define _g(c) ((c)&0x03E0)
#define _b(c) ((c)&0x7C00)
#define _rgb(r,g,b) ((r) | ((g)&0x03E0) | ((b)&0x7C00) | (1<<15))
#define _mix(c1, c2) _rgb( (_r(c1) + _r(c2))>>1, (_g(c1) + _g(c2))>>1, (_b(c1) + _b(c2))>>1 )

void swapBuffer()
{
	unsigned short* src = videoBuffer;
	unsigned short* dst = (unsigned short*)up_screen_addr;
	int accumulatorY = 0;

	for (int h = 0; h < drvHeight; h++)
	{
		accumulatorY += iAdd;
		if (accumulatorY >= iModulo || h == drvHeight - 1)
		{
			accumulatorY -= iModulo;
			
			unsigned short pixel = 0;
			unsigned short *dstX=dst;
			int accumulatorX = 0;

			for (int w = 0; w < drvWidth; ++w)
			{
				if (accumulatorX >= iAdd)
					pixel = _mix(pixel, src[w]);
				else
					pixel = src[w];

				accumulatorX += iAdd;
				if (accumulatorX >= iModulo || w == drvWidth - 1)
				{
					accumulatorX -= iModulo;
					*dstX++ = pixel;
				}
			}
			dst += SCREEN_WIDTH;
		}
		src += VIDEO_BUFFER_WIDTH;
	}
  
	ds2_flipScreen(UP_SCREEN, 0);
}
