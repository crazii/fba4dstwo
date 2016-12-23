#include "tiles_generic.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "upd7759.h"
#include "k007232.h"

static unsigned char DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInputPort3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInputPort4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvDip[3]        = {0, 0, 0};
static unsigned char DrvInput[5]      = {0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char DrvReset         = 0;

static unsigned char *Mem                 = NULL;
static unsigned char *MemEnd              = NULL;
static unsigned char *RamStart            = NULL;
static unsigned char *RamEnd              = NULL;
static unsigned char *Drv68KRom           = NULL;
static unsigned char *Drv68KRam           = NULL;
static unsigned char *DrvZ80Rom           = NULL;
static unsigned char *DrvZ80Ram           = NULL;
static unsigned char *DrvK007232Rom       = NULL;
static unsigned char *DrvUPD7759CRom      = NULL;
static unsigned char *DrvTileRom          = NULL;
static unsigned char *DrvSpriteRom        = NULL;
static unsigned char *DrvPaletteRam       = NULL;
static unsigned short *DrvNvRam            = NULL;
static unsigned char *DrvTiles            = NULL;
static unsigned char *DrvSprites          = NULL;
static unsigned char *DrvTempRom          = NULL;
static unsigned int  *DrvPalette          = NULL;
static INT16         *DrvTitleSample      = NULL;

static int nCyclesDone[2], nCyclesTotal[2];
static int nCyclesSegment;

static unsigned char bIrqEnable;
static int LayerColourBase[3];
static int SpriteColourBase;
static unsigned char DrvSoundLatch;
static int PriorityFlag;

static int DrvNvRamBank;

static int TitleSoundLatch;
static int PlayTitleSample;
double TitleSamplePos = 0;

static struct BurnInputInfo TmntInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 coin"   },
	{"Coin 3"            , BIT_DIGITAL  , DrvInputPort0 + 2, "p3 coin"   },
	{"Coin 4"            , BIT_DIGITAL  , DrvInputPort0 + 3, "p4 coin"   },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },
	
	{"P3 Up"             , BIT_DIGITAL  , DrvInputPort3 + 2, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL  , DrvInputPort3 + 3, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL  , DrvInputPort3 + 0, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL  , DrvInputPort3 + 1, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL  , DrvInputPort3 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL  , DrvInputPort3 + 5, "p3 fire 2" },
	
	{"P4 Up"             , BIT_DIGITAL  , DrvInputPort4 + 2, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL  , DrvInputPort4 + 3, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL  , DrvInputPort4 + 0, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL  , DrvInputPort4 + 1, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL  , DrvInputPort4 + 4, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL  , DrvInputPort4 + 5, "p4 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "service"   },
	{"Service 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "service2"  },
	{"Service 3"         , BIT_DIGITAL  , DrvInputPort0 + 6, "service3"  },
	{"Service 4"         , BIT_DIGITAL  , DrvInputPort0 + 7, "service4"  },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Tmnt);

static struct BurnInputInfo Tmnt2pInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "service"   },
	{"Service 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "service2"  },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Tmnt2p);

static struct BurnInputInfo MiaInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 4, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort1 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort2 + 6, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 6, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Mia);

