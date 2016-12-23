void K007232Update(short* pSoundBuf, int nLength);
unsigned char K007232ReadReg(int r);
void K007232WriteReg(int r, int v);
void K007232SetPortWriteHandler(void (*Handler)(int v));
void K007232Init(int clock, UINT8 *pPCMData, int PCMDataSize);
void K007232Exit();
int K007232Scan(int nAction, int *pnMin);
void K007232SetVolume(int channel,int volumeA,int volumeB);
