#include <pspkernel.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <psppower.h>

#include <stdio.h>
#include <string.h>
#include <pspdisplay.h>
#include <png.h>

#include "psp.h"
#include "font.h"
#include "burnint.h"
#include "UniCache.h"
#include "burner.h"
#include "pspadhoc.h"


#define find_rom_list_cnt	10

short gameSpeedCtrl = 1;
unsigned int hotButtons = (PSP_CTRL_SQUARE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS);

short screenMode=0;
short wifiStatus=0;
short saveIndex=0;
short gameScreenWidth=SCREEN_WIDTH, gameScreenHeight=SCREEN_HEIGHT;
bool enableJoyStick=true;
char LBVer[]="FinalBurn Alpha for PSP "SUB_VERSION" (ver: LB_V12.5.4)";
static int find_rom_count = 0;
static int find_rom_select = 0;
static int find_rom_top = 0;
char ui_current_path[MAX_PATH];

static unsigned int nPrevGame = ~0U;

static int ui_mainmenu_select = 0;

static int ui_process_pos = 0;
int InpMake(unsigned int);
int DoInputBlank(int bDipSwitch);
static unsigned int bgW=0,bgH=0,bgIndex=0;
static bool needPreview=true;

static void screenshot(const char* filename)
{
        u16* vram16;
       	int VideoBufferWidth,VideoBufferHeight;
      	
        int i, x, y;
        png_structp png_ptr;
        png_infop info_ptr;
        FILE* fp;
        u8* line;
        
        fp = fopen(filename, "wb");
        if (!fp) return;
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) return;
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
                png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
                fclose(fp);
                return;
        }
        png_init_io(png_ptr, fp);
        BurnDrvGetFullSize(&VideoBufferWidth, &VideoBufferHeight);
        png_set_IHDR(png_ptr, info_ptr, VideoBufferWidth, VideoBufferHeight,
                8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        line = (u8*)tmpBuf;         
        vram16 = GU_FRAME_ADDR(tex_frame);
        for (y = 0; y < VideoBufferHeight; y++) {
                for (i = 0, x = 0; x < VideoBufferWidth; x++) {
                        u32 color = 0;
                        u8 r = 0, g = 0, b = 0;
                                         color = vram16[x + y * PSP_LINE_SIZE];
                                        r = (color & 0x1f) << 3; 
                                        g = ((color >> 5) & 0x3f) << 2 ;
                                        b = ((color >> 11) & 0x1f) << 3 ;
   		                line[i++] = r;
                        line[i++] = g;
                        line[i++] = b;
                }
                png_write_row(png_ptr, line);
        }
        //free(line);
        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fclose(fp);
}

static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
        // ignore PNG warnings
}

static void loadImage(const unsigned char* imgBuf, const char* filename, unsigned int* previewWidth, unsigned int* previewHeight)
{

        u16* vram16;


        png_structp png_ptr;
        png_infop info_ptr;
        unsigned int sig_read = 0;

        int bit_depth, color_type, interlace_type, x, y;
        u32* line;
        FILE *fp;

        if ((fp = fopen(filename, "rb")) == NULL) return;
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
                fclose(fp);
                return;
        }
        png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
                fclose(fp);
                png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
                return;
        }
        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, sig_read);
        png_read_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)previewWidth, (png_uint_32*)previewHeight, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
        png_set_strip_16(png_ptr);
        png_set_packing(png_ptr);
        if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        line = (u32*)tmpBuf; 
        if (!line) {
                fclose(fp);
                png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
                return;
        }
        vram16 = (u16*)imgBuf;
        for (y = 0; y < *previewHeight; y++) {
                png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
                for (x = 0; x < *previewWidth; x++) {
                        u32 color32 = line[x];
                        u16 color16;
                        int r = color32 & 0xff;
                        int g = (color32 >> 8) & 0xff;
                        int b = (color32 >> 16) & 0xff;
                                        color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
                                        vram16[x + y * PSP_LINE_SIZE] = color16;
                  }
        }

        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
}
void processScreenMode()
{
	switch(screenMode)
	{
		case 3:
		case 7:
			gameScreenWidth=204;			
			break;
		case 1:
		case 5:
			gameScreenWidth=362;			
			break;		
		case 2:
		case 4:
		case 6:
		default:
			gameScreenWidth=SCREEN_WIDTH;			
			break;
	}
}

