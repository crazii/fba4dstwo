#include "tiles_generic.h"

static unsigned char DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvDip[3]        = {0, 0, 0};
static unsigned char DrvInput[2]      = {0x00, 0x00};
static unsigned char DrvReset         = 0;

static unsigned char *Mem                 = NULL;
static unsigned char *MemEnd              = NULL;
static unsigned char *RamStart            = NULL;
static unsigned char *RamEnd              = NULL;
static unsigned char *DrvZ80Rom1          = NULL;
static unsigned char *DrvZ80Rom2          = NULL;
static unsigned char *DrvZ80Rom3          = NULL;
static unsigned char *Drv54xxRom          = NULL;
static unsigned char *Drv51xxRom          = NULL;
static unsigned char *DrvVideoRam         = NULL;
static unsigned char *DrvSharedRam1       = NULL;
static unsigned char *DrvSharedRam2       = NULL;
static unsigned char *DrvSharedRam3       = NULL;
static unsigned char *DrvPromPalette      = NULL;
static unsigned char *DrvPromCharLookup   = NULL;
static unsigned char *DrvPromSpriteLookup = NULL;
static unsigned char *DrvChars            = NULL;
static unsigned char *DrvSprites          = NULL;
static unsigned char *DrvTempRom          = NULL;
static unsigned int  *DrvPalette          = NULL;

static unsigned char DrvCPU1FireIRQ;
static unsigned char DrvCPU2FireIRQ;
static unsigned char DrvCPU3FireIRQ;
static unsigned char DrvCPU2Halt;
static unsigned char DrvCPU3Halt;

static int nCyclesDone[3], nCyclesTotal[3];
static int nCyclesSegment;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 5, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p2 start"  },

	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 6, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Drv);

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = 0xff;

	// Compile Digital Inputs
	for (int i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
	}
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x80, NULL                     },
	{0x0d, 0xff, 0xff, 0xf7, NULL                     },
	{0x0e, 0xff, 0xff, 0x97, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x80, 0x80, "Off"                    },
	{0x0c, 0x01, 0x08, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x0d, 0x01, 0x03, 0x03, "Easy"                   },
	{0x0d, 0x01, 0x03, 0x00, "Medium"                 },
	{0x0d, 0x01, 0x03, 0x01, "Hard"                   },
	{0x0d, 0x01, 0x03, 0x02, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0d, 0x01, 0x08, 0x08, "Off"                    },
	{0x0d, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0d, 0x01, 0x10, 0x10, "Off"                    },
	{0x0d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Rack Test"              },
	{0x0d, 0x01, 0x20, 0x20, "Off"                    },
	{0x0d, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x80, 0x80, "Upright"                },
	{0x0d, 0x01, 0x80, 0x00, "Cocktail"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x0e, 0x01, 0x07, 0x04, "4 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x06, "2 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x0e, 0x01, 0x07, 0x01, "2 Coins 3 Plays"        },
	{0x0e, 0x01, 0x07, 0x03, "1 Coin  2 Plays"        },
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x0e, 0x01, 0x07, 0x00, "Freeplay"               },	
	
	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x0e, 0x01, 0x38, 0x20, "20k  60k  60k"          },
	{0x0e, 0x01, 0x38, 0x18, "20k  60k"               },
	{0x0e, 0x01, 0x38, 0x10, "20k  70k  70k"          },
	{0x0e, 0x01, 0x38, 0x30, "20k  80k  80k"          },
	{0x0e, 0x01, 0x38, 0x38, "30k  80k"               },
	{0x0e, 0x01, 0x38, 0x08, "30k 100k 100k"          },
	{0x0e, 0x01, 0x38, 0x28, "30k 120k 120k"          },
	{0x0e, 0x01, 0x38, 0x00, "None"                   },	
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0xc0, 0x00, "2"                      },
	{0x0e, 0x01, 0xc0, 0x80, "3"                      },
	{0x0e, 0x01, 0xc0, 0x40, "4"                      },
	{0x0e, 0x01, 0xc0, 0xc0, "5"                      },
};

STDDIPINFO(Drv);

