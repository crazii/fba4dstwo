#include <stdarg.h>

#include "burnint.h"
#include "nds.h"
#include "font.h"


void * show_frame = (void *)(SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 0);
void * draw_frame = (void *)(SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 1);
void * work_frame = (void *)(SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 2);
void * tex_frame  = (void *)(SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 3);

static unsigned char* list=(unsigned char*)((SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 4)|0x4000000);
unsigned char* bgBuf=(unsigned char*)((SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 5)|0x4000000);
unsigned char* previewBuf=(unsigned char*)((SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 6)|0x4000000);
unsigned char* tmpBuf=(unsigned char*)((SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 7)|0x4000000);

static int nPrevStage;

static int VideoBufferWidth, VideoBufferHeight;

static int myProgressRangeCallback(double dProgressRange)
{
	return 0;
}

static int myProgressUpdateCallback(double dProgress, const char* pszText, bool bAbs)
{
	bprintf(PRINT_IMPORTANT, "%s %f %d", pszText, dProgress, bAbs);
	return 0;
}

int AppDebugPrintf(int nStatus, char* pszFormat, ...)
{
	va_list vaFormat;
	va_start(vaFormat, pszFormat);
	char buf[256];
	
	sprintf(buf, pszFormat, vaFormat);
	
	unsigned short fc;
	switch (nStatus) {
	case PRINT_UI:			fc = R8G8B8_to_B5G6R5(0x404040); break;
	case PRINT_IMPORTANT:	fc = R8G8B8_to_B5G6R5(0x600000); break;
	case PRINT_ERROR:		fc = R8G8B8_to_B5G6R5(0x606000); break;
	default:				fc = R8G8B8_to_B5G6R5(0x404040); break;
	}
	
	drawRect((unsigned short*)up_screen_addr, 0, 0, 480, 20, fc);
	drawString(buf, (unsigned short*)up_screen_addr, 4, 4, R8G8B8_to_B5G6R5(0xffffff));

	drawRect((unsigned short*)up_screen_addr, 0, 0, 480, 20, fc);
	drawString(buf, (unsigned short*)up_screen_addr, 4, 4, R8G8B8_to_B5G6R5(0xffffff));
	
	va_end(vaFormat);
	
	update_gui();
	
	return 0;
}


void init_gui()
{
	ds2_clearScreen(UP_SCREEN, 0);
	
	nPrevStage = nGameStage;
	//bprintf = AppDebugPrintf;
	
	
	BurnExtProgressRangeCallback = myProgressRangeCallback;
	BurnExtProgressUpdateCallback = myProgressUpdateCallback;
}

void exit_gui()
{
	
}

void update_gui()
{
	if ( nPrevStage != nGameStage ) {
		if ( nGameStage ) {
			//sceGuTexImage(0, 512, 512, 512, GU_FRAME_ADDR(work_frame));
			//sceGuTexFilter(GU_NEAREST, GU_NEAREST);
		} else {
			BurnDrvGetFullSize(&VideoBufferWidth, &VideoBufferHeight);
			//sceGuTexImage(0, 512, 512, 512, GU_FRAME_ADDR(tex_frame));
			//sceGuTexFilter(GU_LINEAR, GU_LINEAR);
		}
		nPrevStage = nGameStage;
	}
	//TODO:
}

void clear_gui_texture(int color, int w, int h)
{
	ds2_clearScreen(UP_SCREEN, 0);
}