int DrvInitCallback()
{
	return DrvInit(nBurnDrvSelect,false);
}



static struct {
	int cpu, bus; 
} cpu_speeds[] = { { 222, 111}, { 266, 133}, { 300, 150}, { 333, 166} };

static int cpu_speeds_select = 3;

enum uiMainIndex
{
	SELECT_ROM=0,
	LOAD_GAME,
	SAVE_GAME,
	RESET_GAME,
	SCREEN_SHOT,
	CONTROLLER,
	SKIP_FRAMES,
	SCREEN_MODE,
	GAME_SCREEN_WIDTH,
	GAME_SCREEN_HEIGHT,
	CPU_SPEED,
	JOYSTICK,
	WIFI_GAME,
	SYNC_GAME,
	PREVIEW,
	MONO_SOUND,
	EXIT_FBA,
	MENU_COUNT
};
static char *ui_main_menu[] = {
	"Select ROM ",
	"%1u Load Game ",
	"%1u Save Game ",
	"Reset Game ",
	"Screen Shot ",
	"Controller: %1uP ",
	"Max Skip Frames: %d",
	"Screen Mode: %u ",
	"Screen Width: %3u ",
	"Screen Height: %3u ",
	"CPU Speed: %3dMHz ",
	"JoyStick: %s ",
	"Wifi Game: %s ",
	"P2P Sync Game ",
	"Preview: %s ",
	"Mono Sound: %s ",
	"Exit FinaBurn Alpha"	
};

