#include <stdarg.h>

#include "burnint.h"
#include "nds.h"
#include "font.h"

//note: font(gui) directly write to screen buffer (DOWN_SCREEN, down_screen_addr, SCREEN_WIDTH, SCREEN_HEIGHT)

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
	
	vsprintf(buf, pszFormat, vaFormat);
	
	unsigned short fc;
	switch (nStatus) {
	case PRINT_UI:			fc = R8G8B8_to_B5G6R5(0x404040); break;
	case PRINT_IMPORTANT:	fc = R8G8B8_to_B5G6R5(0x600000); break;
	case PRINT_ERROR:		fc = R8G8B8_to_B5G6R5(0x606000); break;
	default:				fc = R8G8B8_to_B5G6R5(0x404040); break;
	}
	
	drawRect((unsigned short*)down_screen_addr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	drawRect((unsigned short*)down_screen_addr, 0, 0, SCREEN_WIDTH, 20, fc);
	drawString(buf, (unsigned short*)down_screen_addr, 4, 4, R8G8B8_to_B5G6R5(0xffffff));

	drawRect((unsigned short*)down_screen_addr, 0, 0, SCREEN_WIDTH, 20, fc);
	drawString(buf, (unsigned short*)down_screen_addr, 4, 4, R8G8B8_to_B5G6R5(0xffffff));
	
	va_end(vaFormat);
	
	ds2_flipScreen(DOWN_SCREEN, 2);
	
	struct key_buf pad;
	do {ds2_getrawInput(&pad);}while((pad.key&KEY_SELECT) == 0);
	do {ds2_getrawInput(&pad);}while((pad.key&KEY_SELECT) != 0);
	return 0;
}


void init_gui()
{
	ds2_clearScreen(DOWN_SCREEN, 2);

	bprintf = AppDebugPrintf;
	
	BurnExtProgressRangeCallback = myProgressRangeCallback;
	BurnExtProgressUpdateCallback = myProgressUpdateCallback;
}

void exit_gui()
{
	
}

void update_gui()
{
	ds2_flipScreen(DOWN_SCREEN, 2);
}

void clear_gui_texture(int color, int w, int h)
{
	//used in game. clear up screen
	ds2_clearScreen(UP_SCREEN, 0);
}
