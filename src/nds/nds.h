#ifndef _H_PSP_
#define _H_PSP_


#define PSP_LINE_SIZE	512
#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

#define GU_FRAME_ADDR(frame)		(unsigned short *)((unsigned int)frame | 0x44000000)

/* main.cpp */

extern int nGameStage;
extern int bGameRunning;
extern char currentPath[];
extern char szAppCachePath[];
extern unsigned int debugValue[2];
void resetGame();

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	signed short x, y, z;
};


/* ui.cpp */
#define UI_COLOR	R8G8B8_to_B5G6R5(0xffffff)
#define UI_BGCOLOR	R8G8B8_to_B5G6R5(0x102030)

extern char ui_current_path[];
extern void setGameStage(int stage);
int do_ui_key(unsigned int key);

void draw_ui_main();
void draw_ui_browse(bool rebuiltlist);

void ui_update_progress(float size, char * txt);
void ui_update_progress2(float size, const char * txt);
extern short gameSpeedCtrl;
extern unsigned int hotButtons ;
extern short screenMode;
extern short wifiStatus;
extern short gameScreenWidth, gameScreenHeight;
/* roms.cpp */

int findRomsInDir(bool force);
char * getRomsFileName(int idx);
int getRomsFileStat(int idx);

/* gui.cpp */

extern void * show_frame;
extern void * draw_frame;
extern void * work_frame;
extern void * tex_frame;
extern unsigned char *bgBuf,*previewBuf,*tmpBuf;
void init_gui();
void exit_gui();
void update_gui();
void configureVertices();
void drawPreview();
/* bzip */
extern char szAppRomPath[];


/* input */
int DrvInit(int nDrvNum, bool bRestore);
int DrvExit();
int InpInit();
int InpExit();
void InpDIP();
void loadDefaultInput();

/* snd.cpp */

#define SND_RATE		22050
#define SND_FRAME_SIZE	((SND_RATE * 100 + 5999) / 6000)
extern int mixbufidDiff;
extern unsigned char monoSound;
int sound_start();
int sound_stop();
void sound_next();
void sound_pause();
void sound_continue();

#endif	// _H_PSP_