static void update_status_str(char *batt_str)
{
	char batt_life_str[10];
		int batt_life_per = scePowerGetBatteryLifePercent();
		
		if(batt_life_per < 0) {
			strcpy(batt_str, "BATT. --%");
		} else {
			sprintf(batt_str, "BATT.%3d%%", batt_life_per);
		}
		
		if(scePowerIsPowerOnline()) {
			strcpy(batt_life_str, "[DC IN]");
		} else {
			int batt_life_time = scePowerGetBatteryLifeTime();
			int batt_life_hour = (batt_life_time / 60) % 100;
			int batt_life_min = batt_life_time % 60;
			
			if(batt_life_time < 0) {
				strcpy(batt_life_str, "[--:--]");
			} else {
				sprintf(batt_life_str, "[%2d:%02d]", batt_life_hour, batt_life_min);
			}
		}

	strcat(batt_str, batt_life_str);
}
void draw_ui_main()
{
	char buf[320];
	if(bgIndex!=1)
	{
		if(access("bg1.png",0)==0)
		{
			loadImage(bgBuf,"bg1.png", &bgW, &bgH);
			bgIndex=1;				
		}
	}
	if(bgIndex!=1)
	{
		drawRect(GU_FRAME_ADDR(work_frame), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, UI_BGCOLOR);
	}else
	{
		drawImage(GU_FRAME_ADDR(work_frame), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
			(unsigned short*)bgBuf, bgW, bgH);
	}
	
	drawString(LBVer, GU_FRAME_ADDR(work_frame), 10, 10, UI_COLOR);
	update_status_str(buf);
	drawString(buf, GU_FRAME_ADDR(work_frame), 470 - getDrawStringLength(buf), 10, UI_COLOR);
    drawRect(GU_FRAME_ADDR(work_frame), 8, 28, 464, 1, UI_COLOR);
    
        
    for(int i=0; i<MENU_COUNT; i++)  {
	    	    
	    switch ( i ) {
	    
	    case LOAD_GAME:
	    	sprintf( buf, ui_main_menu[i],saveIndex );
			break;
		case SAVE_GAME:
	    	sprintf( buf, ui_main_menu[i],saveIndex );
			break;
	    case CONTROLLER:
	    	if(currentInp)
	    		sprintf( buf, ui_main_menu[i],currentInp );
	    	else
	    		strcpy(buf,"Controller: ALL");
			break;
	    case SKIP_FRAMES:
	    	sprintf( buf, ui_main_menu[i],gameSpeedCtrl );
			break;
		case SCREEN_MODE:
	    	sprintf( buf, ui_main_menu[i],screenMode );
			break;
		case GAME_SCREEN_WIDTH:
	    	sprintf( buf, ui_main_menu[i],gameScreenWidth );
			break;
		case GAME_SCREEN_HEIGHT:
	    	sprintf( buf, ui_main_menu[i],gameScreenHeight );
			break;		
	    case CPU_SPEED:
	    	sprintf( buf, ui_main_menu[i], cpu_speeds[cpu_speeds_select].cpu );
			break;
		case JOYSTICK:
			if(enableJoyStick)
	    		sprintf( buf, ui_main_menu[i], "ENABLED" );
	    	else
	    		sprintf( buf, ui_main_menu[i], "DISABLED" );
			break;
		case WIFI_GAME:
			switch(wifiStatus)
			{
				case 1:
					sprintf( buf, ui_main_menu[i], "HOST" );
					break;
				case 2:
					sprintf( buf, ui_main_menu[i], "CLIENT" );
					break;
				case 3:
					sprintf( buf, ui_main_menu[i], "P2P" );
					break;
				default:
					sprintf( buf, ui_main_menu[i], "OFF" );
					break;
			}
			break;
		case PREVIEW:
			if(needPreview)
				sprintf( buf, ui_main_menu[i], "ON" );
			else
				sprintf( buf, ui_main_menu[i], "OFF" );
			break;
		case MONO_SOUND:
			if(monoSound==1)
				sprintf( buf, ui_main_menu[i], "ON" );
			else
				sprintf( buf, ui_main_menu[i], "OFF" );
			break;
	    default:
	    	sprintf( buf, ui_main_menu[i]);
    	}
    	drawString(buf, 
	    			GU_FRAME_ADDR(work_frame), 
	    			80+240*(i/10),
	    			44 + (i%10) * 18, UI_COLOR);
	}
	drawRect(GU_FRAME_ADDR(work_frame), 2+240*(ui_mainmenu_select/10), 40+(ui_mainmenu_select%10)*18, 236, 18, UI_COLOR,0x40);
	
    drawRect(GU_FRAME_ADDR(work_frame), 8, 230, 464, 1, UI_COLOR);
    drawString("FB Alpha contains parts of MAME & Final Burn. (C) 2004, Team FB Alpha.", GU_FRAME_ADDR(work_frame), 10, 238, UI_COLOR);
    drawString("FinalBurn Alpha for PSP (C) 2008, OopsWare and LBICELYNE", GU_FRAME_ADDR(work_frame), 10, 255, UI_COLOR);
	
	//Draw preview
	if(ui_mainmenu_select==LOAD_GAME&&needPreview&&nBurnDrvSelect < nBurnDrvCount)
	{
	    strcpy(buf,szAppCachePath);
		strcat(buf,BurnDrvGetTextA(DRV_NAME));
		sprintf(buf+300,"_%1u.png",saveIndex);	
		strcat(buf,buf+300);
		
		if(access(buf,0)==0)
		{
			unsigned int imgW,imgH;
			loadImage(previewBuf,buf, &imgW, &imgH);
			drawImage(GU_FRAME_ADDR(work_frame), 80, 90, 320, 180, 
			(unsigned short*)previewBuf, imgW, imgH);
		}
	}
	
}

