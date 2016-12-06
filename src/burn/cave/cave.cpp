#include "cave.h"

int nCaveXSize = 0, nCaveYSize = 0;
int nCaveXOffset = 0, nCaveYOffset = 0;
int nCaveExtraXOffset = 0, nCaveExtraYOffset = 0;
int nCaveRowModeOffset = 0;

int CaveScanGraphics()
{
	SCAN_VAR(nCaveXOffset);
	SCAN_VAR(nCaveYOffset);

	SCAN_VAR(nCaveTileBank);

	SCAN_VAR(nCaveSpriteBank);
	SCAN_VAR(nCaveSpriteBankDelay);

	for (int i = 0; i < 4; i++) {
		SCAN_VAR(CaveTileReg[i][0]);
		SCAN_VAR(CaveTileReg[i][1]);
		SCAN_VAR(CaveTileReg[i][2]);
	}

	return 0;
}

// This function fills the screen with the background colour
void CaveClearScreen(unsigned int nColour)
{
#ifdef BUILD_PSP

	extern void clear_gui_texture(int color, int w, int h);
	clear_gui_texture(((nColour & 0x001f ) << 3) | ((nColour & 0x07e0 ) << 5) | ((nColour & 0xf800 ) << 8), nCaveXSize, nCaveYSize);

#else

	if (nColour) {
		unsigned int* pClear = (unsigned int*)pBurnDraw;
		nColour = nColour | (nColour << 16);
		for (int i = nCaveXSize * nCaveYSize / 16; i > 0 ; i--) {
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
		}
	} else {
		memset(pBurnDraw, 0, nCaveXSize * nCaveYSize * sizeof(short));
	}

#endif
}

