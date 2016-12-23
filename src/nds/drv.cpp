// Driver Init module
#include "burner.h"
#include "nds.h"

int bDrvOkay=0; // 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions

int xOff;
int yOff;
int drvWidth;
int drvHeight;
int iAdd;
int iModulo;

static bool bSaveRAM = false;

static int DoLibInit() // Do Init of Burn library driver
{
  	int nRet;
  
	BzipOpen(false);

	// If there is an error with the romset, report it
	if (BzipStatus() != BZIP_STATUS_OK)  {
		// 
	}

	nRet = BurnDrvInit();

	BzipClose();

	return (nRet) ? 3 : 0 ;
}

int DrvExit()
{
	if (bDrvOkay) {

//		StopReplay();

//		VidExit();

//		InvalidateRect(hScrnWnd, NULL, 1);
//		UpdateWindow(hScrnWnd);			// Blank screen window

		if (nBurnDrvSelect < nBurnDrvCount) {

//			MemCardEject();				// Eject memory card if present

//			if (bSaveRAM) {
//				StatedAuto(1);			// Save NV (or full) RAM
//				bSaveRAM = false;
//			}

//			ConfigGameSave(bSaveInputs);

//			GameInpExit();				// Exit game input
			BurnDrvExit();				// Exit the driver
		}
	}

	BurnExtLoadRom = NULL;

	bDrvOkay = 0;					// Stop using the BurnDrv functions

//	bRunPause = 0;					// Don't pause when exitted

//	if (bAudOkay) {
		// Write silence into the sound buffer on exit, and for drivers which don't use pBurnSoundOut
//		memset(nAudNextSound, 0, nAudSegLen << 2);
//	}

	nBurnDrvSelect = ~0U;			// no driver selected
	return 0;
}

static int __cdecl DrvLoadRom(unsigned char* Dest, int* pnWrote, int i)
{
	int nRet;

//printf("real load rom\n");

	BzipOpen(false);

	if ((nRet = BurnExtLoadRom(Dest, pnWrote, i)) != 0) {
		char* pszFilename;

		BurnDrvGetRomName(&pszFilename, i, 0);
//		printf("Error load %s, requested by %s\n", pszFilename, BurnDrvGetText(DRV_NAME));
	}

	BzipClose();

	BurnExtLoadRom = DrvLoadRom;

//	ScrnTitle();

	return nRet;
}

extern "C" int DrvInit(int nDrvNum, bool bRestore)
{
	int nRet=0;
	DrvExit(); // Make sure exited

	nBurnDrvSelect=nDrvNum; // set the driver number
	
	InpInit();
	InpDIP();
	
	nRet=DoLibInit(); // Init the Burn library's driver

	if (nRet!=0) {
		BurnDrvExit(); // Exit the driver
//		printf ("There was an error starting '%s'.\n", BurnDrvGetText(DRV_NAME));
		return 1;
	}

//  TrainStart(); // Trainer allocate memory copies

//  GameInpInit(); // Init game input
  // Load config for the game
//  nRet=ConfigGameLoad(); if (nRet!=0) ConfigGameSave(); // create initial game config file

  // Dip switch values in driver will have been set now:
//  GameInpDefault(); // Auto-config any input which is still undefined to defaults
  // (also sets/gets dip switch valeus from driver)

	BurnExtLoadRom = DrvLoadRom;

	bDrvOkay=1; // Okay to use the BurnDrv functions

//  StatedAuto(0); // Load nv (or full) ram

//  ScrnTitle();
//  ScrnSize();

	nBurnLayer=0xff; // show all layers

  // Init the video plugin
//  nRet=VidInit();
//  if (nRet!=0) { AppError("VidInit Failed",0); return 1; }

  // Reset the speed throttling code, so we don't 'jump' after the load
//	RunReset();

	int width = SCREEN_WIDTH;
	int height = SCREEN_HEIGHT;
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetFullSize(&height, &width);
	} else {
		BurnDrvGetFullSize(&width, &height);
	}
	drvWidth = width;
	drvHeight = height;

	int iAddX, iModuloX,iAddY,iModuloY;
	iAddX = iModuloX = iAddY = iModuloY = 1;
	int scaledW,scaledH;

	if (SCREEN_WIDTH < width)
	{
		iAddX = SCREEN_WIDTH;
		iModuloX = width;
		scaledW = SCREEN_WIDTH;
	}
	if (SCREEN_HEIGHT < height)
	{
		iAddY = SCREEN_HEIGHT;
		iModuloY = height;
		scaledH = SCREEN_HEIGHT;
	}
	if (SCREEN_WIDTH < width || SCREEN_HEIGHT < height)
	{
		if (height * SCREEN_WIDTH < SCREEN_HEIGHT * width)
		{
			// scale is determined by horizontal scale
			iAdd = iAddX;
			iModulo = iModuloX;
			scaledH = (height * SCREEN_WIDTH + width - 1) / width;
			if (scaledH > SCREEN_HEIGHT)
				scaledH = SCREEN_HEIGHT;
		}
		else
		{
			// scale is determined by vertical scale
			iAdd = iAddY;
			iModulo = iModuloY;
			scaledW = (width * SCREEN_HEIGHT + height - 1) / height;
			if (scaledW > SCREEN_WIDTH)
				scaledW = SCREEN_WIDTH;
		}
	}
	
	xOff = (SCREEN_WIDTH - scaledW) / 2;
	yOff = (SCREEN_HEIGHT - scaledH) / 2;
	return 0;
}