static struct BurnRomInfo DrvRomDesc[] = {
	{ "gg1-1b.3p",     0x01000, 0xab036c9f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "gg1-2b.3m",     0x01000, 0xd9232240, BRF_ESS | BRF_PRG }, //	 1
	{ "gg1-3.2m",      0x01000, 0x753ce503, BRF_ESS | BRF_PRG }, //	 2
	{ "gg1-4b.2l",     0x01000, 0x499fcc76, BRF_ESS | BRF_PRG }, //	 3
	
	{ "gg1-5b.3f",     0x01000, 0xbb5caae3, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "gg1-7b.2c",     0x01000, 0xd016686b, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "54xx.bin",      0x00400, 0xee7357e0, BRF_ESS | BRF_PRG }, //  6	54XX Program Code
	
	{ "51xx.bin",      0x00400, 0xc2f57ef8, BRF_ESS | BRF_PRG }, //  7	51XX Program Code
	
	{ "gg1-9.4l",      0x01000, 0x58b2f47c, BRF_GRA },	     //  8	Characters
	
	{ "gg1-11.4d",     0x01000, 0xad447c80, BRF_GRA },	     //  9	Sprites
	{ "gg1-10.4f",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  10
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  11	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  12
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  13
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  14
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  15
};

STD_ROM_PICK(Drv);
STD_ROM_FN(Drv);

static struct BurnRomInfo GallagRomDesc[] = {
	{ "gg1-1",         0x01000, 0xa3a0f743, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "gallag.2",      0x01000, 0x5eda60a7, BRF_ESS | BRF_PRG }, //	 1
	{ "gg1-3.2m",      0x01000, 0x753ce503, BRF_ESS | BRF_PRG }, //	 2
	{ "gg1-4",         0x01000, 0x83874442, BRF_ESS | BRF_PRG }, //	 3
	
	{ "gg1-5",         0x01000, 0x3102fccd, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "gg1-7",         0x01000, 0x8995088d, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "gallag.6",      0x01000, 0x001b70bc, BRF_ESS | BRF_PRG }, //  6	Z80 #4 Program Code
	
	{ "gallag.8",      0x01000, 0x169a98a4, BRF_GRA },	     //  7	Characters
	