static inline void DrvClearOpposites(unsigned char* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = DrvInput[3] = DrvInput[4] = 0x00;

	// Compile Digital Inputs
	for (int i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
		DrvInput[3] |= (DrvInputPort3[i] & 1) << i;
		DrvInput[4] |= (DrvInputPort3[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[1]);
	DrvClearOpposites(&DrvInput[2]);
	DrvClearOpposites(&DrvInput[3]);
	DrvClearOpposites(&DrvInput[4]);
}

static struct BurnDIPInfo TmntDIPList[]=
{
	// Default Values
	{0x21, 0xff, 0xff, 0xff, NULL                     },
	{0x22, 0xff, 0xff, 0x5f, NULL                     },
	{0x23, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16   , "Coinage"               },
	{0x21, 0x01, 0x0f, 0x00, "5 Coins 1 Credit"       },
	{0x21, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"       },
	{0x21, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"       },
	{0x21, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"       },
	{0x21, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"      },
	{0x21, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"      },
	{0x21, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"       },
	{0x21, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"      },
	{0x21, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"      },
	{0x21, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"      },
	{0x21, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"      },
	{0x21, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"      },
	{0x21, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"      },
	{0x21, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"      },
	{0x21, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"      },
	{0x21, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"      },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x22, 0x01, 0x03, 0x03, "1"                      },
	{0x22, 0x01, 0x03, 0x02, "2"                      },
	{0x22, 0x01, 0x03, 0x01, "3"                      },
	{0x22, 0x01, 0x03, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x22, 0x01, 0x60, 0x60, "Easy"                   },
	{0x22, 0x01, 0x60, 0x40, "Normal"                 },
	{0x22, 0x01, 0x60, 0x20, "Difficult"              },
	{0x22, 0x01, 0x60, 0x00, "Very Difficult"         },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x22, 0x01, 0x80, 0x80, "Off"                    },
	{0x22, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x23, 0x01, 0x01, 0x01, "Off"                    },
	{0x23, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x23, 0x01, 0x04, 0x04, "Off"                    },
	{0x23, 0x01, 0x04, 0x00, "On"                     },
};

STDDIPINFO(Tmnt);

static struct BurnDIPInfo Tmnt2pDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0x5f, NULL                     },
	{0x15, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16   , "Coin A"                },
	{0x13, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"      },
	{0x13, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"      },
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"      },
	{0x13, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"      },
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"      },
	{0x13, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"      },
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"      },
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"      },
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"      },
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"      },
	{0x13, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"      },
	{0x13, 0x01, 0x0f, 0x00, "Free Play"              },
	
	{0   , 0xfe, 0   , 15   , "Coin B"                },
	{0x13, 0x01, 0x0f, 0x20, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x0f, 0x50, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x0f, 0x80, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x0f, 0x40, "3 Coins 2 Credits"      },
	{0x13, 0x01, 0x0f, 0x10, "4 Coins 3 Credits"      },
	{0x13, 0x01, 0x0f, 0xf0, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x0f, 0x30, "3 Coins 4 Credits"      },
	{0x13, 0x01, 0x0f, 0x70, "2 Coins 3 Credits"      },
	{0x13, 0x01, 0x0f, 0xe0, "1 Coin  2 Credits"      },
	{0x13, 0x01, 0x0f, 0x60, "2 Coins 5 Credits"      },
	{0x13, 0x01, 0x0f, 0xd0, "1 Coin  3 Credits"      },
	{0x13, 0x01, 0x0f, 0xc0, "1 Coin  4 Credits"      },
	{0x13, 0x01, 0x0f, 0xb0, "1 Coin  5 Credits"      },
	{0x13, 0x01, 0x0f, 0xa0, "1 Coin  6 Credits"      },
	{0x13, 0x01, 0x0f, 0x90, "1 Coin  7 Credits"      },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x03, "1"                      },
	{0x14, 0x01, 0x03, 0x02, "2"                      },
	{0x14, 0x01, 0x03, 0x01, "3"                      },
	{0x14, 0x01, 0x03, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x14, 0x01, 0x60, 0x60, "Easy"                   },
	{0x14, 0x01, 0x60, 0x40, "Normal"                 },
	{0x14, 0x01, 0x60, 0x20, "Difficult"              },
	{0x14, 0x01, 0x60, 0x00, "Very Difficult"         },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x01, 0x01, "Off"                    },
	{0x15, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x15, 0x01, 0x04, 0x04, "Off"                    },
	{0x15, 0x01, 0x04, 0x00, "On"                     },
};

STDDIPINFO(Tmnt2p);

static struct BurnDIPInfo MiaDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	{0x15, 0xff, 0xff, 0x56, NULL                     },
	{0x16, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16   , "Coin A"                },
	{0x14, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"      },
	{0x14, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"      },
	{0x14, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"      },
	{0x14, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"      },
	{0x14, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"      },
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"      },
	{0x14, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"      },
	{0x14, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"      },
	{0x14, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"      },
	{0x14, 0x01, 0x0f, 0x00, "Free Play"              },
	
	{0   , 0xfe, 0   , 15   , "Coin B"                },
	{0x14, 0x01, 0x0f, 0x20, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0x0f, 0x50, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x0f, 0x80, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x0f, 0x40, "3 Coins 2 Credits"      },
	{0x14, 0x01, 0x0f, 0x10, "4 Coins 3 Credits"      },
	{0x14, 0x01, 0x0f, 0xf0, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x0f, 0x30, "3 Coins 4 Credits"      },
	{0x14, 0x01, 0x0f, 0x70, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x0f, 0xe0, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x0f, 0x60, "2 Coins 5 Credits"      },
	{0x14, 0x01, 0x0f, 0xd0, "1 Coin  3 Credits"      },
	{0x14, 0x01, 0x0f, 0xc0, "1 Coin  4 Credits"      },
	{0x14, 0x01, 0x0f, 0xb0, "1 Coin  5 Credits"      },
	{0x14, 0x01, 0x0f, 0xa0, "1 Coin  6 Credits"      },
	{0x14, 0x01, 0x0f, 0x90, "1 Coin  7 Credits"      },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0x03, 0x03, "2"                      },
	{0x15, 0x01, 0x03, 0x02, "3"                      },
	{0x15, 0x01, 0x03, 0x01, "5"                      },
	{0x15, 0x01, 0x03, 0x00, "7"                      },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x18, 0x18, "30k  80k"               },
	{0x15, 0x01, 0x18, 0x10, "50k 100k"               },
	{0x15, 0x01, 0x18, 0x08, "50k"                    },
	{0x15, 0x01, 0x18, 0x00, "100k"                   },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0x60, 0x60, "Easy"                   },
	{0x15, 0x01, 0x60, 0x40, "Normal"                 },
	{0x15, 0x01, 0x60, 0x20, "Difficult"              },
	{0x15, 0x01, 0x60, 0x00, "Very Difficult"         },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x15, 0x01, 0x80, 0x80, "Off"                    },
	{0x15, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x16, 0x01, 0x01, 0x01, "Off"                    },
	{0x16, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "VRAM Character Check"   },
	{0x16, 0x01, 0x02, 0x02, "Off"                    },
	{0x16, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x16, 0x01, 0x04, 0x04, "Off"                    },
	{0x16, 0x01, 0x04, 0x00, "On"                     },
};

STDDIPINFO(Mia);

static struct BurnRomInfo TmntRomDesc[] = {
	{ "963-x23.j17",        0x020000, 0xa9549004, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-x24.k17",        0x020000, 0xe5cc9067, BRF_ESS | BRF_PRG }, //  1
	{ "963-x21.j15",        0x010000, 0x5789cf92, BRF_ESS | BRF_PRG }, //  2
	{ "963-x22.k15",        0x010000, 0x0a74e277, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmnt);
STD_ROM_FN(Tmnt);

static struct BurnRomInfo TmntuRomDesc[] = {
	{ "963-r23.j17",        0x020000, 0xa7f61195, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-r24.k17",        0x020000, 0x661e056a, BRF_ESS | BRF_PRG }, //  1
	{ "963-r21.j15",        0x010000, 0xde047bb6, BRF_ESS | BRF_PRG }, //  2
	{ "963-r22.k15",        0x010000, 0xd86a0888, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmntu);
STD_ROM_FN(Tmntu);

static struct BurnRomInfo TmntuaRomDesc[] = {
	{ "963-j23.j17",        0x020000, 0xf77314e2, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-j24.k17",        0x020000, 0x47f662d3, BRF_ESS | BRF_PRG }, //  1
	{ "963-j21.j15",        0x010000, 0x7bee9fe8, BRF_ESS | BRF_PRG }, //  2
	{ "963-j22.k15",        0x010000, 0x2efed09f, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmntua);
STD_ROM_FN(Tmntua);

static struct BurnRomInfo TmhtRomDesc[] = {
	{ "963-f23.j17",        0x020000, 0x9cb5e461, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-f24.k17",        0x020000, 0x2d902fab, BRF_ESS | BRF_PRG }, //  1
	{ "963-f21.j15",        0x010000, 0x9fa25378, BRF_ESS | BRF_PRG }, //  2
	{ "963-f22.k15",        0x010000, 0x2127ee53, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmht);
STD_ROM_FN(Tmht);

static struct BurnRomInfo TmntjRomDesc[] = {
	{ "963_223.j17",        0x020000, 0x0d34a5ff, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963_224.k17",        0x020000, 0x2fd453f2, BRF_ESS | BRF_PRG }, //  1
	{ "963_221.j15",        0x010000, 0xfa8e25fd, BRF_ESS | BRF_PRG }, //  2
	{ "963_222.k15",        0x010000, 0xca437a4f, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmntj);
STD_ROM_FN(Tmntj);

static struct BurnRomInfo Tmht2pRomDesc[] = {
	{ "963-u23.j17",        0x020000, 0x58bec748, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-u24.k17",        0x020000, 0xdce87c8d, BRF_ESS | BRF_PRG }, //  1
	{ "963-u21.j15",        0x010000, 0xabce5ead, BRF_ESS | BRF_PRG }, //  2
	{ "963-u22.k15",        0x010000, 0x4ecc8d6b, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmht2p);
STD_ROM_FN(Tmht2p);

static struct BurnRomInfo Tmht2paRomDesc[] = {
	{ "963-_23.j17",        0x020000, 0x8698061a, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-_24.k17",        0x020000, 0x4036c075, BRF_ESS | BRF_PRG }, //  1
	{ "963-_21.j15",        0x010000, 0xddcc979c, BRF_ESS | BRF_PRG }, //  2
	{ "963-_22.k15",        0x010000, 0x71a38d27, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmht2pa);
STD_ROM_FN(Tmht2pa);

static struct BurnRomInfo Tmnt2pjRomDesc[] = {
	{ "963-123.j17",        0x020000, 0x6a3527c9, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "963-124.k17",        0x020000, 0x2c4bfa15, BRF_ESS | BRF_PRG }, //  1
	{ "963-121.j15",        0x010000, 0x4181b733, BRF_ESS | BRF_PRG }, //  2
	{ "963-122.k15",        0x010000, 0xc64eb5ff, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmnt2pj);
STD_ROM_FN(Tmnt2pj);

static struct BurnRomInfo Tmnt2poRomDesc[] = {
	{ "tmnt123.j17",        0x020000, 0x2d905183, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "tmnt124.k17",        0x020000, 0xe0125352, BRF_ESS | BRF_PRG }, //  1
	{ "tmnt21.j15",         0x010000, 0x12deeafb, BRF_ESS | BRF_PRG }, //  2
	{ "tmnt22.k15",         0x010000, 0xaec4f1c3, BRF_ESS | BRF_PRG }, //  3
	
	{ "963e20.g13",         0x008000, 0x1692a6d6, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "963a28.h27",         0x080000, 0xdb4769a8, BRF_GRA },	   //  5	Tiles
	{ "963a29.k27",         0x080000, 0x8069cd2e, BRF_GRA },	   //  6
	
	{ "963a17.h4",          0x080000, 0xb5239a44, BRF_GRA },	   //  7	Sprites
	{ "963a18.h6",          0x080000, 0xdd51adef, BRF_GRA },	   //  8
	{ "963a15.k4",          0x080000, 0x1f324eed, BRF_GRA },	   //  9
	{ "963a16.k6",          0x080000, 0xd4bd9984, BRF_GRA },	   //  10
	
	{ "963a30.g7",          0x000100, 0xabd82680, BRF_GRA },	   //  11	PROMs
	{ "963a31.g19",         0x000100, 0xf8004a1c, BRF_GRA },	   //  12
	
	{ "963a26.c13",         0x020000, 0xe2ac3063, BRF_SND },	   //  13	K007232 Samples
	
	{ "963a27.d18",         0x020000, 0x2dfd674b, BRF_SND },	   //  14	UP7759C Samples
	
	{ "963a25.d5",          0x080000, 0xfca078c7, BRF_SND },	   //  15	Title Music Sample
};

STD_ROM_PICK(Tmnt2po);
STD_ROM_FN(Tmnt2po);

static struct BurnRomInfo MiaRomDesc[] = {
	{ "808t20.h17",         0x020000, 0x6f0acb1d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "808t21.j17",         0x020000, 0x42a30416, BRF_ESS | BRF_PRG }, //  1
	
	{ "808e03.f4",          0x008000, 0x3d93a7cd, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "808e12.f28",         0x010000, 0xd62f1fde, BRF_GRA },	   //  3	Tiles
	{ "808e13.h28",         0x010000, 0x1fa708f4, BRF_GRA },	   //  4
	{ "808e22.i28",         0x010000, 0x73d758f6, BRF_GRA },	   //  5
	{ "808e23.k28",         0x010000, 0x8ff08b21, BRF_GRA },	   //  6
	
	{ "808d17.j4",          0x080000, 0xd1299082, BRF_GRA },	   //  7	Sprites
	{ "808d15.h4",          0x080000, 0x2b22a6b6, BRF_GRA },	   //  8
	
	{ "808a18.f16",         0x000100, 0xeb95aede, BRF_GRA },	   //  9	PROMs
	
	{ "808d01.d4",          0x020000, 0xfd4d37c0, BRF_SND },	   //  10	K007232 Samples
};

STD_ROM_PICK(Mia);
STD_ROM_FN(Mia);

static struct BurnRomInfo Mia2RomDesc[] = {
	{ "808s20.h17",         0x020000, 0xcaa2897f, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "808s21.j17",         0x020000, 0x3d892ffb, BRF_ESS | BRF_PRG }, //  1
	
	{ "808e03.f4",          0x008000, 0x3d93a7cd, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "808e12.f28",         0x010000, 0xd62f1fde, BRF_GRA },	   //  3	Tiles
	{ "808e13.h28",         0x010000, 0x1fa708f4, BRF_GRA },	   //  4
	{ "808e22.i28",         0x010000, 0x73d758f6, BRF_GRA },	   //  5
	{ "808e23.k28",         0x010000, 0x8ff08b21, BRF_GRA },	   //  6
	
	{ "808d17.j4",          0x080000, 0xd1299082, BRF_GRA },	   //  7	Sprites
	{ "808d15.h4",          0x080000, 0x2b22a6b6, BRF_GRA },	   //  8
	
	{ "808a18.f16",         0x000100, 0xeb95aede, BRF_GRA },	   //  9	PROMs
	
	{ "808d01.d4",          0x020000, 0xfd4d37c0, BRF_SND },	   //  10	K007232 Samples
};

STD_ROM_PICK(Mia2);
STD_ROM_FN(Mia2);

static struct BurnRomInfo CuebrickRomDesc[] = {
	{ "903d25.g12",         0x010000, 0x8d575663, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "903d24.f12",         0x010000, 0x2973625d, BRF_ESS | BRF_PRG }, //  1
	
	{ "903c29.k21",         0x010000, 0xfada986d, BRF_GRA },	   //  2	Tiles
	{ "903c27.k17",         0x010000, 0x5bd4b8e1, BRF_GRA },	   //  3
	{ "903c28.k19",         0x010000, 0x80d2bfaf, BRF_GRA },	   //  4
	{ "903c26.k15",         0x010000, 0xf808fa3d, BRF_GRA },	   //  5
	
	{ "903d23.k12",         0x010000, 0xc39fc9fd, BRF_GRA },	   //  6	Sprites
	{ "903d21.k8",          0x010000, 0x3c7bf8cd, BRF_GRA },	   //  7
	{ "903d22.k10",         0x010000, 0x95ad8591, BRF_GRA },	   //  8
	{ "903d20.k6",          0x010000, 0x2872a1bb, BRF_GRA },	   //  9
};

STD_ROM_PICK(Cuebrick);
STD_ROM_FN(Cuebrick);

static int TmntMemIndex()
{
	unsigned char *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x060000;
	DrvZ80Rom              = Next; Next += 0x008000;
	DrvK007232Rom          = Next; Next += 0x020000;
	DrvUPD7759CRom         = Next; Next += 0x020000;
	DrvTileRom             = Next; Next += 0x100000;
	DrvSpriteRom           = Next; Next += 0x200000;
	
	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x004000;
	DrvZ80Ram              = Next; Next += 0x000800;
	DrvPaletteRam          = Next; Next += 0x001000;

	RamEnd                 = Next;
	
	DrvPalette             = (unsigned int*)Next; Next += 0x00400 * sizeof(unsigned int);
	DrvTitleSample         = (INT16*)Next; Next += 0x40000 * sizeof(INT16);
	DrvTiles               = Next; Next += 0x008000 * 8 * 8;
	DrvSprites             = Next; Next += 0x004000 * 16 * 16;

	MemEnd                 = Next;

	return 0;
}

static int MiaMemIndex()
{
	unsigned char *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x040000;
	DrvZ80Rom              = Next; Next += 0x008000;
	DrvK007232Rom          = Next; Next += 0x020000;
	DrvTileRom             = Next; Next += 0x040000;
	DrvSpriteRom           = Next; Next += 0x100000;
	
	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x008000;
	DrvZ80Ram              = Next; Next += 0x000800;
	DrvPaletteRam          = Next; Next += 0x001000;

	RamEnd                 = Next;
	
	DrvPalette             = (unsigned int*)Next; Next += 0x00400 * sizeof(unsigned int);
	DrvTiles               = Next; Next += 0x002000 * 8 * 8;
	DrvSprites             = Next; Next += 0x002000 * 16 * 16;

	MemEnd                 = Next;

	return 0;
}

static int CuebrickMemIndex()
{
	unsigned char *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x020000;
	DrvTileRom             = Next; Next += 0x040000;
	DrvSpriteRom           = Next; Next += 0x040000;
	
	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x008000;
	DrvPaletteRam          = Next; Next += 0x001000;
	DrvNvRam               = (unsigned short*)Next; Next += 0x000400 * 0x20 * sizeof(unsigned short);

	RamEnd                 = Next;
	
	DrvPalette             = (unsigned int*)Next; Next += 0x00400 * sizeof(unsigned int);
	DrvTiles               = Next; Next += 0x002000 * 8 * 8;
	DrvSprites             = Next; Next += 0x001000 * 16 * 16;

	MemEnd                 = Next;

	return 0;
}

static int DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnYM2151Reset();	
	
	KonamiICReset();
	
	bIrqEnable = 0;
	DrvSoundLatch = 0;
	TitleSoundLatch = 0;
	PlayTitleSample = 0;
	TitleSamplePos = 0;
	PriorityFlag = 0;
	
	return 0;
}

static int TmntDoReset()
{
	int nRet = DrvDoReset();
	
	UPD7759Reset();
	
	UPD7759StartWrite(0);
	UPD7759ResetWrite(1);
	
	return nRet;
}

static int CuebrickDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	BurnYM2151Reset();	
	
	KonamiICReset();
	
	bIrqEnable = 0;
	DrvNvRamBank = 0;
//	DrvSoundLatch = 0;
//	TitleSoundLatch = 0;
//	PlayTitleSample = 0;
//	TitleSamplePos = 0;
//	PriorityFlag = 0;
	
	return 0;
}

unsigned char __fastcall Tmnt68KReadByte(unsigned int a)
{
	K052109WordNoA12Read(0x100000)
	K051937ByteRead(0x140000)
	K051960ByteRead(0x140400)
	
	switch (a) {
		case 0x0a0001: {
			return 0xff - DrvInput[0];
		}
		
		case 0x0a0003: {
			return 0xff - DrvInput[1];
		}
		
		case 0x0a0005: {
			return 0xff - DrvInput[2];
		}
		
		case 0x0a0007: {
			return 0xff - DrvInput[3];
		}
		
		case 0x0a0011: {
			return DrvDip[0];
		}
		
		case 0x0a0013: {
			return DrvDip[1];
		}
		
		case 0x0a0015: {
			return 0xff - DrvInput[4];
		}
		
		case 0x0a0019: {
			return DrvDip[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Tmnt68KWriteByte(unsigned int a, unsigned char d)
{
	K052109WordNoA12Write(0x100000)
	K015937ByteWrite(0x140000)
	K051960ByteWrite(0x140400)

	switch (a) {
		case 0x0a0001: {
			static int Last;
			if (Last == 0x08 && (d & 0x08) == 0) {
				ZetOpen(0);
				ZetRaiseIrq(0);
				ZetClose();
			}
			Last = d & 0x08;
			
			bIrqEnable = (d & 0x20) ? 1 : 0;
			
			K052109RMRDLine = d & 0x80;
						
			return;
		}
		
		case 0x0a0009: {
			DrvSoundLatch = d;
			return;
		}
		
		case 0x0a0011: {
			// watchdog write
			return;
		}
		
		case 0x0c0001: {
			PriorityFlag = (d & 0x0c) >> 2;
			return;
		}
		
		case 0x10e801: {
			// nop???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

unsigned short __fastcall Tmnt68KReadWord(unsigned int a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Tmnt68KWriteWord(unsigned int a, unsigned short d)
{
	K051960WordWrite(0x140400)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

unsigned char __fastcall Mia68KReadByte(unsigned int a)
{
	K052109WordNoA12Read(0x100000)
	K051937ByteRead(0x140000)
	K051960ByteRead(0x140400)
	
	switch (a) {
		case 0x0a0001: {
			return 0xff - DrvInput[0];
		}
		
		case 0x0a0003: {
			return 0xff - DrvInput[1];
		}
		
		case 0x0a0005: {
			return 0xff - DrvInput[2];
		}
		
		case 0x0a0011: {
			return DrvDip[0];
		}
		
		case 0x0a0013: {
			return DrvDip[1];
		}
		
		case 0x0a0019: {
			return DrvDip[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Mia68KWriteByte(unsigned int a, unsigned char d)
{
	K052109WordNoA12Write(0x100000)
	K015937ByteWrite(0x140000)
	K051960ByteWrite(0x140400)
	
	switch (a) {
		case 0x0a0001: {
			static int Last;
			if (Last == 0x08 && (d & 0x08) == 0) {
				ZetOpen(0);
				ZetRaiseIrq(0);
				ZetClose();
			}
			Last = d & 0x08;
			
			bIrqEnable = (d & 0x20) ? 1 : 0;
			
			K052109RMRDLine = d & 0x80;
						
			return;
		}
		
		case 0x0a0009: {
			DrvSoundLatch = d;
			return;
		}
		
		case 0x0a0011: {
			// watchdog write
			return;
		}
		
		case 0x10e801: {
			// nop???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

unsigned short __fastcall Mia68KReadWord(unsigned int a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Mia68KWriteWord(unsigned int a, unsigned short d)
{
	K051960WordWrite(0x140400)
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

unsigned char __fastcall Cuebrick68KReadByte(unsigned int a)
{
	K052109WordNoA12Read(0x100000)
	K051937ByteRead(0x140000)
	K051960ByteRead(0x140400)
	
	if (a >= 0x0b0000 && a <= 0x0b03ff) {
		return DrvNvRam[((a - 0x0b0000) >> 1) + (DrvNvRamBank * 0x200)];
	}
	
	switch (a) {
		case 0x0a0001: {
			return 0xff - DrvInput[0];
		}
		
		case 0x0a0003: {
			return 0xff - DrvInput[1];
		}
		
		case 0x0a0005: {
			return 0xff - DrvInput[2];
		}
		
		case 0x0a0011: {
			return DrvDip[1];
		}
		
		case 0x0a0013: {
			return DrvDip[0];
		}
		
		case 0x0a0019: {
			return DrvDip[2];
		}
		
		case 0x0c0000:
		case 0x0c0002: {
			return BurnYM2151ReadStatus();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Cuebrick68KWriteByte(unsigned int a, unsigned char d)
{
	K052109WordNoA12Write(0x100000)
	K015937ByteWrite(0x140000)
	K051960ByteWrite(0x140400)
	
	if (a >= 0x0b0000 && a <= 0x0b03ff) {
		DrvNvRam[((a - 0x0b0000) >> 1) + (DrvNvRamBank * 0x200)] = d;
		return;
	}
	
	switch (a) {
		case 0x0a0001: {
			bIrqEnable = (d & 0x20) ? 1 : 0;
			
			K052109RMRDLine = d & 0x80;
						
			return;
		}
		
		case 0x0a0011: {
			// watchdog write
			return;
		}
		
		case 0x0b0400: {
			DrvNvRamBank = d >> 8;
			return;
		}
		
		case 0x0c0000: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x0c0002: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

unsigned short __fastcall Cuebrick68KReadWord(unsigned int a)
{
	if (a >= 0x0b0000 && a <= 0x0b03ff) {
		return DrvNvRam[((a - 0x0b0000) >> 1) + (DrvNvRamBank * 0x200)];
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Cuebrick68KWriteWord(unsigned int a, unsigned short d)
{
	K051960WordWrite(0x140400)
	
	if (a >= 0x0b0000 && a <= 0x0b03ff) {
		DrvNvRam[((a - 0x0b0000) >> 1) + (DrvNvRamBank * 0x200)] = d;
		return;
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

unsigned char __fastcall TmntZ80Read(unsigned short a)
{
	if (a >= 0xb000 && a <= 0xb00d) {
		return K007232ReadReg(a - 0xb000);
	}
	
	switch (a) {
		case 0x9000: {
			return TitleSoundLatch;
		}
		
		case 0xa000: {
			return DrvSoundLatch;
		}
		
		case 0xc001: {
			return BurnYM2151ReadStatus();
		}
		
		case 0xf000: {
			return UPD7759BusyRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall TmntZ80Write(unsigned short a, unsigned char d)
{
	if (a >= 0xb000 && a <= 0xb00d) {
		K007232WriteReg((a - 0xb000), d);
		return;
	}
	
	switch (a) {
		case 0x9000: {
			TitleSoundLatch = d;
			if (d & 0x04) {
				PlayTitleSample = 1;
			} else {
				PlayTitleSample = 0;
				TitleSamplePos = 0;
			}
			
			UPD7759ResetWrite(d & 2);
			return;
		}
		
		case 0xc000: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0xc001: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0xd000: {
			UPD7759PortWrite(d);
			return;
		}
		
		case 0xe000: {
			UPD7759StartWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

unsigned char __fastcall MiaZ80Read(unsigned short a)
{
	if (a >= 0xb000 && a <= 0xb00d) {
		return K007232ReadReg(a - 0xb000);
	}
	
	switch (a) {
		case 0xa000: {
			return DrvSoundLatch;
		}
		
		case 0xc001: {
			return BurnYM2151ReadStatus();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall MiaZ80Write(unsigned short a, unsigned char d)
{
	if (a >= 0xb000 && a <= 0xb00d) {
		K007232WriteReg((a - 0xb000), d);
		return;
	}
	
	switch (a) {
		case 0xc000: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0xc001: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void shuffle(UINT16 *buf, int len)
{
	int i;
	UINT16 t;

	if (len == 2) return;

	if (len % 4) return;

	len /= 2;

	for (i = 0;i < len/2;i++) {
		t = buf[len/2 + i];
		buf[len/2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	shuffle(buf,len);
	shuffle(buf + len,len);
}

static void byte_shuffle(UINT8 *buf, int len)
{
	int i;
	UINT8 t;

	if (len == 2) return;

	if (len % 4) return;

	len /= 2;

	for (i = 0;i < len/2;i++) {
		t = buf[len/2 + i];
		buf[len/2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	byte_shuffle(buf,len);
	byte_shuffle(buf + len,len);
}

static void TmntUnscrambleGfx(unsigned char *pSrc, int nLength)
{
	int bits[32];
	for (int i = 0; i < nLength; i += 4) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 8; k++) {
				bits[8*j + k] = (pSrc[i + j] >> k) & 1;
			}
		}

		for (int j = 0; j < 4; j++) {
			pSrc[i + j] = 0;
			for (int k = 0; k < 8; k++) {
				pSrc[i + j] |= bits[j + 4*k] << k;
			}
		}
	}
}

static void TmntUnscrambleSprites()
{
	BurnLoadRom(DrvTempRom + 0x200000, 11, 1);
	unsigned char *SpriteConvTable = DrvTempRom + 0x200000;
	
	memcpy(DrvTempRom, DrvSpriteRom, 0x200000);
	
	for (int A = 0; A < 0x80000; A++) {
		int B, i, entry;
		int bits[10];

		static const UINT8 bit_pick_table[10][8] = {
			{ 3,   3,   3,   3,   3,   3,   3,   3 },
			{ 0,   0,   5,   5,   5,   5,   5,   5 },
			{ 1,   1,   0,   0,   0,   7,   7,   7 },
			{ 2,   2,   1,   1,   1,   0,   0,   9 },
			{ 4,   4,   2,   2,   2,   1,   1,   0 },
			{ 5,   6,   4,   4,   4,   2,   2,   1 },
			{ 6,   5,   6,   6,   6,   4,   4,   2 },
			{ 7,   7,   7,   7,   8,   6,   6,   4 },
			{ 8,   8,   8,   8,   7,   8,   8,   6 },
			{ 9,   9,   9,   9,   9,   9,   9,   8 }
		};

		entry = SpriteConvTable[(A & 0x7f800) >> 11] & 7;

		for (i = 0;i < 10;i++)
			bits[i] = (A >> i) & 0x01;

		B = A & 0x7fc00;

		for (i = 0; i < 10;i++) B |= bits[bit_pick_table[i][entry]] << i;

		DrvSpriteRom[4*A+0] = DrvTempRom[4*B+0];
		DrvSpriteRom[4*A+1] = DrvTempRom[4*B+1];
		DrvSpriteRom[4*A+2] = DrvTempRom[4*B+2];
		DrvSpriteRom[4*A+3] = DrvTempRom[4*B+3];
	}
}

static void MiaUnscrambleSprites()
{
	memcpy(DrvTempRom, DrvSpriteRom, 0x100000);
	
	for (int A = 0; A < 0x40000; A++) {
		int B, i;
		int bits[8];
		for (i = 0; i < 8; i++) bits[i] = (A >> i) & 0x01;

		B = A & 0x3ff00;

		if ((A & 0x3c000) == 0x3c000) {
			B |= bits[3] << 0;
			B |= bits[5] << 1;
			B |= bits[0] << 2;
			B |= bits[1] << 3;
			B |= bits[2] << 4;
			B |= bits[4] << 5;
			B |= bits[6] << 6;
			B |= bits[7] << 7;
		} else {
			B |= bits[3] << 0;
			B |= bits[5] << 1;
			B |= bits[7] << 2;
			B |= bits[0] << 3;
			B |= bits[1] << 4;
			B |= bits[2] << 5;
			B |= bits[4] << 6;
			B |= bits[6] << 7;
		}

		DrvSpriteRom[4*A+0] = DrvTempRom[4*B+0];
		DrvSpriteRom[4*A+1] = DrvTempRom[4*B+1];
		DrvSpriteRom[4*A+2] = DrvTempRom[4*B+2];
		DrvSpriteRom[4*A+3] = DrvTempRom[4*B+3];
	}
}

static void TmntDecodeTitleSample()
{
	for (int i = 0; i < 0x40000; i++) {
		int val = DrvTempRom[2 * i + 0] + (DrvTempRom[(2 * i) + 1] << 8);
		int expo = val >> 13;

	  	val = (val >> 3) & (0x3ff);
		val -= 0x200;

		val <<= (expo-3);

		DrvTitleSample[i] = val;
	}
}

static int TilePlaneOffsets[4]     = { 24, 16, 8, 0 };
static int TileXOffsets[8]         = { 0, 1, 2, 3, 4, 5, 6, 7 };
static int TileYOffsets[8]         = { 0, 32, 64, 96, 128, 160, 192, 224 };
static int SpritePlaneOffsets[4]   = { 24, 16, 8, 0 };
static int SpriteXOffsets[16]      = { 0, 1, 2, 3, 4, 5, 6, 7, 256, 257, 258, 259, 260, 261, 262, 263 };
static int SpriteYOffsets[16]      = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };

static void K052109TmntCallback(int Layer, int Bank, int *Code, int *Colour, int* /*xFlip*/)
{
	*Code |= ((*Colour & 0x03) << 8) | ((*Colour & 0x10) << 6) | ((*Colour & 0x0c) << 9) | (Bank << 13);
	*Colour = LayerColourBase[Layer] + ((*Colour & 0xe0) >> 5);
}

static void K052109MiaCallback(int Layer, int Bank, int *Code, int *Colour, int *xFlip)
{
	*xFlip = *Colour & 0x04;
	
	if (Layer == 0) {
		*Code |= ((*Colour & 0x01) << 8);
		*Colour = LayerColourBase[Layer] + ((*Colour & 0x80) >> 5) + ((*Colour & 0x10) >> 1);
	} else {
		*Code |= ((*Colour & 0x01) << 8) | ((*Colour & 0x18) << 6) | (Bank << 11);
		*Colour = LayerColourBase[Layer] + ((*Colour & 0xe0) >> 5);
	}
}

static void K052109CuebrickCallback(int Layer, int /*Bank*/, int *Code, int *Colour, int */*xFlip*/)
{
	if (K052109RMRDLine == 0 && Layer == 0) {
		*Code |= ((*Colour & 0x01) << 8);
		*Colour = LayerColourBase[Layer] + ((*Colour & 0x80) >> 5) + ((*Colour & 0x10) >> 1);
	} else {
		*Code |= ((*Colour & 0x0f) << 8);
		*Colour = LayerColourBase[Layer] + ((*Colour & 0xe0) >> 5);
	}
}

static void K051960TmntCallback(int *Code, int *Colour, int* /*Priority*/, int* /*Shadow*/)
{
	*Code |= (*Colour & 0x10) << 9;
	*Colour = SpriteColourBase + (*Colour & 0x0f);
}

static void K051960MiaCallback(int* /*Code*/, int *Colour, int* /*Priority*/, int* /*Shadow*/)
{
	*Colour = SpriteColourBase + (*Colour & 0x0f);
}

static void DrvK007232VolCallback(int v)
{
	K007232SetVolume(0, (v >> 4) * 0x11, 0);
	K007232SetVolume(1, 0, (v & 0x0f) * 0x11);
}

static int CuebrickSndIrqFire;

static void CuebrickYM2151IrqHandler(int Irq)
{
//	if (Irq) {
		//SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
//	} else {
//		SekSetIRQLine(6, SEK_IRQSTATUS_NONE);
//	}

	CuebrickSndIrqFire = Irq;
}

static int TmntInit()
{
	int nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	TmntMemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	TmntMemIndex();
	
	K052109Init(DrvTileRom, 0x0fffff);
	K052109SetCallback(K052109TmntCallback);
	K051960Init(DrvSpriteRom, 0x1fffff);
	K051960SetCallback(K051960TmntCallback);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x040001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x040000, 3, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTileRom + 0x000000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x080000, 6, 1); if (nRet != 0) return 1;
	shuffle((UINT16*)DrvTileRom, 0x080000);
	TmntUnscrambleGfx(DrvTileRom, 0x100000);
	GfxDecode(0x100000 / 32, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTileRom, DrvTiles);
	
	// Load the sprites
	DrvTempRom = (unsigned char *)malloc(0x200100);
	nRet = BurnLoadRom(DrvSpriteRom + 0x000000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x080000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x100000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x180000, 10, 1); if (nRet != 0) return 1;
	shuffle((UINT16*)DrvSpriteRom, 0x100000);
	TmntUnscrambleGfx(DrvSpriteRom, 0x200000);
	TmntUnscrambleSprites();
	
	// Decode the sprites
	GfxDecode(0x200000 / 128, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvSpriteRom, DrvSprites);	
	
	// Load K007232 Sample Roms
	nRet = BurnLoadRom(DrvK007232Rom + 0x00000, 13, 1); if (nRet != 0) return 1;	
	
	// Load UPD7759C Sample Roms
	nRet = BurnLoadRom(DrvUPD7759CRom + 0x00000, 14, 1); if (nRet != 0) return 1;
	
	// Load title sample
	memset(DrvTempRom, 0, 0x080000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 15, 1); if (nRet != 0) return 1;
	TmntDecodeTitleSample();
	
	free(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x05ffff, SM_ROM);
	SekMapMemory(Drv68KRam           , 0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x080000, 0x080fff, SM_RAM);
	SekSetReadWordHandler(0, Tmnt68KReadWord);
	SekSetWriteWordHandler(0, Tmnt68KWriteWord);
	SekSetReadByteHandler(0, Tmnt68KReadByte);
	SekSetWriteByteHandler(0, Tmnt68KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(1);
	ZetOpen(0);
	ZetSetReadHandler(TmntZ80Read);
	ZetSetWriteHandler(TmntZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom                );
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	
	K007232Init(3579545, DrvK007232Rom, 0x20000);
	K007232SetPortWriteHandler(DrvK007232VolCallback);
	
	UPD7759Init(UPD7759_STANDARD_CLOCK, DrvUPD7759CRom);
	
	GenericTilesInit();
	
	LayerColourBase[0] = 0;
	LayerColourBase[1] = 32;
	LayerColourBase[2] = 40;
	SpriteColourBase = 16;

	// Reset the driver
	TmntDoReset();

	return 0;
}

static int MiaInit()
{
	int nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MiaMemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MiaMemIndex();
	
	K052109Init(DrvTileRom, 0x03ffff);
	K052109SetCallback(K052109MiaCallback);
	K051960Init(DrvSpriteRom, 0x0fffff);
	K051960SetCallback(K051960MiaCallback);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000000, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTileRom + 0x000000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x020000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x020001, 6, 2); if (nRet != 0) return 1;
	shuffle((UINT16*)DrvTileRom, 0x020000);
	TmntUnscrambleGfx(DrvTileRom, 0x040000);
	GfxDecode(0x2000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTileRom, DrvTiles);
	
	// Load the sprites
	DrvTempRom = (unsigned char *)malloc(0x100000);
	nRet = BurnLoadRom(DrvSpriteRom + 0x000000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x080000,  8, 1); if (nRet != 0) return 1;
	shuffle((UINT16*)DrvSpriteRom, 0x080000);
	TmntUnscrambleGfx(DrvSpriteRom, 0x100000);
	MiaUnscrambleSprites();
	
	// Decode the sprites
	GfxDecode(0x2000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvSpriteRom, DrvSprites);	
	
	// Load K007232 Sample Roms
	nRet = BurnLoadRom(DrvK007232Rom + 0x00000, 10, 1); if (nRet != 0) return 1;
	
	free(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRam + 0x0000  , 0x040000, 0x043fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x4000  , 0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x080000, 0x080fff, SM_RAM);
	SekSetReadWordHandler(0, Mia68KReadWord);
	SekSetWriteWordHandler(0, Mia68KWriteWord);
	SekSetReadByteHandler(0, Mia68KReadByte);
	SekSetWriteByteHandler(0, Mia68KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(1);
	ZetOpen(0);
	ZetSetReadHandler(MiaZ80Read);
	ZetSetWriteHandler(MiaZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom                );
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	
	K007232Init(3579545, DrvK007232Rom, 0x20000);
	K007232SetPortWriteHandler(DrvK007232VolCallback);
	
	GenericTilesInit();
	
	LayerColourBase[0] = 0;
	LayerColourBase[1] = 32;
	LayerColourBase[2] = 40;
	SpriteColourBase = 16;

	// Reset the driver
	DrvDoReset();

	return 0;
}

static int CuebrickInit()
{
	int nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	CuebrickMemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	CuebrickMemIndex();
	
	K052109Init(DrvTileRom, 0x03ffff);
	K052109SetCallback(K052109CuebrickCallback);
	K051960Init(DrvSpriteRom, 0x03ffff);
	K051960SetCallback(K051960MiaCallback);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000000, 1, 2); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTileRom + 0x000000, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x000001, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x020000, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTileRom + 0x020001, 5, 2); if (nRet != 0) return 1;
	byte_shuffle(DrvTileRom, 0x040000);
	GfxDecode(0x2000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTileRom, DrvTiles);
	
	// Load the sprites
	nRet = BurnLoadRom(DrvSpriteRom + 0x000000, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x000001, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x020000, 8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSpriteRom + 0x020001, 9, 2); if (nRet != 0) return 1;
	byte_shuffle(DrvSpriteRom, 0x040000);
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvSpriteRom, DrvSprites);	
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(Drv68KRam + 0x0000  , 0x040000, 0x043fff, SM_RAM);
//	SekMapMemory(Drv68KRam + 0x4000  , 0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x080000, 0x080fff, SM_RAM);
	SekSetReadWordHandler(0, Cuebrick68KReadWord);
	SekSetWriteWordHandler(0, Cuebrick68KWriteWord);
	SekSetReadByteHandler(0, Cuebrick68KReadByte);
	SekSetWriteByteHandler(0, Cuebrick68KWriteByte);
	SekClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	BurnYM2151SetIrqHandler(&CuebrickYM2151IrqHandler);
	
	GenericTilesInit();
	
	LayerColourBase[0] = 0;
	LayerColourBase[1] = 32;
	LayerColourBase[2] = 40;
	SpriteColourBase = 16;

	// Reset the driver
	CuebrickDoReset();

	return 0;
}

static int DrvExit()
{
	SekExit();
	ZetExit();
	
	BurnYM2151Exit();
	UPD7759Exit();
	K007232Exit();
	
	GenericTilesExit();
	
	KonamiICExit();
	
	free(Mem);
	Mem = NULL;
	
	bIrqEnable = 0;
	DrvSoundLatch = 0;
	TitleSoundLatch = 0;
	PlayTitleSample = 0;
	TitleSamplePos = 0;
	memset(LayerColourBase, 0, 3);
	SpriteColourBase = 0;
	PriorityFlag = 0;
	
	DrvNvRamBank = 0;

	return 0;
}

static inline unsigned char pal5bit(unsigned char bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

static void DrvCalcPalette()
{
	for (int i = 0; i < 0x800; i += 2) {
		int Offset = i & ~1;
		UINT16 *PaletteRam = (UINT16*)DrvPaletteRam;
		UINT32 Data = (PaletteRam[Offset] << 8) | PaletteRam[Offset + 1];
		
		DrvPalette[Offset >> 1] = BurnHighCol(pal5bit(Data >> 0), pal5bit(Data >> 5), pal5bit(Data >> 10), 0);
	}
}
static void TmntDraw()
{
	//BurnTransferClear();
	DrvCalcPalette();

	K052109UpdateScroll();
	
	K052109RenderLayer(2, 1, DrvTiles);
	if ((PriorityFlag & 1) == 1) K051960SpritesRender(DrvSprites);
	K052109RenderLayer(1, 0, DrvTiles);
	if ((PriorityFlag & 1) == 0) K051960SpritesRender(DrvSprites);
	K052109RenderLayer(0, 0, DrvTiles);
	
	BurnTransferCopy(DrvPalette);
}

static void RenderTitleSample(short *pSoundBuf, int nLength)
{
	double Addr = TitleSamplePos;
	double Step = (double)20000 / nBurnSoundRate;
	
	for (int i = 0; i < nLength; i += 2) {
		if (Addr > 0x3ffff) break;
		INT16 Sample = DrvTitleSample[(int)Addr];
		
		pSoundBuf[i + 0] += Sample;
		pSoundBuf[i + 1] += Sample;
		
		Addr += Step;
	}
	
	TitleSamplePos = Addr;
}
#define nInterleave 368
#define nSegmentLength 1
#define nCyclesTotalSek 8000000/60						
#define nCyclesTotalZet 3579545/60
static int TmntFrame()
{
	//int nInterleave = 128;
	

	if (DrvReset) TmntDoReset();

	DrvMakeInputs();

	int nCyclesDoneSek=0;
	int nCyclesDoneZet=0;
	

	int nSoundBufferPos = 0;
	
	SekNewFrame();
	ZetNewFrame();
	SekOpen(0);
	ZetOpen(0);
	for (int i = 1; i <= nInterleave; i++) {

		// Run 68000

		nCyclesDoneSek += SekRun(i * (nCyclesTotalSek / nInterleave) - nCyclesDoneSek);
		
		
		// Run Z80
		
		nCyclesDoneZet += ZetRun(i* (nCyclesTotalZet / nInterleave) - nCyclesDoneZet);
		
		
		//if (pBurnSoundOut) 
		{
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(pSoundBuf, nSegmentLength);
			UPD7759Update(pSoundBuf, nSegmentLength);
			if (PlayTitleSample) RenderTitleSample(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	SekClose();
	ZetClose();
	if ( bIrqEnable) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int leftLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (leftLength>0) {
			BurnYM2151Render(pSoundBuf, leftLength);
			K007232Update(pSoundBuf, leftLength);
			UPD7759Update(pSoundBuf, leftLength);
			if (PlayTitleSample) RenderTitleSample(pSoundBuf, leftLength);
		}
	}
	
	if (pBurnDraw) TmntDraw();

	return 0;
}

static int MiaFrame()
{
	//int nInterleave = 256;
	int nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 8000000 / 60;
	nCyclesTotal[1] = 3579545 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();
	
	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1) && bIrqEnable) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();
		
		if (pBurnSoundOut) {
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int leftLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (leftLength) {
			BurnYM2151Render(pSoundBuf, leftLength);
			K007232Update(pSoundBuf, leftLength);
		}
	}
	
	if (pBurnDraw) TmntDraw();

	return 0;
}

static int CuebrickFrame()
{
	//int nInterleave = 10;
	int nSoundBufferPos = 0;

	if (DrvReset) CuebrickDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 8000000 / 60;
	nCyclesDone[0] = 0;

	SekNewFrame();
	SekOpen(0);
	
	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;		
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1) && bIrqEnable) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		if (CuebrickSndIrqFire) SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
				
		if (pBurnSoundOut) {
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int leftLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (leftLength) {
			BurnYM2151Render(pSoundBuf, leftLength);
		}
	}
	
	SekClose();
	
	if (pBurnDraw) TmntDraw();

	return 0;
}

static int DrvScan(int nAction, int *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029693;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	KonamiICScan(nAction);
	
	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		
		BurnYM2151Scan(nAction);		

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
		SCAN_VAR(bIrqEnable);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(TitleSoundLatch);
		SCAN_VAR(PlayTitleSample);
		SCAN_VAR(TitleSamplePos);
		SCAN_VAR(PriorityFlag);
	}

	return 0;
}

static int TmntScan(int nAction, int *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		K007232Scan(nAction, pnMin);
		UPD7759Scan(nAction, pnMin);
	}
	
	return DrvScan(nAction, pnMin);
}

static int MiaScan(int nAction, int *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		K007232Scan(nAction, pnMin);
	}
	
	return DrvScan(nAction, pnMin);
}

static int CuebrickScan(int nAction, int *pnMin)
{
	struct BurnArea ba;
	
	if (nAction & ACB_NVRAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data = DrvNvRam;
		ba.nLen = 0x400 * 0x20 * sizeof(UINT16);
		ba.szName = "NV RAM";
		BurnAcb(&ba);
	}

	return DrvScan(nAction, pnMin);
}

struct BurnDriver BurnDrvTmnt = {
	"tmnt", NULL, NULL, "1989",
	"Teenage Mutant Ninja Turtles (World 4 Players)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, TmntRomInfo, TmntRomName, TmntInputInfo, TmntDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmntu = {
	"tmntu", "tmnt", NULL, "1989",
	"Teenage Mutant Ninja Turtles (US 4 Players, set 1)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, TmntuRomInfo, TmntuRomName, TmntInputInfo, TmntDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmntua = {
	"tmntua", "tmnt", NULL, "1989",
	"Teenage Mutant Ninja Turtles (US 4 Players, set 2)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, TmntuaRomInfo, TmntuaRomName, TmntInputInfo, TmntDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmht = {
	"tmht", "tmnt", NULL, "1989",
	"Teenage Mutant Hero Turtles (UK 4 Players)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, TmhtRomInfo, TmhtRomName, TmntInputInfo, TmntDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmntj = {
	"tmntj", "tmnt", NULL, "1990",
	"Teenage Mutant Ninja Turtles (Japan 4 Players)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, TmntjRomInfo, TmntjRomName, TmntInputInfo, TmntDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmht2p = {
	"tmht2p", "tmnt", NULL, "1989",
	"Teenage Mutant Hero Turtles (UK 2 Players, set 1)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, Tmht2pRomInfo, Tmht2pRomName, Tmnt2pInputInfo, Tmnt2pDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmht2pa = {
	"tmht2pa", "tmnt", NULL, "1989",
	"Teenage Mutant Hero Turtles (UK 2 Players, set 2)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, Tmht2paRomInfo, Tmht2paRomName, Tmnt2pInputInfo, Tmnt2pDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmht2pj = {
	"tmnt2pj", "tmnt", NULL, "1990",
	"Teenage Mutant Ninja Turtles (Japan 2 Players)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, Tmnt2pjRomInfo, Tmnt2pjRomName, Tmnt2pInputInfo, Tmnt2pDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvTmht2po = {
	"tmnt2po", "tmnt", NULL, "1989",
	"Teenage Mutant Ninja Turtles (Oceania 2 Players)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KONAMI_68K_Z80, //GBF_SCRFIGHT, 0,
	NULL, Tmnt2poRomInfo, Tmnt2poRomName, Tmnt2pInputInfo, Tmnt2pDIPInfo,
	TmntInit, DrvExit, TmntFrame, NULL, TmntScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvMia = {
	"mia", NULL, NULL, "1989",
	"M.I.A. - Missing in Action (version T)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KONAMI_68K_Z80, //GBF_PLATFORM, 0,
	NULL, MiaRomInfo, MiaRomName, MiaInputInfo, MiaDIPInfo,
	MiaInit, DrvExit, MiaFrame, NULL, MiaScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvMia2 = {
	"mia2", "mia", NULL, "1989",
	"M.I.A. - Missing in Action (version S)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KONAMI_68K_Z80, //GBF_PLATFORM, 0,
	NULL, Mia2RomInfo, Mia2RomName, MiaInputInfo, MiaDIPInfo,
	MiaInit, DrvExit, MiaFrame, NULL, MiaScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};

struct BurnDriver BurnDrvCuebrick = {
	"cuebrick", NULL, NULL, "1989",
	"Cue Brick (World version D)\0", NULL, "Konami", "Konami",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KONAMI_68K_Z80, //GBF_PLATFORM, 0,
	NULL, CuebrickRomInfo, CuebrickRomName, MiaInputInfo, MiaDIPInfo,
	CuebrickInit, DrvExit, CuebrickFrame, NULL, CuebrickScan,
	0, NULL, NULL, NULL, NULL, 304, 224, 4, 3
};
