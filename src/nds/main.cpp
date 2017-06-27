#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "burnint.h"
#include "nds.h"

#define VIDEO_BUFFER_WIDTH 512
#define VIDEO_BUFFER_HEIGHT 512

int nGameStage = 0;
int bGameRunning = 0;
int graphicsSetting = 0;
char szAppCachePath[32] = {"/FBA4DSTWO/CACHE"};

unsigned int debugValue[2]={0,};

static unsigned short* videoBuffer = NULL;
static bool p2pFrameStatus=false;

static void swapBuffer();
void clear_gui_texture(int color, short w, short h);
extern "C" void swapBufferHQ(void* vbuff);
extern "C" void swapBufferLQ(void* vbuff);


void returnToMenu()
{
	//mdelay(100); // needed to avoid ds2_setBacklight crashing
	//ds2_setBacklight(2);
	clear_gui_texture(0, VIDEO_BUFFER_WIDTH, VIDEO_BUFFER_HEIGHT);
	swapBuffer();
	
	ds2_setCPUclocklevel(10);
	setGameStage(1);
	sound_pause();
	draw_ui_main();
	ui_update_progress(0, "Press B to return to game");
}

static unsigned int HighCol16(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	//5551
	t  = (b << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (r >> 3) & 0x001F;
	return t;
}

static void chech_and_mk_dir(const char * dir)
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
	
	chech_and_mk_dir( szAppRomPath );
	strcat(szAppRomPath, "/");
	
	chech_and_mk_dir( szAppCachePath );
	strcat(szAppCachePath, "/");

	setGameStage (1);
	init_gui();

	BurnLibInit();
	nBurnDrvSelect = ~0U;
	bBurnUseASMCPUEmulation = false;
	
	sound_start();
	
	//BurnSetRefreshRate(25);
	nBurnBpp = 2;
	nBurnPitch = VIDEO_BUFFER_WIDTH * nBurnBpp;
	BurnHighCol = HighCol16;
	
	ds2_clearScreen(UP_SCREEN, 0);
	ds2_flipScreen(UP_SCREEN, 1);
	ds2_clearScreen(UP_SCREEN, 0);
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

	struct key_buf lastpad = {0, 0, 0};
	struct key_buf pad = {0, 0, 0};
	int frameskip = 0;
	while( bGameRunning ) {
		
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

		lastpad = pad;
		ds2_getrawInput(&pad);

		if ( nGameStage ) {
			
			do_ui_key( pad.key );
			update_gui();

		} else {
			
			if ( (pad.key & (KEY_SELECT|KEY_START)) == (KEY_SELECT|KEY_START) ) 
			{
				mdelay(100); // needed to avoid ds2_setBacklight crashing
				ds2_setBacklight(3);
				returnToMenu();
				continue;
			}
			
			if ( (pad.key & (KEY_L|KEY_START)) != (KEY_L|KEY_START) 
				&& (lastpad.key & (KEY_L|KEY_START)) == (KEY_L|KEY_START) )
			{
				ui_dec_frame_skip();
				update_gui();
				continue;
			}
			
			if ( (pad.key & (KEY_R|KEY_START)) != (KEY_R|KEY_START) 
				&& (lastpad.key & (KEY_R|KEY_START)) == (KEY_R|KEY_START) )
			{
				ui_inc_frame_skip();
				update_gui();
				continue;
			}
			//bprintf(3, "frame start");
			
			if((nCurrentFrame&0x3)<2)
			{
				autoFireButtons=autoFireButtons&pad.key;
				if(pad.key&KEY_SELECT)
					autoFireButtons=autoFireButtons|KEY_SELECT;
				if ( pad.key & KEY_R ) 
					autoFireButtons=autoFireButtons|(pad.key&(~KEY_R));
			}
			else
				pad.key=pad.key&(~autoFireButtons);
			
			nCurrentFrame++;
			InpMake(pad.key);
			
			int currentSkip = (nCurrentFrame*gameSpeedCtrl) % FRAME_RATE;
			if( currentSkip < frameskip)
			{
				//bprintf(3, "frame skip");
				*((unsigned char* volatile*)&pBurnDraw) = NULL;
				BurnDrvFrame();
				*((unsigned char* volatile*)&pBurnDraw) = (unsigned char*)videoBuffer;
			}
			else
			{
				//bprintf(3, "frame");
				BurnDrvFrame();
#ifdef SHOW_FPS
				drawString(fps, (unsigned short*)((unsigned int)up_screen_addr, 11, 11, R8G8B8_to_B5G5R5(0x404040));
				drawString(fps, (unsigned short*)((unsigned int)up_screen_addr, 10, 10, R8G8B8_to_B5G5R5(0xffffff));
#endif	
				//bprintf(3, "pre swp");
				swapBuffer();
				//bprintf(3, "post swp");
			}
			frameskip = currentSkip;
			//bprintf(3, "sound");
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
#if 1
	if(graphicsSetting)
	{
		#if 1
		swapBufferHQ(videoBuffer);
		#else
		unsigned short* src = videoBuffer;
		__builtin_prefetch(src, 0);
		unsigned short* dst = (unsigned short*)up_screen_addr + xOff + (yOff * SCREEN_WIDTH);
		__builtin_prefetch(dst, 1);
		const register short scaleMod = iModulo;
		const register short scaleMul = iAdd;
		const register short height = drvHeight;
		const register short width = drvWidth;
		short accumulatorY = 0;
		bool mix = 0;

		for (short h = 0; h < height; h++)
		{
			short accumulatorX = 0;
			unsigned short pixel = 0;
			unsigned short* dstX = dst;

			for (short w = 0; w < width; ++w)
			{
				if (accumulatorX >= scaleMul)
				{
					unsigned short c = src[w];
					pixel = _mix(pixel, c);
				}
				else
					pixel = src[w];

				accumulatorX += scaleMul;
				if (accumulatorX >= scaleMod || w == width-1)
				{
					accumulatorX -= scaleMod;
					if (mix)
					{
						unsigned short c = *dstX;
						pixel = _mix(c, pixel);
					}
					*dstX++ = pixel;
				}
			}

			accumulatorY += scaleMul;
			if (accumulatorY >= scaleMod)
			{
				accumulatorY -= scaleMod;
				dst += SCREEN_WIDTH;
				__builtin_prefetch(dst, 1);
				mix = false;
			}
			else
				mix = true;

			src += VIDEO_BUFFER_WIDTH;
			__builtin_prefetch(src, 0);
		}
		#endif
	}
	else
	{
		#if 1 
		swapBufferLQ(videoBuffer);
		#else
		unsigned short* src = videoBuffer;
		__builtin_prefetch(src, 0);
		unsigned short* dst = (unsigned short*)up_screen_addr + xOff + (yOff * SCREEN_WIDTH);
		__builtin_prefetch(dst, 1);
		const register short scaleMod = iModulo;
		const register short scaleMul = iAdd;
		const register short height = drvHeight;
		const register short width = drvWidth;
		short accumulatorY = 0;

		for (short h = 0; h < height; h++)
		{
			accumulatorY += scaleMul;
			if (accumulatorY >= scaleMod || h == height - 1)
			{
				accumulatorY -= scaleMod;

				unsigned short pixel;
				register unsigned short *dstX=dst;
				register short accumulatorX = 0;
				for (short w = 0; w < width; ++w)
				{
					if (accumulatorX >= scaleMul)
					{
						unsigned short c = src[w];
						pixel = _mix(pixel, c);
					}
					else
						pixel = src[w];

					accumulatorX += scaleMul;
					if (accumulatorX >= scaleMod || w == width-1)
					{
						accumulatorX -= scaleMod;
						*dstX++ = pixel;
					}
				}
				dst += SCREEN_WIDTH;
				__builtin_prefetch(dst, 1);
			}

			src += VIDEO_BUFFER_WIDTH;
			__builtin_prefetch(src, 0);
		}
		#endif
	}
#else
	int w = drvWidth < SCREEN_WIDTH ? drvWidth : SCREEN_WIDTH;
	int h = drvHeight < SCREEN_HEIGHT ? drvHeight : SCREEN_HEIGHT;
	for(int i = 0; i < h; ++i)
		memcpy((short*)up_screen_addr + i*SCREEN_WIDTH, videoBuffer + i*VIDEO_BUFFER_WIDTH, w*2);
#endif

	ds2_flipScreen(UP_SCREEN, 0);
}

void clear_gui_texture(int color, short w, short h)
{
	//used in game. clear video buffer

	color |= 1<<15;
	//2 pixels
	color = (color&0xFFFF)|(color<<16);
	w /= 2;
	short mod8 = w&0x7;
	w = w >> 3;
	
	int* src = (int*)videoBuffer;
	for(short i = 0; i < h; ++i)
	{
		for(short j = 0; j < w; ++j, src+=8)
		{
			src[0] = color;
			src[1] = color;
			src[2] = color;
			src[3] = color;
			src[4] = color;
			src[5] = color;
			src[6] = color;
			src[7] = color;
		}

		for(short j = 0; j < mod8; ++j)
			src[j] = color;

		src += VIDEO_BUFFER_WIDTH/2 - (w << 3);
		__builtin_prefetch(src, 1, 0);
	}
}

