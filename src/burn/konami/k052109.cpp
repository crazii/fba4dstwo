// K052109

#include "tiles_generic.h"
#include "konamiic.h"

static unsigned char *K052109Ram = NULL;

typedef void (*K052109_Callback)(int Layer, int Bank, int *Code, int *Colour, int *xFlip);
static K052109_Callback K052109Callback;
static int K052109ScrollX[3];
static int K052109ScrollY[3];
static unsigned char K052109ScrollCtrl;
static unsigned char K052109CharRomBank[4];
int K052109RMRDLine;
static unsigned char K052109RomSubBank;
static unsigned int K052109RomMask;
static unsigned char *K052109Rom;
static int K052109FlipEnable;

void K052109UpdateScroll()
{
	if ((K052109ScrollCtrl & 0x03) == 0x02) {
		bprintf(PRINT_IMPORTANT, _T("K052109 Tilemap 1 Row scroll mode 1\n"));
	} else {
		if ((K052109ScrollCtrl & 0x03) == 0x03) {
			bprintf(PRINT_IMPORTANT, _T("K052109 Tilemap 1 Row scroll mode 2\n"));
		} else {
			if ((K052109ScrollCtrl & 0x04) == 0x04) {
				bprintf(PRINT_IMPORTANT, _T("K052109 Tilemap 1 Col scroll\n"));
			} else {
				K052109ScrollX[1] = K052109Ram[0x1a00] + (K052109Ram[0x1a01] << 8) - 6;
				K052109ScrollY[1] = K052109Ram[0x180c];
			}
		}
	}
	
	if ((K052109ScrollCtrl & 0x18) == 0x10) {
		bprintf(PRINT_IMPORTANT, _T("K052109 Tilemap 2 Row scroll mode 1\n"));
	} else {
		if ((K052109ScrollCtrl & 0x18) == 0x18) {
			bprintf(PRINT_IMPORTANT, _T("K052109 Tilemap 2 Row scroll mode 2\n"));
		} else {
			if ((K052109ScrollCtrl & 0x20) == 0x20) {
				bprintf(PRINT_IMPORTANT, _T("K052109 Tilemap 2 Col scroll\n"));
			} else {
				K052109ScrollX[2] = K052109Ram[0x3a00] + (K052109Ram[0x3a01] << 8) - 6;
				K052109ScrollY[2] = K052109Ram[0x380c];
			}
		}
	}		
}

