#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym3526.h"

void (*BurnYM3526Update)(short* pSoundBuf, int nSegmentEnd);

static int (*BurnYM3526StreamCallback)(int nSoundRate);

static int nBurnYM3526SoundRate;

static short* pBuffer;
static short* pYM3526Buffer;

static int nYM3526Position;

static unsigned int nSampleSize;
static unsigned int nFractionalPosition;

static int bYM3526AddSignal;

// ----------------------------------------------------------------------------
// Dummy functions

static void YM3526UpdateDummy(short* , int /* nSegmentEnd */)
{
	return;
}

static int YM3526StreamCallbackDummy(int /* nSoundRate */)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM3526 for part of a frame

static void YM3526Render(int nSegmentLength)
{
	if (nYM3526Position >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    YM3526 render %6i -> %6i\n", nYM3526Position, nSegmentLength));

	nSegmentLength -= nYM3526Position;

	YM3526UpdateOne(0, pBuffer + 0 * 4096 + 4 + nYM3526Position, nSegmentLength);

	nYM3526Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM3526UpdateResample(short* pSoundBuf, int nSegmentEnd)
{
	int nSegmentLength = nSegmentEnd;
	int nSamplesNeeded = nSegmentEnd * nBurnYM3526SoundRate / nBurnSoundRate + 1;

//	bprintf(PRINT_NORMAL, _T("    YM3526 update        -> %6i\n", nSegmentLength));

	if (nSamplesNeeded < nYM3526Position) {
		nSamplesNeeded = nYM3526Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM3526Render(nSamplesNeeded);

	pYM3526Buffer = pBuffer + 0 * 4096 + 4;

	for (int i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		short nSample =  INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
												pYM3526Buffer[(nFractionalPosition >> 16) - 3],
												pYM3526Buffer[(nFractionalPosition >> 16) - 2],
												pYM3526Buffer[(nFractionalPosition >> 16) - 1],
												pYM3526Buffer[(nFractionalPosition >> 16) - 0]);
		if (bYM3526AddSignal) {
			pSoundBuf[i + 0] += nSample;
			pSoundBuf[i + 1] += nSample;
		} else {
			pSoundBuf[i + 0] = nSample;
			pSoundBuf[i + 1] = nSample;
		}
	}

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSamplesNeeded - (nFractionalPosition >> 16);

//		bprintf(PRINT_NORMAL, _T("   %6i rendered, %i extra, %i <- %i\n"), nSamplesNeeded, nExtraSamples, nExtraSamples, (nFractionalPosition >> 16) + nExtraSamples - 1);

		for (int i = -4; i < nExtraSamples; i++) {
			pYM3526Buffer[i] = pYM3526Buffer[(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0xFFFF;

		nYM3526Position = nExtraSamples;
	}
}

static void YM3526UpdateNormal(short* pSoundBuf, int nSegmentEnd)
{
	int nSegmentLength = nSegmentEnd;

//	bprintf(PRINT_NORMAL, _T("    YM3526 render %6i -> %6i\n"), nYM3526Position, nSegmentEnd);

	if (nSegmentEnd < nYM3526Position) {
		nSegmentEnd = nYM3526Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YM3526Render(nSegmentEnd);

	pYM3526Buffer = pBuffer + 4 + 0 * 4096;

	for (int i = nFractionalPosition; i < nSegmentLength; i++) {
		if (bYM3526AddSignal) {
			pSoundBuf[(i << 1) + 0] += pYM3526Buffer[i];
			pSoundBuf[(i << 1) + 1] += pYM3526Buffer[i];
		} else {
			pSoundBuf[(i << 1) + 0] = pYM3526Buffer[i];
			pSoundBuf[(i << 1) + 1] = pYM3526Buffer[i];
		}
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (int i = 0; i < nExtraSamples; i++) {
			pYM3526Buffer[i] = pYM3526Buffer[nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYM3526Position = nExtraSamples;

	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM3526 core

void BurnYM3526UpdateRequest(int, int)
{
	YM3526Render(BurnYM3526StreamCallback(nBurnYM3526SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM3526Reset()
{
	BurnTimerReset();

	YM3526ResetChip(0);
}

void BurnYM3526Exit()
{
	YM3526Shutdown();

	BurnTimerExit();

	free(pBuffer);
	
	bYM3526AddSignal = 0;
}

int BurnYM3526Init(int nClockFrequency, OPL_IRQHANDLER IRQCallback, int (*StreamCallback)(int), int bAddSignal)
{
	BurnTimerInit(&YM3526TimerOver, NULL);

	if (nBurnSoundRate <= 0) {
		BurnYM3526StreamCallback = YM3526StreamCallbackDummy;

		BurnYM3526Update = YM3526UpdateDummy;

		YM3526Init(1, nClockFrequency, 11025);
		return 0;
	}

	BurnYM3526StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM3526 core samplerate to match the hardware
		nBurnYM3526SoundRate = nClockFrequency / 72;
		// Bring YM3526 core samplerate within usable range
		while (nBurnYM3526SoundRate > nBurnSoundRate * 3) {
			nBurnYM3526SoundRate >>= 1;
		}

		BurnYM3526Update = YM3526UpdateResample;

		nSampleSize = (unsigned int)nBurnYM3526SoundRate * (1 << 16) / nBurnSoundRate;
		nFractionalPosition = 0;
	} else {
		nBurnYM3526SoundRate = nBurnSoundRate;

		BurnYM3526Update = YM3526UpdateNormal;
	}

	YM3526Init(1, nClockFrequency, nBurnYM3526SoundRate);
	YM3526SetIRQHandler(0, IRQCallback, 0);
	YM3526SetTimerHandler(0, &BurnOPLTimerCallback, 0);
	YM3526SetUpdateHandler(0, &BurnYM3526UpdateRequest, 0);

	pBuffer = (short*)malloc(4096 * sizeof(short));
	memset(pBuffer, 0, 4096 * sizeof(short));

	nYM3526Position = 0;

	nFractionalPosition = 0;
	
	bYM3526AddSignal = bAddSignal;

	return 0;
}

void BurnYM3526Scan(int nAction, int* pnMin)
{
	BurnTimerScan(nAction, pnMin);
}