	{ "gg1-11.4d",     0x01000, 0xad447c80, BRF_GRA },	     //  8	Sprites
	{ "gg1-10.4f",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  9
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  10	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  11
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  12
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  13
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  14
};

STD_ROM_PICK(Gallag);
STD_ROM_FN(Gallag);

static int MemIndex()
{
	unsigned char *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x04000;
	DrvZ80Rom2             = Next; Next += 0x04000;
	DrvZ80Rom3             = Next; Next += 0x04000;
	Drv54xxRom             = Next; Next += 0x00400;
	Drv51xxRom             = Next; Next += 0x00400;
	DrvPromPalette         = Next; Next += 0x00020;
	DrvPromCharLookup      = Next; Next += 0x00100;
	DrvPromSpriteLookup    = Next; Next += 0x00100;
	
	RamStart               = Next;

	DrvVideoRam            = Next; Next += 0x00800;
//	DrvSharedRam1          = Next; Next += 0x00400;
	DrvSharedRam1          = Next; Next += 0x04000;
	DrvSharedRam2          = Next; Next += 0x00400;
	DrvSharedRam3          = Next; Next += 0x00400;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x100 * 8 * 8;
	DrvSprites             = Next; Next += 0x080 * 16 * 16;
	DrvPalette             = (unsigned int*)Next; Next += 576 * sizeof(unsigned int);

	MemEnd                 = Next;

	return 0;
}

static int DrvDoReset()
{
	for (int i = 0; i < 3; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
//	ZetOpen(2);
//	ZetNmi();
//	nCyclesDone[2] += ZetRun(200);
//	ZetClose();
	
	DrvCPU1FireIRQ = 0;
	DrvCPU2FireIRQ = 0;
	DrvCPU3FireIRQ = 0;
	DrvCPU2Halt = 0;
	DrvCPU3Halt = 0;

	return 0;
}

static int customio_command;
static int mode,credits;
static int coinpercred,credpercoin;
static unsigned char customio[16];

static unsigned char galaga_customio_r()
{
	return customio_command;
}

static void galaga_customio_w(unsigned char data)
{
	customio_command = data;

	switch (data)
	{
		case 0x10:
//			if (nmi_timer) timer_remove (nmi_timer);
//			nmi_timer = 0;
			return;

		case 0xa1:	/* go into switch mode */
			mode = 1;
			ZetNmi();
			return;

//		case 0xe1:	/* go into credit mode */
//			credits = 0;	/* this is a good time to reset the credits counter */
//			mode = 0;
//			break;
	}
	
	bprintf(PRINT_NORMAL, _T("IO Write %x\n"), data);

//	nmi_timer = timer_pulse (TIME_IN_USEC (50), 0, galaga_nmi_generate);
}

unsigned char __fastcall DrvGalagaRead1(unsigned short a)
{
	if (a >= 0x8000 && a <= 0x87ff) return DrvVideoRam[a - 0x8000];
	if (a >= 0x8800 && a <= 0x8bff) return DrvSharedRam1[a - 0x8800];
	if (a >= 0x9000 && a <= 0x93ff) return DrvSharedRam2[a - 0x9000];
	if (a >= 0x9800 && a <= 0x9bff) return DrvSharedRam3[a - 0x9800];
	
	switch (a) {
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			int Offset = a - 0x6800;
			
			int bit0 = (DrvDip[1] >> Offset) & 1;
			int bit1 = (DrvDip[2] >> Offset) & 1;

			return bit0 | (bit1 << 1);
		}
		
		case 0x7100: {
			return galaga_customio_r();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGalagaWrite1(unsigned short a, unsigned char d)
{
	if (a >= 0x8000 && a <= 0x87ff) {
		DrvVideoRam[a - 0x8000] = d;
		return;
	}
	if (a >= 0x8800 && a <= 0x8bff) {
		DrvSharedRam1[a - 0x8800] = d;
		return;
	}
	if (a >= 0x9000 && a <= 0x93ff) {
		DrvSharedRam2[a - 0x9000] = d;
		return;
	}
	if (a >= 0x9800 && a <= 0x9bff) {
		DrvSharedRam3[a - 0x9800] = d;
		return;
	}
	
	switch (a) {
		case 0x6822: {
			int bit = d & 1;
			if (!bit) {
//				ZetClose();
//				ZetOpen(2);
//				ZetNmi();
//				nCyclesDone[2] += ZetRun(200);
//				ZetClose();
//				ZetOpen(0);
			}
			return;
		}
		
		case 0x6823: {
			int bit = d & 1;
			if (!bit) {
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(2);
				ZetReset();
				ZetClose();
				ZetOpen(0);
				DrvCPU2Halt = 1;
				DrvCPU3Halt = 1;
			} else {
				DrvCPU2Halt = 0;
				DrvCPU3Halt = 0;
			}
			return;
		}
	
		case 0x6830: {
			// watchdog write
			return;
		}
		
		case 0x7100: {
			galaga_customio_w(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X, %x\n"), a, d, customio_command);
		}
	}
}

unsigned char __fastcall DrvGalagaPortRead1(unsigned short a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGalagaPortWrite1(unsigned short a, unsigned char d)
{
	a &= 0xff;
	
	switch (a) {		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

unsigned char __fastcall DrvGalagaRead2(unsigned short a)
{
	if (a >= 0x8000 && a <= 0x87ff) return DrvVideoRam[a - 0x8000];
	if (a >= 0x8800 && a <= 0x8bff) return DrvSharedRam1[a - 0x8800];
	if (a >= 0x9000 && a <= 0x93ff) return DrvSharedRam2[a - 0x9000];
	if (a >= 0x9800 && a <= 0x9bff) return DrvSharedRam3[a - 0x9800];
	
	switch (a) {
		case 0x6802:
		case 0x6804: {
			int Offset = a - 0x6800;
			
			int bit0 = (DrvDip[1] >> Offset) & 1;
			int bit1 = (DrvDip[2] >> Offset) & 1;

			return bit0 | (bit1 << 1);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGalagaWrite2(unsigned short a, unsigned char d)
{
	if (a >= 0x8000 && a <= 0x87ff) {
		DrvVideoRam[a - 0x8000] = d;
		return;
	}
	if (a >= 0x8800 && a <= 0x8bff) {
		DrvSharedRam1[a - 0x8800] = d;
		return;
	}
	if (a >= 0x9000 && a <= 0x93ff) {
		DrvSharedRam2[a - 0x9000] = d;
		return;
	}
	if (a >= 0x9800 && a <= 0x9bff) {
		DrvSharedRam3[a - 0x9800] = d;
		return;
	}
	
	switch (a) {
		case 0x6821: {
			int bit = d & 1;
			DrvCPU2FireIRQ = bit;
			if (!DrvCPU2FireIRQ) {
				ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			}
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

unsigned char __fastcall DrvGalagaPortRead2(unsigned short a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGalagaPortWrite2(unsigned short a, unsigned char d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

unsigned char __fastcall DrvGalagaRead3(unsigned short a)
{
	if (a >= 0x8000 && a <= 0x87ff) return DrvVideoRam[a - 0x8000];
	if (a >= 0x8800 && a <= 0x8bff) return DrvSharedRam1[a - 0x8800];
	if (a >= 0x9000 && a <= 0x93ff) return DrvSharedRam2[a - 0x9000];
	if (a >= 0x9800 && a <= 0x9bff) return DrvSharedRam3[a - 0x9800];
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGalagaWrite3(unsigned short a, unsigned char d)
{
	if (a >= 0x8000 && a <= 0x87ff) {
		DrvVideoRam[a - 0x8000] = d;
		return;
	}
	if (a >= 0x8800 && a <= 0x8bff) {
		DrvSharedRam1[a - 0x8800] = d;
		return;
	}
	if (a >= 0x9000 && a <= 0x93ff) {
		DrvSharedRam2[a - 0x9000] = d;
		return;
	}
	if (a >= 0x9800 && a <= 0x9bff) {
		DrvSharedRam3[a - 0x9800] = d;
		return;
	}
	
	switch (a) {
		case 0x6822: {
			int bit = d & 1;
			if (!bit) {
//				ZetNmi();
//				nCyclesDone[2] += ZetRun(200);
			}
//			DrvCPU3FireIRQ = !bit;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Write => %04X, %02X\n"), a, d);
		}
	}
}

unsigned char __fastcall DrvGalagaPortRead3(unsigned short a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGalagaPortWrite3(unsigned short a, unsigned char d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static int CharPlaneOffsets[2]   = { 0, 4 };
static int CharXOffsets[8]       = { 64, 65, 66, 67, 0, 1, 2, 3 };
static int CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static int SpritePlaneOffsets[2] = { 0, 4 };
static int SpriteXOffsets[16]    = { 0, 1, 2, 3, 64, 65, 66, 67, 128, 129, 130, 131, 192, 193, 194, 195 };
static int SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 256, 264, 272, 280, 288, 296, 304, 312 };

static int DrvInit()
{
	int nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (unsigned char *)malloc(0x02000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  4, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000,  5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom,            8, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x02000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x01000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x80, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,       11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  13, 1); if (nRet != 0) return 1;
	
	free(DrvTempRom);
	
	// Setup the Z80 emulation
	ZetInit(3);
	ZetOpen(0);
	ZetSetReadHandler(DrvGalagaRead1);
	ZetSetWriteHandler(DrvGalagaWrite1);
	ZetSetInHandler(DrvGalagaPortRead1);
	ZetSetOutHandler(DrvGalagaPortWrite1);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom1             );
/*	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam            );
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1          );
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2          );
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3          );*/
	ZetMapArea(0x8000, 0x9fff, 0, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 1, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 2, DrvSharedRam1          );	
	ZetMemEnd();
	ZetClose();
	
	ZetOpen(1);
	ZetSetReadHandler(DrvGalagaRead2);
	ZetSetWriteHandler(DrvGalagaWrite2);
	ZetSetInHandler(DrvGalagaPortRead2);
	ZetSetOutHandler(DrvGalagaPortWrite2);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom2             );
/*	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam            );
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1          );
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2          );
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3          );*/
	ZetMapArea(0x8000, 0x9fff, 0, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 1, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 2, DrvSharedRam1          );
	ZetMemEnd();
	ZetClose();
	
	ZetOpen(2);
	ZetSetReadHandler(DrvGalagaRead3);
	ZetSetWriteHandler(DrvGalagaWrite3);
	ZetSetInHandler(DrvGalagaPortRead3);
	ZetSetOutHandler(DrvGalagaPortWrite3);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom3             );
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom3             );
/*	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam            );
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1          );
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2          );
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3          );*/
	ZetMapArea(0x8000, 0x9fff, 0, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 1, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 2, DrvSharedRam1          );
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static int GallagInit()
{
	int nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (unsigned char *)malloc(0x02000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  4, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000,  5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom,            7, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x02000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x01000,  9, 1); if (nRet != 0) return 1;
	GfxDecode(0x80, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,       10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  12, 1); if (nRet != 0) return 1;
	
	free(DrvTempRom);
	
	// Setup the Z80 emulation
	ZetInit(3);
	ZetOpen(0);
	ZetSetReadHandler(DrvGalagaRead1);
	ZetSetWriteHandler(DrvGalagaWrite1);
	ZetSetInHandler(DrvGalagaPortRead1);
	ZetSetOutHandler(DrvGalagaPortWrite1);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom1             );
/*	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam            );
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1          );
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2          );
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3          );*/
	ZetMapArea(0x8000, 0x9fff, 0, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 1, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 2, DrvSharedRam1          );	
	ZetMemEnd();
	ZetClose();
	
	ZetOpen(1);
	ZetSetReadHandler(DrvGalagaRead2);
	ZetSetWriteHandler(DrvGalagaWrite2);
	ZetSetInHandler(DrvGalagaPortRead2);
	ZetSetOutHandler(DrvGalagaPortWrite2);
	ZetMapArea(0x0000, 0x0fff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0x0fff, 2, DrvZ80Rom2             );
/*	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam            );
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1          );
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2          );
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3          );*/
	ZetMapArea(0x8000, 0x9fff, 0, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 1, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 2, DrvSharedRam1          );
	ZetMemEnd();
	ZetClose();
	
	ZetOpen(2);
	ZetSetReadHandler(DrvGalagaRead3);
	ZetSetWriteHandler(DrvGalagaWrite3);
	ZetSetInHandler(DrvGalagaPortRead3);
	ZetSetOutHandler(DrvGalagaPortWrite3);
	ZetMapArea(0x0000, 0x0fff, 0, DrvZ80Rom3             );
	ZetMapArea(0x0000, 0x0fff, 2, DrvZ80Rom3             );
/*	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam            );
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam            );
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1          );
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1          );
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2          );
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2          );
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3          );
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3          );*/
	ZetMapArea(0x8000, 0x9fff, 0, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 1, DrvSharedRam1          );
	ZetMapArea(0x8000, 0x9fff, 2, DrvSharedRam1          );
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	ZetExit();

	GenericTilesExit();
	
	free(Mem);
	Mem = NULL;
	
	DrvCPU1FireIRQ = 0;
	DrvCPU2FireIRQ = 0;
	DrvCPU3FireIRQ = 0;
	DrvCPU2Halt = 0;
	DrvCPU3Halt = 0;

	return 0;
}

static void DrvCalcPalette()
{
	int i;
	unsigned int Palette[96];
	
	for (i = 0; i < 32; i++) {
		int bit0, bit1, bit2, r, g, b;
		
		bit0 = (DrvPromPalette[i] >> 0) & 0x01;
		bit1 = (DrvPromPalette[i] >> 1) & 0x01;
		bit2 = (DrvPromPalette[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (DrvPromPalette[i] >> 3) & 0x01;
		bit1 = (DrvPromPalette[i] >> 4) & 0x01;
		bit2 = (DrvPromPalette[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (DrvPromPalette[i] >> 6) & 0x01;
		bit2 = (DrvPromPalette[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		
		Palette[i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 64; i++) {
		int bits, r, g, b;
		static const int map[4] = { 0x00, 0x47, 0x97, 0xde };
		
		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];
		
		Palette[32 + i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[i] = Palette[((DrvPromCharLookup[i]) & 0x0f) + 0x10];
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[256 + i] = Palette[DrvPromSpriteLookup[i] & 0x0f];
	}
	
	for (i = 0; i < 64; i++) {
		DrvPalette[512 + i] = Palette[32 + i];
	}
}

static void DrvRenderTilemap()
{
	int mx, my, Code, Colour, x, y, TileIndex, xScroll, yScroll, Flip, xFlip, yFlip, Row, Col;

	for (mx = 0; mx < 36; mx++) {
		for (my = 0; my < 28; my++) {
			Row = mx + 2;
			Col = my - 2;
			if (Col & 0x20) {
				TileIndex = Row + ((Col & 0x1f) << 5);
			} else {
				TileIndex = Col + (Row << 5);
			}
			
/*			int color = galaga_videoram[tile_index + 0x400] & 0x3f;
	SET_TILE_INFO(
			0,
			(galaga_videoram[tile_index] & 0x7f) | (flip_screen_get() ? 0x80 : 0) | (galaga_gfxbank << 8),
			color,
			flip_screen_get() ? TILE_FLIPX : 0);
	tileinfo->group = color;*/
			
//			Code = DrvVideoRam[TileIndex + 0x000] & 0x7f;
//			Colour = DrvVideoRam[TileIndex + 0x400] & 0x3f;

			Code = DrvSharedRam1[TileIndex + 0x000] & 0x7f;
			Colour = DrvSharedRam1[TileIndex + 0x400] & 0x3f;
			
//			y = 8 * mx;
//			x = 8 * my;

			y = 8 * mx;
			x = 8 * my;

//			x -= xScroll;
//			if (x < -16) x += 512;
			
//			y -= yScroll;
//			if (y < -16) y += 512;
			
			y -= 16;

			if (x > 0 && x < 280 && y > 0 && y < 216) {
				Render8x8Tile(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
			} else {
				Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
			}
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	
	//Render8x8Tile_Mask(pTransDraw, 0x0a, 100, 100, 6, 2, 0, 0, DrvChars);
	//Render16x16Tile_Mask(pTransDraw, 0x34, 100, 100, 6, 2, 0, 0, DrvSprites);
	
	DrvRenderTilemap();
	
	BurnTransferCopy(DrvPalette);
}

static int DrvFrame()
{
	int nInterleave = 200;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = (18432000 / 6) / 60;
	nCyclesTotal[1] = (18432000 / 6) / 60;
	nCyclesTotal[2] = (18432000 / 6) / 60;
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = 0;
	
	ZetNewFrame();

	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == (nInterleave - 1)/* && DrvCPU1FireIRQ*/) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);//{ ZetRaiseIrq(0); DrvCPU1FireIRQ = 0; }
//		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		// Run Z80 #2
		if (!DrvCPU2Halt) {
			nCurrentCPU = 1;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			if (i == (nInterleave - 1) /*&& DrvCPU2FireIRQ*/) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);//{ ZetRaiseIrq(0); DrvCPU2FireIRQ = 0; }
			ZetClose();
		}
		
		// Run Z80 #3
		if (!DrvCPU3Halt) {
			nCurrentCPU = 2;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
//			if (i == 50 || i == 150) ZetNmi();
			ZetClose();
		}
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

static int DrvScan(int nAction, int *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	return 0;
}

struct BurnDriverD BurnDrvGalaga = {
	"galaga", NULL, NULL, "1981",
	"Galaga (Namco rev. B)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, //GBF_VERSHOOT, 0,
	NULL, DrvRomInfo, DrvRomName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	0, NULL, NULL, NULL, NULL, 224, 288, 3, 4
};

struct BurnDriverD BurnDrvGallag = {
	"gallag", "galaga", NULL, "1981",
	"Gallag\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, //GBF_VERSHOOT, 0,
	NULL, GallagRomInfo, GallagRomName, DrvInputInfo, DrvDIPInfo,
	GallagInit, DrvExit, DrvFrame, NULL, DrvScan,
	0, NULL, NULL, NULL, NULL, 224, 288, 3, 4
};