void K052109RenderLayer(int nLayer, int Opaque, unsigned char *pSrc)
{
	int mx, my, Bank, Code, Colour, x, y, xFlip = 0, yFlip, TileIndex = 0;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Colour = K052109Ram[TileIndex + 0x0000];
			Code = K052109Ram[TileIndex + 0x2000] + (K052109Ram[TileIndex + 0x4000] << 8);
			
			if (nLayer == 1) {
				Colour = K052109Ram[TileIndex + 0x0800];
				Code = K052109Ram[TileIndex + 0x2800] + (K052109Ram[TileIndex + 0x4800] << 8);
			}
			
			if (nLayer == 2) {
				Colour = K052109Ram[TileIndex + 0x1000];
				Code = K052109Ram[TileIndex + 0x3000] + (K052109Ram[TileIndex + 0x5000] << 8);
			}
			
			Bank = K052109CharRomBank[(Colour & 0x0c) >> 2];
						
			Colour = (Colour & 0xf3) | ((Bank & 0x03) << 2);
			Bank >>= 2;
			
			yFlip = Colour & 0x02;
			
			K052109Callback(nLayer, Bank, &Code, &Colour, &xFlip);
			
			if (xFlip && !(K052109FlipEnable & 1)) xFlip = 0;
			if (yFlip && !(K052109FlipEnable & 2)) yFlip = 0;

			x = 8 * mx;
			y = 8 * my;
			
			x -= K052109ScrollX[nLayer] & 0x1ff;
			y -= K052109ScrollY[nLayer] & 0xff;
			if (x < -8) x += 512;
			if (y < -8) y += 256;
			
			x -= 104;
			y -= 16;
			
			if (Opaque) {
				if (x >= 0 && x < (nScreenWidth - 8) && y >= 0 && y <= (nScreenHeight - 8)) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					}
				}
			} else {
				if (x >= 0 && x < (nScreenWidth - 8) && y >= 0 && y <= (nScreenHeight - 8)) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

unsigned char K052109Read(unsigned int Offset)
{
	if (K052109RMRDLine) {
		int Code = (Offset & 0x1fff) >> 5;
		int Colour = K052109RomSubBank;
		int Bank = K052109CharRomBank[(Colour & 0x0c) >> 2] >> 2;
		int Addr;
		
		K052109Callback(0, Bank, &Code, &Colour, 0);
		
		Addr = (Code << 5) + (Offset & 0x1f);
		Addr &= K052109RomMask;
		
		return K052109Rom[Addr];
	}
	
	return K052109Ram[Offset];
}

void K052109Write(unsigned int Offset, unsigned char Data)
{
	K052109Ram[Offset] = Data;
	
	if ((Offset & 0x1fff) >= 0x1800) {
		switch (Offset) {
			case 0x1c80: {
				K052109ScrollCtrl = Data;
				return;
			}
			
			case 0x1d80: {
				K052109CharRomBank[0] = Data & 0x0f;
				K052109CharRomBank[1] = (Data >> 4) & 0x0f;
				return;
			}
			
			case 0x1e00: {
				K052109RomSubBank = Data;
				return;
			}
			
			case 0x1e80: {
				// flip
				K052109FlipEnable = ((Data & 0x06) >> 1);
				return;
			}
			
			case 0x1f00: {
				K052109CharRomBank[2] = Data & 0x0f;
				K052109CharRomBank[3] = (Data >> 4) & 0x0f;
				return;
			}
			
			case 0x180c:
			case 0x180d:
			case 0x1a00:
			case 0x1a01:
			case 0x380c:
			case 0x380d:
			case 0x3a00:
			case 0x3a01: {
				// Scroll Writes
				return;
			}
			
			case 0x1c00: {
				//???
				return;
			}
		}
		bprintf(PRINT_NORMAL, _T("K052109 Write %x, %x\n"), Offset, Data);
	}
}

void K052109SetCallback(void (*Callback)(int Layer, int Bank, int *Code, int *Colour, int *xFlip))
{
	K052109Callback = Callback;
}

void K052109Reset()
{
	memset(K052109ScrollX, 0, 3);
	memset(K052109ScrollY, 0, 3);
	K052109ScrollCtrl = 0;
	memset(K052109CharRomBank, 0, 4);
	K052109RMRDLine = 0;
	K052109RomSubBank = 0;
}

void K052109Init(unsigned char *pRomSrc, unsigned int RomMask)
{
	K052109Ram = (unsigned char*)malloc(0x6000);
	
	K052109RomMask = RomMask;
	
	K052109Rom = pRomSrc;
	
	KonamiIC_K052109InUse = 1;
}

void K052109Exit()
{
	free(K052109Ram);
	K052109Ram = NULL;
	
	K052109Callback = NULL;
	K052109RomMask = 0;
	K052109Rom = NULL;
	
	memset(K052109ScrollX, 0, 3);
	memset(K052109ScrollY, 0, 3);
	K052109ScrollCtrl = 0;
	memset(K052109CharRomBank, 0, 4);
	K052109RMRDLine = 0;
	K052109RomSubBank = 0;
	K052109FlipEnable = 0;
}

void K052109Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K052109Ram;
		ba.nLen	  = 0x6000;
		ba.szName = "K052109 Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K052109ScrollX);
		SCAN_VAR(K052109ScrollY);
		SCAN_VAR(K052109ScrollCtrl);
		SCAN_VAR(K052109CharRomBank);
		SCAN_VAR(K052109RMRDLine);
		SCAN_VAR(K052109RomSubBank);
		SCAN_VAR(K052109FlipEnable);
	}
}
