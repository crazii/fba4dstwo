#include "burnint.h"
#include "ics2115.h"
#include "arm7.h"
//#define PGM_MUTE
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

extern int pgmInit();
extern int pgmKov2Init();
extern int pgmExit();
extern int pgmFrame();
extern int kov2Frame();
extern int pgmDraw();
extern int pgmScan(int nAction, int *pnMin);
void install_oldsa_protection();
void oldsa_reset();
extern void (*pPgmResetCallback)();

extern int nPGM68KROMLen;
extern unsigned char *Ram68K;
extern unsigned char *USER0, *USER1, *USER2;
extern unsigned char *PGM68KROM, *PGMTileROM, *PGMTileROMExp, *PGMSPRColROM, *PGMSPRMaskROM;
extern unsigned short *RamRs, *RamPal, *RamVReg, *RamSpr;
extern unsigned int *RamBg, *RamTx, *RamCurPal;
extern unsigned char nPgmPalRecalc;
extern int nPGMSPRColROMLen;
extern int nPGMSPRMaskROMLen;

extern bool bPgmUseCache;
extern unsigned int * pgmSprIndex;
extern unsigned char * pgmIdxCacheTemp;
extern unsigned char * pgmDatCacheTemp;
extern unsigned char * pgmSprCache;
extern unsigned int PgmCacheOffset;
extern FILE * pgmCacheFile;

#define PGM_CACHE_SIZE	0x1000000

extern void (*pPgmInitCallback)();
extern int (*pPgmScanCallback)(int, int*);

// pgm_draw
extern int pgmDraw();

// pgm_prot.cpp

extern void pgm_asic28_w(unsigned short offset, unsigned short data);
extern unsigned short pgm_asic28_r(unsigned short offset);

extern void pgm_asic3_reg_w(unsigned short offset, unsigned short data);
extern void pgm_asic3_w(unsigned short offset, unsigned short data);
extern unsigned char pgm_asic3_r(unsigned short offset);
extern unsigned short sango_protram_r(unsigned short offset);

extern unsigned short dw2_d80000_r(unsigned int sekAddress);

extern unsigned short *killbld_sharedprotram;
extern void killbld_prot_w(unsigned int sekAddress, unsigned short data);
extern unsigned short killbld_prot_r(unsigned int sekAddress);

extern unsigned short PSTARS_protram_r(unsigned int sekAddress);
extern unsigned short PSTARS_r16(unsigned int sekAddress);
extern void PSTARS_w16(unsigned int sekAddress, unsigned short data);


extern void prot_reset();

extern int asic28Scan(int nAction,int */*pnMin*/);
extern int asic3Scan(int nAction,int */*pnMin*/);
extern int killbldtScan(int nAction,int */*pnMin*/);
extern int pstarsScan(int nAction,int */*pnMin*/);
extern int oldsScan(int nAction,int */*pnMin*/);

// pgm_crypt.cpp

extern void pgm_kov_decrypt();
extern void pgm_kovsh_decrypt();
extern void pgm_dw2_decrypt();
extern void pgm_djlzz_decrypt();
extern void pgm_pstar_decrypt();
extern void pgm_dw3_decrypt();
extern void pgm_killbld_decrypt();
extern void pgm_dfront_decrypt();
extern void pgm_ddp2_decrypt();
extern void pgm_mm_decrypt();
extern void pgm_kov2_decrypt();
extern void pgm_kov2p_decrypt();
extern void pgm_puzzli2_decrypt();
extern void pgm_theglad_decrypt();

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
