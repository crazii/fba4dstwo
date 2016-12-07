#include <stdio.h>
#include <string.h>

#include "nds.h"
#include "font.h"
#include "burnint.h"
#include "UniCache.h"
#include "burner.h"

//note: font(gui) directly write to screen buffer (DOWN_SCREEN, down_screen_addr, SCREEN_WIDTH, SCREEN_HEIGHT)

#define find_rom_list_cnt	6
#define menu_item_height 18

short gameSpeedCtrl = 1;
unsigned int hotButtons = (KEY_A|KEY_B|KEY_Y);
short screenMode=0;
short saveIndex=0;
char LBVer[]="FinalBurn Alpha for NDS "SUB_VERSION" (ver: 1.0)";
static int find_rom_count = 0;
static int find_rom_select = 0;
static int find_rom_top = 0;
char ui_current_path[MAX_PATH];

static unsigned int nPrevGame = ~0U;

static int ui_mainmenu_select = 0;

static int ui_process_pos = 0;
int InpMake(unsigned int);
int DoInputBlank(int bDipSwitch);

int DrvInitCallback()
{
	return DrvInit(nBurnDrvSelect,false);
}

int cpu_speeds[] = { 336, 350, 384, 396 };
static int cpu_speeds_select = 3;

enum uiMainIndex
{
	SELECT_ROM=0,
	LOAD_GAME,
	SAVE_GAME,
	RESET_GAME,
	CONTROLLER,
	SKIP_FRAMES,
	CPU_SPEED,
	MONO_SOUND,
	EXIT_FBA,
	MENU_COUNT
};
static char *ui_main_menu[] = {
	"Select ROM ",
	"%1u Load Game ",
	"%1u Save Game ",
	"Reset Game ",
	"Controller: %1uP ",
	"Max Skip Frames: %d",
	"CPU Speed: %3dMHz ",
	"Mono Sound: %s ",
	"Exit FinaBurn Alpha"	
};

