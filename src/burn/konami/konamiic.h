// konamiic.cpp
extern unsigned int KonamiIC_K051960InUse;
extern unsigned int KonamiIC_K052109InUse;

void KonamiICReset();
void KonamiICExit();
void KonamiICScan(int nAction);

// k051960.cpp
extern int K051960ReadRoms;

void K051960SpritesRender(unsigned char *pSrc);
unsigned char K0519060FetchRomData(unsigned int Offset);
unsigned char K051960Read(unsigned int Offset);
void K051960Write(unsigned int Offset, unsigned char Data);
void K051960SetCallback(void (*Callback)(int *Code, int *Colour, int *Priority, int *Shadow));
void K051960Reset();
void K051960Init(unsigned char* pRomSrc, unsigned int RomMask);
void K051960Exit();
void K051960Scan(int nAction);
void K051937Write(unsigned int Offset, unsigned char Data);

// k052109.cpp
extern int K052109RMRDLine;

void K052109UpdateScroll();
void K052109RenderLayer(int nLayer, int Opaque, unsigned char *pSrc);
unsigned char K052109Read(unsigned int Offset);
void K052109Write(unsigned int Offset, unsigned char Data);
void K052109SetCallback(void (*Callback)(int Layer, int Bank, int *Code, int *Colour, int *xFlip));
void K052109Reset();
void K052109Init(unsigned char *pRomSrc, unsigned int RomMask);
void K052109Exit();
void K052109Scan(int nAction);

#define K051960ByteRead(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	return K051960Read(a - nStartAddress);					\
}

#define K051960ByteWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	K051960Write((a - nStartAddress), d);					\
	return;									\
}

#define K051960WordWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	if (a & 1) {								\
		K051960Write((a - nStartAddress) + 1, d & 0xff);		\
	} else {								\
		K051960Write((a - nStartAddress) + 0, (d >> 8) & 0xff);		\
	}									\
	return;									\
}

#define K051937ByteRead(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 7) {				\
	int Offset = (a - nStartAddress);					\
										\
	if (Offset == 0) {							\
		static int Counter;						\
		return (Counter++) & 1;						\
	}									\
										\
	if (K051960ReadRoms && (Offset >= 0x04 && Offset <= 0x07)) {		\
		return K0519060FetchRomData(Offset & 3);			\
	}									\
										\
	return 0;								\
}

#define K015937ByteWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 7) {				\
	K051937Write((a - nStartAddress), d);					\
	return;									\
}

#define K052109WordNoA12Read(nStartAddress)					\
if (a >= nStartAddress && a <= nStartAddress + 0x7fff) {			\
	int Offset = (a - nStartAddress) >> 1;					\
	Offset = ((Offset & 0x3000) >> 1) | (Offset & 0x07ff);			\
										\
	if (a & 1) {								\
		return K052109Read(Offset + 0x2000);				\
	} else {								\
		return K052109Read(Offset + 0x0000);				\
	}									\
}


#define K052109WordNoA12Write(nStartAddress)					\
if (a >= nStartAddress && a <= nStartAddress + 0x7fff) {			\
	int Offset = (a - nStartAddress) >> 1;					\
	Offset = ((Offset & 0x3000) >> 1) | (Offset & 0x07ff);			\
										\
	if (a & 1) {								\
		K052109Write(Offset + 0x2000, d);				\
	} else {								\
		K052109Write(Offset + 0x0000, d);				\
	}									\
	return;									\
}
