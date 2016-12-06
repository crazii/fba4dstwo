#include "burnint.h"
#include "ics2115.h"

#define PGM_LOW_MEMORY

// pgm_run.cpp
extern unsigned char PgmJoy1[];
extern unsigned char PgmJoy2[];
extern unsigned char PgmJoy3[];
extern unsigned char PgmJoy4[];
extern unsigned char PgmBtn1[];
extern unsigned char PgmBtn2[];
extern unsigned char PgmInput[];
extern unsigned char PgmReset;

int pgmInit();
int pgmExit();
int pgmFrame();
int pgmDraw();
int pgmScan(int nAction, int *pnMin);

extern int nPGM68KROMLen;
extern int nPGMSPRColMaskLen;
extern int nPGMSPRMaskMaskLen;
extern int nPGMTileROMLen;

extern unsigned char *Ram68K;
extern unsigned char *USER0, *USER1, *USER2;
extern unsigned char *PGM68KROM, *PGMARMROM;
extern unsigned char *PGMARMRAM0, *PGMARMRAM1, *PGMARMRAM2, *PGMARMShareRAM;
extern unsigned short *RamRs, *RamPal, *RamVReg, *RamSpr;
extern unsigned int *RamBg, *RamTx, *RamCurPal;
extern unsigned char nPgmPalRecalc;

extern unsigned long nPGMTileROMOffset;
extern unsigned long nPGMSPRColROMOffset;
extern unsigned long nPGMSPRMaskROMOffset;

extern unsigned int * pgmSprIndex;
extern unsigned char * pgmIdxCacheTemp;
extern unsigned char * pgmDatCacheTemp;
extern unsigned char * pgmSprCache;
extern unsigned int PgmCacheOffset;
extern FILE * pgmCacheFile;

extern void (*pPgmInitCallback)();
extern void (*pPgmResetCallback)();
extern int (*pPgmScanCallback)(int, int*);

void pgm_cpu_sync();
void pgm_arm7_resume();
void pgm_arm7_suspend();

// pgm_draw
extern int pgmDraw();

// pgm_prot.cpp

void install_asic28_protection();
void install_pstars_protection();
void install_dw2_protection();
void install_killbldt_protection();
void install_asic3_protection();
void install_asic27A_protection();
void install_oldsa_protection();
void install_kovsh_protection();

void pstars_reset();
void killbldt_reset();
void asic28_reset();
void asic3_reset();
void oldsa_reset();

void olds_ramhack();

// pgm_crypt
void pgm_kov_decrypt();
void pgm_kovsh_decrypt();
void pgm_dw2_decrypt();
void pgm_djlzz_decrypt();
void pgm_pstar_decrypt();
void pgm_dw3_decrypt();
void pgm_killbld_decrypt();
void pgm_dfront_decrypt();
void pgm_ddp2_decrypt();
void pgm_mm_decrypt();
void pgm_kov2_decrypt();
void pgm_puzzli2_decrypt();
void pgm_kov2p_decrypt();
void pgm_theglad_decrypt();
void pgm_oldsplus_decrypt();
void pgm_kovshp_decrypt();
void pgm_killbldp_decrypt();
void pgm_svg_decrypt();

struct BoffsetHead
{
	unsigned char cacheIndexLow;
	unsigned char cacheIndexHigh;
	unsigned char magicChar1;
	unsigned char magicChar2;
};
struct SpriteCacheHead
{
	BoffsetHead* sprMaskHeadPtr;
	SpriteCacheHead* nextSrcPtr; 
	unsigned short cacheIndex;
	unsigned short wide;
	unsigned short high;
	unsigned short reserved;
	
};
struct SpriteCacheIndex
{
	unsigned int aoff;
	SpriteCacheHead *src;
};

#define SPRITE_CACHE_SIZE 8192
extern SpriteCacheIndex *spriteCacheArray;
extern unsigned int spriteCacheArrayFreeP;