void draw_ui_browse(bool rebuiltlist)
{
	unsigned int bds = nBurnDrvSelect;
	char buf[1024];
	if(bgIndex!=2)
	{
		if(access("bg2.png",0)==0)
		{
			loadImage(bgBuf,"bg2.png", &bgW, &bgH);
			bgIndex=2;				
		}
	}
	if(bgIndex!=2)
	{
		drawRect(GU_FRAME_ADDR(work_frame), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, UI_BGCOLOR);
	}else
	{
		drawImage(GU_FRAME_ADDR(work_frame), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
			(unsigned short*)bgBuf, bgW, bgH);
	}

	find_rom_count = findRomsInDir( rebuiltlist );

	strcpy(buf, "PATH: ");
	strcat(buf, ui_current_path);
	
	drawString(buf, GU_FRAME_ADDR(work_frame), 10, 10, UI_COLOR, 460);
    drawRect(GU_FRAME_ADDR(work_frame), 8, 28, 464, 1, UI_COLOR);
	
	for(int i=0; i<find_rom_list_cnt; i++) {
		char *p = getRomsFileName(i+find_rom_top);
		
		
		if (p) {
			switch( getRomsFileStat(i+find_rom_top) ) {
			case -2: // unsupport
			case -3: // not working
				drawString(p, GU_FRAME_ADDR(work_frame), 12, 44+i*18, R8G8B8_to_B5G6R5(0x808080), 180);
				break;
			case -1: // directry
				//drawString("<DIR>", GU_FRAME_ADDR(work_frame), 194, 44 + i*18, fc);
				break;
			default:
				drawString(p, GU_FRAME_ADDR(work_frame), 12, 44+i*18, UI_COLOR, 180);
			}
		}
		if ((i+find_rom_top) == find_rom_select) {
			drawRect(GU_FRAME_ADDR(work_frame), 10, 40+i*18, 140, 18, UI_COLOR, 0x40);
		}
		if ( find_rom_count > find_rom_list_cnt ) {
			drawRect(GU_FRAME_ADDR(work_frame), 154, 40, 5, 18 * find_rom_list_cnt, R8G8B8_to_B5G6R5(0x807060));
		
			drawRect(GU_FRAME_ADDR(work_frame), 154, 
					40 + find_rom_top * 18 * find_rom_list_cnt / find_rom_count , 5, 
					find_rom_list_cnt * 18 * find_rom_list_cnt / find_rom_count, UI_COLOR);
		} else
			drawRect(GU_FRAME_ADDR(work_frame), 154, 40, 5, 18 * find_rom_list_cnt, UI_COLOR);

	}
	
    drawRect(GU_FRAME_ADDR(work_frame), 8, 230, 464, 1, UI_COLOR);

	nBurnDrvSelect = getRomsFileStat(find_rom_select);

	strcpy(buf, "Game Info: ");
	if ( nBurnDrvSelect < nBurnDrvCount)
		strcat(buf, BurnDrvGetTextA( DRV_FULLNAME ) );
    drawString(buf, GU_FRAME_ADDR(work_frame), 10, 238, UI_COLOR, 460);

	strcpy(buf, "Released by: ");
	if ( nBurnDrvSelect < nBurnDrvCount ) {
		strcat(buf, BurnDrvGetTextA( DRV_MANUFACTURER ));
		strcat(buf, " (");
		strcat(buf, BurnDrvGetTextA( DRV_DATE ));
		strcat(buf, ", ");
		strcat(buf, BurnDrvGetTextA( DRV_SYSTEM ));
		strcat(buf, " hardware)");
	}
    drawString(buf, GU_FRAME_ADDR(work_frame), 10, 255, UI_COLOR, 460);
   

    if (needPreview&&nBurnDrvSelect < nBurnDrvCount ) {
	    strcpy(buf,szAppCachePath);
		strcat(buf,BurnDrvGetTextA(DRV_NAME));
		strcat(buf,".png");
		int i=-1;
		if(access(buf,0)!=0)
		{
			for(i=0;i<10;i++)
			{
				strcpy(buf,szAppCachePath);
				strcat(buf,BurnDrvGetTextA(DRV_NAME));
				sprintf(buf+512,"_%1u.png",i);	
				strcat(buf,buf+512);
				if(access(buf,0)==0)
					break;
			}
		}
		if(i<10)
		{
			unsigned int imgW,imgH;
			loadImage(previewBuf,buf, &imgW, &imgH);
			drawImage(GU_FRAME_ADDR(work_frame), 160, 40, 320, 180, 
			(unsigned short*)previewBuf, imgW, imgH);
			
		}
    }
	nBurnDrvSelect = bds;
}