void draw_ui_main()
{
	char buf[320];
	int x = 0, y = 0;
	drawRect((unsigned short*)down_screen_addr, x, y, SCREEN_WIDTH, SCREEN_HEIGHT, UI_BGCOLOR);
	x = 10; y = 10;
	
	drawString(LBVer, (unsigned short*)down_screen_addr, x, y, UI_COLOR);
	y += FONT_HEIGHT + 6;	//font + spacing
	x -= 2;
	
    drawRect((unsigned short*)down_screen_addr, x, y, SCREEN_WIDTH-x*2, 1, UI_COLOR);
    
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
	    case CPU_SPEED:
	    	sprintf( buf, ui_main_menu[i], cpu_speeds[cpu_speeds_select] );
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
	    			(unsigned short*)down_screen_addr, 
	    			40+SCREEN_WIDTH/2*(i/10),
	    			44 + (i%10) * menu_item_height, UI_COLOR);
	}
	drawRect((unsigned short*)down_screen_addr, 2+SCREEN_WIDTH/2*(ui_mainmenu_select/10), 40+(ui_mainmenu_select%10)*menu_item_height, SCREEN_HEIGHT-FONT_HEIGHT*2, menu_item_height, UI_COLOR,0x40);
	
    drawRect((unsigned short*)down_screen_addr, x, SCREEN_HEIGHT-FONT_HEIGHT*2-1, SCREEN_WIDTH-x*2, 1, UI_COLOR);
    drawString("FB Alpha contains parts of MAME & Final Burn. (C) 2004, Team FB Alpha.", (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT*2, UI_COLOR);
    drawString("FinalBurn Alpha for DS (C) 2008, Crazii", (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT, UI_COLOR);	
}

void draw_ui_browse(bool rebuiltlist)
{
	unsigned int bds = nBurnDrvSelect;
	char buf[1024];
	drawRect((unsigned short*)down_screen_addr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, UI_BGCOLOR);

	find_rom_count = findRomsInDir( rebuiltlist );

	strcpy(buf, "PATH: ");
	strcat(buf, ui_current_path);
	
	int x = 10, y = 10;
	drawString(buf, (unsigned short*)down_screen_addr, x, y, UI_COLOR, SCREEN_WIDTH-20);
	y += FONT_HEIGHT + 6;	//font + spacing
	x -= 2;
    drawRect((unsigned short*)down_screen_addr, x, y, SCREEN_WIDTH-x*2, 1, UI_COLOR);
	
	for(int i=0; i<find_rom_list_cnt; i++) {
		char *p = getRomsFileName(i+find_rom_top);
		
		
		if (p) {
			switch( getRomsFileStat(i+find_rom_top) ) {
			case -2: // unsupport
			case -3: // not working
				drawString(p, (unsigned short*)down_screen_addr, 12, 44+i*menu_item_height, R8G8B8_to_B5G6R5(0x808080), SCREEN_WIDTH/2-20);
				break;
			case -1: // directry
				break;
			default:
				drawString(p, (unsigned short*)down_screen_addr, 12, 44+i*menu_item_height, UI_COLOR, SCREEN_WIDTH/2-20);
			}
		}
		if ((i+find_rom_top) == find_rom_select) {
			drawRect((unsigned short*)down_screen_addr, 10, 40+i*menu_item_height, 140, menu_item_height, UI_COLOR, 0x40);
		}
		if ( find_rom_count > find_rom_list_cnt ) {
			drawRect((unsigned short*)down_screen_addr, 154, 40, 5, menu_item_height * find_rom_list_cnt, R8G8B8_to_B5G6R5(0x807060));
		
			drawRect((unsigned short*)down_screen_addr, 154, 
					40 + find_rom_top * menu_item_height * find_rom_list_cnt / find_rom_count , 5, 
					find_rom_list_cnt * menu_item_height * find_rom_list_cnt / find_rom_count, UI_COLOR);
		} else
			drawRect((unsigned short*)down_screen_addr, 154, 40, 5, menu_item_height * find_rom_list_cnt, UI_COLOR);

	}
	
    drawRect((unsigned short*)down_screen_addr, x, SCREEN_HEIGHT-FONT_HEIGHT*3, SCREEN_WIDTH-x*2, 1, UI_COLOR);

	nBurnDrvSelect = getRomsFileStat(find_rom_select);

	strcpy(buf, "Game Info: ");
	if ( nBurnDrvSelect < nBurnDrvCount)
		strcat(buf, BurnDrvGetTextA( DRV_FULLNAME ) );
    drawString(buf, (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT*2, UI_COLOR, SCREEN_WIDTH-20);

	strcpy(buf, "Released by: ");
	if ( nBurnDrvSelect < nBurnDrvCount ) {
		strcat(buf, BurnDrvGetTextA( DRV_MANUFACTURER ));
		strcat(buf, " (");
		strcat(buf, BurnDrvGetTextA( DRV_DATE ));
		strcat(buf, ", ");
		strcat(buf, BurnDrvGetTextA( DRV_SYSTEM ));
		strcat(buf, " hardware)");
	}
    drawString(buf, (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT, UI_COLOR, SCREEN_WIDTH-20);
	nBurnDrvSelect = bds;
}

static void return_to_game()
{
	setGameStage(0);
	
	if( cpu_speeds_select > 3)
		cpu_speeds_select = 3;
	else if(cpu_speeds_select < 0)
		cpu_speeds_select = 0;
	ds2_setCPUclocklevel(10 + cpu_speeds_select);
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
		case KEY_UP:
			if (ui_mainmenu_select <= 0)
				ui_mainmenu_select = MENU_COUNT-1;
			else
				ui_mainmenu_select--;
			draw_ui_main();
			break;
		case KEY_DOWN:
			if (ui_mainmenu_select >=MENU_COUNT-1 )
				ui_mainmenu_select = 0;
			else
				ui_mainmenu_select++;
			draw_ui_main();
			break;

		case KEY_LEFT:
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
			case CPU_SPEED:
				if ( cpu_speeds_select > 0 ) {
					cpu_speeds_select--;
					draw_ui_main();
				}
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
		case KEY_RIGHT:
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
			case CPU_SPEED:
				if ( cpu_speeds_select < 3 ) {
					cpu_speeds_select++;
					draw_ui_main();
				}
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
			
		case KEY_A:
			switch( ui_mainmenu_select ) {
			case SELECT_ROM:
				setGameStage(2);
				strcpy(ui_current_path, szAppRomPath);
				//ui_current_path[strlen(ui_current_path)-1] = 0;
				draw_ui_browse(true);
				break;
			case LOAD_GAME:
				if(nPrevGame != ~0U)
				{
					strcpy(ui_current_path,szAppCachePath);
					strcat(ui_current_path,BurnDrvGetTextA(DRV_NAME));
					sprintf(buf,"_%1u.sav",saveIndex);	
					strcat(ui_current_path,buf);
									
					if(!BurnStateLoad(ui_current_path,1,&DrvInitCallback))
					{
						setGameStage(0);
						sound_continue();
					}
				}
				break;
			case SAVE_GAME:
				if(nPrevGame != ~0U)
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
						//screenshot(ui_current_path);
						setGameStage(0);
						sound_continue();
					}
					
				}
				break;
			case RESET_GAME:
				if(nPrevGame != ~0U)
				{					
					resetGame();
				}
				break;
			case EXIT_FBA:	// Exit
				bGameRunning = 0;
				break;
				
			}
			break;
			
		case KEY_B:
			return_to_game();
			break;
		}
		break;
	/* ---------------------------- Rom Browse ---------------------------- */
	case 2:		
		switch( key ) {
		case KEY_UP:
			if (find_rom_select == 0) break;
			if (find_rom_top >= find_rom_select) find_rom_top--;
			find_rom_select--;
			draw_ui_browse(false);
			break;
		case KEY_DOWN:
			if ((find_rom_select+1) >= find_rom_count) break;
			find_rom_select++;
			if ((find_rom_top + find_rom_list_cnt) <= find_rom_select) find_rom_top++;
			draw_ui_browse(false);
			break;
		case KEY_L:
			find_rom_top=find_rom_top-find_rom_list_cnt;
			find_rom_select=find_rom_select-find_rom_list_cnt;
			if (find_rom_top < 0)
			{
				 find_rom_top=0;
				 find_rom_select=0;
			}
			draw_ui_browse(false);
			break;
		case KEY_R:
			find_rom_top=find_rom_top+find_rom_list_cnt;
			find_rom_select=find_rom_select+find_rom_list_cnt;
			if (find_rom_select >= find_rom_count)
			{
				 find_rom_top=find_rom_count- find_rom_list_cnt;
				 find_rom_select=find_rom_count-1;
			}
			draw_ui_browse(false);
			break;
			
		case KEY_A:
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
		case KEY_B:	// cancel
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
	key &= KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_L | KEY_R | KEY_A | KEY_B ;
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
			repeat = def & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_L | KEY_R);
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
	drawRect( (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT*2, SCREEN_WIDTH-20, 30, UI_BGCOLOR );
	drawRect( (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT*2, SCREEN_WIDTH-20, 12, R8G8B8_to_B5G6R5(0x807060) );
	drawRect( (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT*2, ui_process_pos, 12, R8G8B8_to_B5G6R5(0xffc090) );

	int sz = (int)((SCREEN_WIDTH-20) * size + 0.5);
	if (sz + ui_process_pos > (SCREEN_WIDTH-20)) sz = (SCREEN_WIDTH-20) - ui_process_pos;
	drawRect( (unsigned short*)down_screen_addr, 10 + ui_process_pos, SCREEN_HEIGHT-FONT_HEIGHT*2, sz, 12, R8G8B8_to_B5G6R5(0xc09878) );
	drawString(txt, (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT, UI_COLOR, (SCREEN_WIDTH-20));
	
	ui_process_pos += sz;
	if (ui_process_pos > (SCREEN_WIDTH-20)) ui_process_pos = (SCREEN_WIDTH-20);

	update_gui();
}

void ui_update_progress2(float size, const char * txt)
{
	static int ui_process_pos2 = 0;
	int sz = (int)((SCREEN_WIDTH-20) * size + 0.5);
	if ( txt ) ui_process_pos2 = sz;
	else ui_process_pos2 += sz;
	if ( ui_process_pos2 > (SCREEN_WIDTH-20) ) ui_process_pos2 = (SCREEN_WIDTH-20);
	drawRect( (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT-10, ui_process_pos2, 3, R8G8B8_to_B5G6R5(0xf06050) );
	
	if ( txt ) {
		drawRect( (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT, (SCREEN_WIDTH-20), 13, UI_BGCOLOR );
		drawString(txt, (unsigned short*)down_screen_addr, 10, SCREEN_HEIGHT-FONT_HEIGHT, UI_COLOR, (SCREEN_WIDTH-20));	
	}

	update_gui();
}
void setGameStage(int stage)
{
	nGameStage=stage;
}



