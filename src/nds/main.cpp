#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ds2io.h>
#include <ds2_cpu.h>

#include "burnint.h"
#include "font.h"
#include "nds.h"
#include "UniCache.h"

short skipFrame=0;
int nGameStage = 0;
int bGameRunning = 0;
char currentPath[MAX_PATH];
unsigned int debugValue[2]={0,};
unsigned int inputKeys[3][3]={{0,},};

static bool p2pFrameStatus=false;

void returnToMenu()
{
	ds2_setCPUclocklevel(10);
	setGameStage(1);
	sound_pause();
	draw_ui_main();
}


inline void *video_frame_addr(void *frame, int x, int y)
{
	return (void *)(((unsigned int)frame|0x44000000) + ((x + (y << 9)) << 1));
}

static unsigned int HighCol16(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (b << 8) & 0xF800;
	t |= (g << 3) & 0x07E0;
	t |= (r >> 3) & 0x001F;
	return t;
}

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
	getcwd(currentPath, MAX_PATH - 1);
	strcat(currentPath, "/");
	
	strcpy(szAppRomPath, "/FBA4DSTWO/roms");
	chech_and_mk_dir( szAppRomPath );
	strcat(szAppRomPath, "/");
	
	strcpy(szAppCachePath, currentPath);
	strcat(szAppCachePath, "CACHE");
	chech_and_mk_dir( szAppCachePath );
	strcat(szAppCachePath, "/");
		
	setGameStage (1);
	init_gui();

	BurnLibInit();
	nBurnDrvSelect = ~0U;
	bBurnUseASMCPUEmulation = false;
	
	sound_start();
	
	nBurnBpp = 2;
	nBurnPitch  = 512 * nBurnBpp;
	BurnHighCol = HighCol16;
	
	pBurnDraw = (unsigned char *) up_screen_addr;
	
	draw_ui_main();
	bGameRunning = 1;
	nCurrentFrame = 0;

#ifdef SHOW_FPS
	u64 ctk, ptk;
	int nframes = 0,nTicksCountInSec=0;
	char fps[32] = {0, };
	//sceRtcGetCurrentTick( &ctk );
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
			//TODO:
			//show_frame = draw_frame;
			//draw_frame = sceGuSwapBuffers();
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
			
			if(mixbufidDiff<3&&skipFrame<gameSpeedCtrl)
			{
				skipFrame++;
			}else
			{
				skipFrame=0;
				
				while(mixbufidDiff>6&&bGameRunning)
				{
					//sceKernelDelayThread(1000);
				}
				
				//TODO:
				//pBurnDraw = (unsigned char *) video_frame_addr(tex_frame, 0, 0);
			}
#ifdef SHOW_FPS			
			drawString(fps, (unsigned short*)((unsigned int)GU_FRAME_ADDR(tex_frame)|0x40000000), 11, 11, R8G8B8_to_B5G6R5(0x404040));
			drawString(fps, (unsigned short*)((unsigned int)GU_FRAME_ADDR(tex_frame)|0x40000000), 10, 10, R8G8B8_to_B5G6R5(0xffffff));
#endif			
			if(pBurnDraw)
			{
				//show_frame = draw_frame;
				//draw_frame = sceGuSwapBuffers();
				update_gui();
			}
			BurnDrvFrame();
			pBurnDraw = NULL;
			
						
					
			//sceDisplayWaitVblankStart();
			sound_next();
		}
		
	}

	ds2_setCPUclocklevel(10);
	
	sound_stop();
	exit_gui();
	
	DrvExit();
	BurnLibExit();
	InpExit();
	
	ds2_plug_exit();
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