static void return_to_game()
{
	if ( nPrevGame < nBurnDrvCount ) {
		if(wifiStatus)
			adhocInit(BurnDrvGetTextA(DRV_NAME));
		else
			adhocTerm();
		if(wifiStatus!=2)
		{
				scePowerSetClockFrequency(
								cpu_speeds[cpu_speeds_select].cpu, 
								cpu_speeds[cpu_speeds_select].cpu, 
								cpu_speeds[cpu_speeds_select].bus );				
				sound_continue();
		}			
		setGameStage(0);
	}
}

static void process_key( int key, int down, int repeat )
{
	char buf[16];
	if ( !down ) return ;
	switch( nGameStage ) {
	/* ---------------------------- Main Menu ---------------------------- */
	case 1:		
		//ui_mainmenu_select
		switch( key ) {
		case PSP_CTRL_UP:
			if (ui_mainmenu_select <= 0)
				ui_mainmenu_select = MENU_COUNT-1;
			else
				ui_mainmenu_select--;
			draw_ui_main();
			break;
		case PSP_CTRL_DOWN:
			if (ui_mainmenu_select >=MENU_COUNT-1 )
				ui_mainmenu_select = 0;
			else
				ui_mainmenu_select++;
			draw_ui_main();
			break;

		case PSP_CTRL_LEFT:
			switch(ui_mainmenu_select) {
			case LOAD_GAME:
			case SAVE_GAME:
				saveIndex--;
				if(saveIndex<0)
					saveIndex=9;
				draw_ui_main();
				break;
			case CONTROLLER:
				currentInp--;
				if(currentInp<0)
					currentInp=4;
				if(nPrevGame!=~0U)
					DoInputBlank(0);
				draw_ui_main();
				break;
			case SKIP_FRAMES:
				gameSpeedCtrl--;
				if ( gameSpeedCtrl<0 ) 
				{
					gameSpeedCtrl=8;
				}
				draw_ui_main();
				break;
			case SCREEN_MODE:				
				screenMode--;
				if ( screenMode < 0 ) {
					screenMode=8;
				}	
				if(nPrevGame!=~0U)
					DoInputBlank(0);
				processScreenMode();
				draw_ui_main();					
				break;
			case GAME_SCREEN_WIDTH:				
				gameScreenWidth=gameScreenWidth-2;
				if ( gameScreenWidth < 2 ) {
					gameScreenWidth=SCREEN_WIDTH;
				}	
				draw_ui_main();					
				break;
			case GAME_SCREEN_HEIGHT:				
				gameScreenHeight=gameScreenHeight-2;
				if ( gameScreenHeight < 2 ) {
					gameScreenHeight=SCREEN_HEIGHT;
				}	
				draw_ui_main();					
				break;
			case CPU_SPEED:
				if ( cpu_speeds_select > 0 ) {
					cpu_speeds_select--;
					draw_ui_main();
				}
				break;
			case JOYSTICK:
				enableJoyStick=!enableJoyStick;
				draw_ui_main();
				break;
			case WIFI_GAME:
				wifiStatus--;
				if(wifiStatus<0)
				{
					wifiStatus=3;
				}
				draw_ui_main();
				break;
			case PREVIEW:
				needPreview=!needPreview;
				draw_ui_main();
				break;
			case MONO_SOUND:
				monoSound=!monoSound;
				draw_ui_main();
				break;
			default:
				if(ui_mainmenu_select>=10)
					ui_mainmenu_select=ui_mainmenu_select-10;
				else
					ui_mainmenu_select=ui_mainmenu_select+10;
				draw_ui_main();
				break;
			}
			break;
		case PSP_CTRL_RIGHT:
			switch(ui_mainmenu_select) {
			case LOAD_GAME:
			case SAVE_GAME:
				saveIndex++;
				if(saveIndex>9)
					saveIndex=0;
				draw_ui_main();
				break;
			case CONTROLLER:
				currentInp++;
				if(currentInp>4)
					currentInp=0;
				if(nPrevGame!=~0U)
					DoInputBlank(0);
				draw_ui_main();
				break;
			case SKIP_FRAMES:
				gameSpeedCtrl++;
				if ( gameSpeedCtrl > 8) 
				{
					gameSpeedCtrl=0;
				}
				draw_ui_main();
				break;
			case SCREEN_MODE:				
				screenMode++;
				if ( screenMode>8 ) {
					screenMode=0;
				}
					
				if(nPrevGame!=~0U)
					DoInputBlank(0);
				processScreenMode();
				draw_ui_main();					
								
				break;
			case GAME_SCREEN_WIDTH:				
				gameScreenWidth=gameScreenWidth+2;
				if ( gameScreenWidth > SCREEN_WIDTH ) {
					gameScreenWidth=2;
				}	
				draw_ui_main();					
				break;
			case GAME_SCREEN_HEIGHT:				
				gameScreenHeight=gameScreenHeight+2;
				if ( gameScreenHeight > SCREEN_HEIGHT ) {
					gameScreenHeight=2;
				}	
				draw_ui_main();					
				break;
			case CPU_SPEED:
				if ( cpu_speeds_select < 3 ) {
					cpu_speeds_select++;
					draw_ui_main();
				}
				break;
			case JOYSTICK:
				enableJoyStick=!enableJoyStick;
				draw_ui_main();
				break;
			case WIFI_GAME:
				wifiStatus++;
				if(wifiStatus>3)
				{
					wifiStatus=0;
				}
				draw_ui_main();
				break;
			case PREVIEW:
				needPreview=!needPreview;
				draw_ui_main();
				break;
			case MONO_SOUND:
				monoSound=!monoSound;
				draw_ui_main();
				break;
			default:
				if(ui_mainmenu_select>=10)
					ui_mainmenu_select=ui_mainmenu_select-10;
				else
					ui_mainmenu_select=ui_mainmenu_select+10;
				draw_ui_main();
				break;
			}
			break;
			
		case PSP_CTRL_CIRCLE:
			switch( ui_mainmenu_select ) {
			case SELECT_ROM:
				setGameStage(2);
				strcpy(ui_current_path, szAppRomPath);
				//ui_current_path[strlen(ui_current_path)-1] = 0;
				draw_ui_browse(true);
				break;
			case LOAD_GAME:
				if(nPrevGame != ~0U&&wifiStatus!=2)
				{
					strcpy(ui_current_path,szAppCachePath);
					strcat(ui_current_path,BurnDrvGetTextA(DRV_NAME));
					sprintf(buf,"_%1u.sav",saveIndex);	
					strcat(ui_current_path,buf);
									
					if(!BurnStateLoad(ui_current_path,1,&DrvInitCallback))
					{
						scePowerSetClockFrequency(
									cpu_speeds[cpu_speeds_select].cpu, 
									cpu_speeds[cpu_speeds_select].cpu, 
									cpu_speeds[cpu_speeds_select].bus );
						setGameStage(0);
						sound_continue();
					}
				}
				break;
			case SAVE_GAME:
				if(nPrevGame != ~0U&&wifiStatus!=2)
				{
					strcpy(ui_current_path,szAppCachePath);
					strcat(ui_current_path,BurnDrvGetTextA(DRV_NAME));
					sprintf(buf,"_%1u.sav",saveIndex);	
					strcat(ui_current_path,buf);
					if(!BurnStateSave(ui_current_path,1))
					{
						strcpy(ui_current_path,szAppCachePath);
						strcat(ui_current_path,BurnDrvGetTextA(DRV_NAME));
						sprintf(buf,"_%1u.png",saveIndex);	
						strcat(ui_current_path,buf);
						screenshot(ui_current_path);
						scePowerSetClockFrequency(
									cpu_speeds[cpu_speeds_select].cpu, 
									cpu_speeds[cpu_speeds_select].cpu, 
									cpu_speeds[cpu_speeds_select].bus );
						setGameStage(0);
						sound_continue();
					}
					
				}
				break;
			case RESET_GAME:
				if(nPrevGame != ~0U)
				{					
						scePowerSetClockFrequency(
								cpu_speeds[cpu_speeds_select].cpu, 
								cpu_speeds[cpu_speeds_select].cpu, 
								cpu_speeds[cpu_speeds_select].bus );
						resetGame();
						if(wifiStatus==3)
							wifiSend(WIFI_CMD_RESET);					
				}
				break;
			case SCREEN_SHOT:
				if(nPrevGame != ~0U)
				{		
						strcpy(ui_current_path,szAppCachePath);
						strcat(ui_current_path,BurnDrvGetTextA(DRV_NAME));
						strcat(ui_current_path,".png");
						screenshot(ui_current_path);
						scePowerSetClockFrequency(
									cpu_speeds[cpu_speeds_select].cpu, 
									cpu_speeds[cpu_speeds_select].cpu, 
									cpu_speeds[cpu_speeds_select].bus );
						setGameStage(0);
						sound_continue();
				}
				break;
			case SYNC_GAME:
				if(nPrevGame != ~0U)
				{					
						return_to_game();
						sendSyncGame();
				}
				break;
			case EXIT_FBA:	// Exit
				bGameRunning = 0;
				break;
				
			}
			break;
			
		case PSP_CTRL_CROSS:
			return_to_game();
			break;
		}
		break;
	/* ---------------------------- Rom Browse ---------------------------- */
	case 2:		
		switch( key ) {
		case PSP_CTRL_UP:
			if (find_rom_select == 0) break;
			if (find_rom_top >= find_rom_select) find_rom_top--;
			find_rom_select--;
			draw_ui_browse(false);
			break;
		case PSP_CTRL_DOWN:
			if ((find_rom_select+1) >= find_rom_count) break;
			find_rom_select++;
			if ((find_rom_top + find_rom_list_cnt) <= find_rom_select) find_rom_top++;
			draw_ui_browse(false);
			break;
		case PSP_CTRL_LTRIGGER:
			find_rom_top=find_rom_top-find_rom_list_cnt;
			find_rom_select=find_rom_select-find_rom_list_cnt;
			if (find_rom_top < 0)
			{
				 find_rom_top=0;
				 find_rom_select=0;
			}
			draw_ui_browse(false);
			break;
		case PSP_CTRL_RTRIGGER:
			find_rom_top=find_rom_top+find_rom_list_cnt;
			find_rom_select=find_rom_select+find_rom_list_cnt;
			if (find_rom_select >= find_rom_count)
			{
				 find_rom_top=find_rom_count- find_rom_list_cnt;
				 find_rom_select=find_rom_count-1;
			}
			draw_ui_browse(false);
			break;
			
		case PSP_CTRL_CIRCLE:
			switch( getRomsFileStat(find_rom_select) ) {
			case -1:	// directry
			/*	{		// printf("change dir %s\n", getRomsFileName(find_rom_select) );
					char * pn = getRomsFileName(find_rom_select);
					if ( strcmp("..", pn) ) {
						strcat(ui_current_path, getRomsFileName(find_rom_select));
						strcat(ui_current_path, "/");
					} else {
						if (strlen(strstr(ui_current_path, ":/")) == 2) break;	// "ROOT:/"
						for(int l = strlen(ui_current_path)-1; l>1; l-- ) {
							ui_current_path[l] = 0;
							if (ui_current_path[l-1] == '/') break;
						}
					}
					//printf("change dir to %s\n", ui_current_path );
					find_rom_count = 0;
					find_rom_select = 0;
					find_rom_top = 0;
					draw_ui_browse(true);
				}
				break;*/
			default: // rom zip file
				{
					nBurnDrvSelect = (unsigned int)getRomsFileStat( find_rom_select );

					if (nPrevGame == nBurnDrvSelect) {
						// same game, reture to it
						return_to_game();
						break;
					}
					
					if ( nPrevGame < nBurnDrvCount ) {
						nBurnDrvSelect = nPrevGame;
						nPrevGame = ~0U;
						DrvExit();
						InpExit();
						adhocTerm();
						loadDefaultInput();
						nCurrentFrame=0;
						
					}
					
					nBurnDrvSelect = (unsigned int)getRomsFileStat( find_rom_select );
					if (nBurnDrvSelect <= nBurnDrvCount && BurnDrvIsWorking() ) {
						
						setGameStage(3);
						ui_process_pos = 0;

						if ( DrvInit( nBurnDrvSelect, false ) == 0 ) {
							
							BurnRecalcPal();
							InpInit();
							InpDIP();
							nPrevGame = nBurnDrvSelect;
							return_to_game();
							
						} else {
							nBurnDrvSelect = ~0U; 
							setGameStage(2);
						}

					} else
						nBurnDrvSelect = ~0U; 

					nPrevGame = nBurnDrvSelect;
											
					//if (nBurnDrvSelect == ~0U) {
					//	bprintf(0, "unkown rom %s", getRomsFileName(find_rom_select));
					//}
				}
				
				
				
			}
			break;
		case PSP_CTRL_CROSS:	// cancel
			setGameStage (1);
			draw_ui_main();
			break;
		}
		break;
	/* ---------------------------- Loading Game ---------------------------- */
	case 3:		
		
		break;

	}
}

int do_ui_key(unsigned int key)
{
	// mask keys
	key &= PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_CIRCLE | PSP_CTRL_CROSS ;
	static int prvkey = 0;
	static int repeat = 0;
	static int repeat_time = 0;
	
	if (key != prvkey) {
		int def = key ^ prvkey;
		repeat = 0;
		repeat_time = 0;
		process_key( def, def & key, 0 );
		if (def & key) {
			// auto repeat up / down only
			repeat = def & (PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER);
		} else repeat = 0;
		prvkey = key;
	} else {
		if ( repeat ) {
			repeat_time++;
			if ((repeat_time >= 32) && ((repeat_time & 0x3) == 0))
				process_key( repeat, repeat, repeat_time );
		}
	}
	return 0;
}


void ui_update_progress(float size, char * txt)
{
	if(bgIndex!=2)
		drawRect( GU_FRAME_ADDR(work_frame), 10, 238, 460, 30, UI_BGCOLOR );
	else
		drawImage(GU_FRAME_ADDR(work_frame), 10, 238, 460, 30, 
			(unsigned short*)(bgBuf+238*PSP_LINE_SIZE*2+10*2), 460, 30);
	drawRect( GU_FRAME_ADDR(work_frame), 10, 238, 460, 12, R8G8B8_to_B5G6R5(0x807060) );
	drawRect( GU_FRAME_ADDR(work_frame), 10, 238, ui_process_pos, 12, R8G8B8_to_B5G6R5(0xffc090) );

	int sz = (int)(460 * size + 0.5);
	if (sz + ui_process_pos > 460) sz = 460 - ui_process_pos;
	drawRect( GU_FRAME_ADDR(work_frame), 10 + ui_process_pos, 238, sz, 12, R8G8B8_to_B5G6R5(0xc09878) );
	drawString(txt, GU_FRAME_ADDR(work_frame), 10, 255, UI_COLOR, 460);
	
	ui_process_pos += sz;
	if (ui_process_pos > 460) ui_process_pos = 460;

	update_gui();
	show_frame = draw_frame;
	draw_frame = sceGuSwapBuffers();
}

void ui_update_progress2(float size, const char * txt)
{
	static int ui_process_pos2 = 0;
	int sz = (int)(460.0 * size + 0.5);
	if ( txt ) ui_process_pos2 = sz;
	else ui_process_pos2 += sz;
	if ( ui_process_pos2 > 460 ) ui_process_pos2 = 460;
	drawRect( GU_FRAME_ADDR(work_frame), 10, 245, ui_process_pos2, 3, R8G8B8_to_B5G6R5(0xf06050) );
	
	if ( txt ) {
		if(bgIndex!=2)	
			drawRect( GU_FRAME_ADDR(work_frame), 10, 255, 460, 13, UI_BGCOLOR );
		else
			drawImage(GU_FRAME_ADDR(work_frame), 10,  255, 460, 13, 
			(unsigned short*)(bgBuf+255*PSP_LINE_SIZE*2+10*2), 460, 13);
		drawString(txt, GU_FRAME_ADDR(work_frame), 10, 255, UI_COLOR, 460);	
	}

	update_gui();
	show_frame = draw_frame;
	draw_frame = sceGuSwapBuffers();
}
void setGameStage(int stage)
{
	nGameStage=stage;
	configureVertices();
}



