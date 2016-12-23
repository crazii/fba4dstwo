// Create a unique name for each of the functions
#define FN(a,b,c,d,e,f,g) RenderSprite ## a ## _ ## b ## _ROT ## c ## d ## e ## _CLIPX ## f ## g
#define FUNCTIONNAME(a,b,c,d,e,f,g) FN(a,b,c,d,e,f,g)

#if ROT == 0
 #define ADVANCEWORD pPixel += ((BPP >> 3) * 16)
 #define PIXELOFFSET(o) ((o)*(BPP >> 3))
 #ifndef NDS
  #define ADVANCEROW pRow += ((BPP >> 3) * XSIZE)
 #else
  #define ADVANCEROW pRow += ((BPP >> 3) * 512)
 #endif 
#elif ROT == 270
#ifndef NDS
#define ADVANCEWORD pPixel -= (((BPP >> 3) * XSIZE) * 16)
#define PIXELOFFSET(o) (-(o)*(BPP >> 3)*XSIZE)
#else
#define ADVANCEWORD pPixel -= (((BPP >> 3) * 512) * 16)
#define PIXELOFFSET(o) (-(o)*(BPP >> 3)*512)
#endif 
#define ADVANCEROW pRow += ((BPP >> 3))
#else
#error unsupported rotation angle specified
#endif

#if EIGHTBIT == 0
 #define DEPTH _16
#elif EIGHTBIT == 1
 #define DEPTH _256
#else
 #error illegal eightbit value
#endif

#define TESTCOLOUR(x) x

#if ZBUFFER == 0
 #define ZBUF _NOZBUFFER
 #define ADVANCEZWORD
 #define ADVANCEZROW
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a)
#elif ZBUFFER == 1
 #define ZBUF _RZBUFFER
 #define ADVANCEZWORD pZPixel += 16
// #ifndef NDS
  #define ADVANCEZROW pZRow += XSIZE
// #else
//  #define ADVANCEZROW pZRow += 512
// #endif
 #define TESTZBUF(a) (pZPixel[a] <= nZPos)
 #define WRITEZBUF(a)
#elif ZBUFFER == 2
 #define ZBUF _WZBUFFER
 #define ADVANCEZWORD pZPixel += 16
// #ifndef NDS
  #define ADVANCEZROW pZRow += XSIZE
// #else
//  #define ADVANCEZROW pZRow += 512
// #endif
 #define TESTZBUF(a) 1
 #define WRITEZBUF(a) pZPixel[a] = nZPos
#elif ZBUFFER == 3
 #define ZBUF _RWZBUFFER
 #define ADVANCEZWORD pZPixel += 16
// #ifndef NDS
  #define ADVANCEZROW pZRow += XSIZE
// #else
//  #define ADVANCEZROW pZRow += 512
// #endif 
 #define TESTZBUF(a) (pZPixel[a] <= nZPos)
 #define WRITEZBUF(a) pZPixel[a] = nZPos
#else
 #error unsupported zbuffer mode specified.
#endif

#if BPP == 16
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTZBUF(a)) {						\
   	WRITEZBUF(a);																\
	*((unsigned short*)(pPixel + PIXELOFFSET(a))) = (unsigned short)pSpritePalette[b];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTZBUF(a)) {						\
	WRITEZBUF(a);																\
	unsigned int nRGB = pSpritePalette[b];										\
	pPixel[PIXELOFFSET(a) + 0] = (unsigned char)nRGB;									\
	pPixel[PIXELOFFSET(a) + 1] = (unsigned char)(nRGB >> 8);								\
	pPixel[PIXELOFFSET(a) + 2] = (unsigned char)(nRGB >> 16);							\
 }
#elif BPP == 32
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTZBUF(a)) {						\
	WRITEZBUF(a);																\
	*((unsigned int*)(pPixel + PIXELOFFSET(a))) = (unsigned int)pSpritePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

#if XFLIP == 0
 #define FLIP _NOFLIP
#elif XFLIP == 1
 #define FLIP _FLIPX
#else
 #error illegal XFLIP value
#endif

#if ZOOM == 0
 #define ZOOMMODE _NOZOOM
#elif ZOOM == 1
 #define ZOOMMODE _ZOOMXY
#else
 #error unsupported rowscroll mode specified.
#endif

#if XFLIP == 0
 #define OFFSET(a) a
 #define CLIP(a,b) if (nColumn >= (XSIZE - a)) { continue; }			\
	if (nXPos >= (0 - a)) { b }
#else
 #define OFFSET(a) (15 - a)
 #define CLIP(a,b) if (nColumn >= (0 - a) && nColumn < (XSIZE - a)) { b }
#endif

#if EIGHTBIT == 0
 #define PLOT8_CLIP(x)		  				 	 	     				\
	CLIP(OFFSET((x + 0)), PLOTPIXEL(OFFSET((x + 0)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 1)), PLOTPIXEL(OFFSET((x + 1)), nColour & 0x0F));	\
																		\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 2)), PLOTPIXEL(OFFSET((x + 2)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 3)), PLOTPIXEL(OFFSET((x + 3)), nColour & 0x0F));	\
																		\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 4)), PLOTPIXEL(OFFSET((x + 4)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 5)), PLOTPIXEL(OFFSET((x + 5)), nColour & 0x0F));	\
																		\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 6)), PLOTPIXEL(OFFSET((x + 6)), nColour & 0x0F));	\
	nColour >>= 4;														\
	CLIP(OFFSET((x + 7)), PLOTPIXEL(OFFSET((x + 7)), nColour & 0x0F));

 #define PLOT8_NOCLIP(x)		   										\
	PLOTPIXEL(OFFSET((x + 0)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 1)), nColour & 0x0F);							\
																		\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 2)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 3)), nColour & 0x0F);							\
																		\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 4)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 5)), nColour & 0x0F);							\
																		\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 6)), nColour & 0x0F);							\
	nColour >>= 4;														\
	PLOTPIXEL(OFFSET((x + 7)), nColour & 0x0F);
#else
 #define PLOT4_CLIP(x) 				 	 	    		 				\
	CLIP(OFFSET((x + 0)), PLOTPIXEL(OFFSET((x + 0)), nColour & 0xFF));	\
	nColour >>= 8;	  													\
	CLIP(OFFSET((x + 1)), PLOTPIXEL(OFFSET((x + 1)), nColour & 0xFF));	\
	nColour >>= 8;	  													\
																	  	\
	CLIP(OFFSET((x + 2)), PLOTPIXEL(OFFSET((x + 2)), nColour & 0xFF));	\
	nColour >>= 8;														\
	CLIP(OFFSET((x + 3)), PLOTPIXEL(OFFSET((x + 3)), nColour & 0xFF));

 #define PLOT4_NOCLIP(x)	   											\
	PLOTPIXEL(OFFSET((x + 0)), nColour & 0xFF);							\
	nColour >>= 8;														\
	PLOTPIXEL(OFFSET((x + 1)), nColour & 0xFF);							\
	nColour >>= 8;														\
																		\
	PLOTPIXEL(OFFSET((x + 2)), nColour & 0xFF);							\
	nColour >>= 8;														\
	PLOTPIXEL(OFFSET((x + 3)), nColour & 0xFF);
#endif

static void FUNCTIONNAME(BPP,XSIZE,ROT,FLIP,ZOOMMODE,ZBUF,DEPTH)()
{
// Create an empty function if unsupported features are requested
	int x, nColumn;
	int nColour;

 #if ZBUFFER == 0
	for (nSpriteRow = 0; nSpriteRow < nYSize; ADVANCEROW, nSpriteRow++, spriteDataOffset += (nSpriteRowSize<<2)) {
 #else
		for (nSpriteRow = 0; nSpriteRow < nYSize; ADVANCEROW, ADVANCEZROW, nSpriteRow++, spriteDataOffset += (nSpriteRowSize<<2)) {
 #endif
		nColumn = nXPos;
unsigned int * pSpriteData=(unsigned int*)getBlock(spriteDataOffset,(nXSize>>(1 + EIGHTBIT))<<2);
 #if ZBUFFER == 0
  #if XFLIP == 0
		for (x = (0 << EIGHTBIT), pPixel = pRow; x < nXSize; x += (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD) {
  #else
		for (x = nXSize - (2 << EIGHTBIT), pPixel = pRow; x >= 0; x -= (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD) {
  #endif
 #else
  #if XFLIP == 0
		for (x = 0, pPixel = pRow, pZPixel = pZRow; x < nXSize; x += (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD, ADVANCEZWORD) {
  #else
		for (x = nXSize - (2 << EIGHTBIT), pPixel = pRow, pZPixel = pZRow; x >= 0; x -= (2 << EIGHTBIT), nColumn += 16, ADVANCEWORD, ADVANCEZWORD) {
  #endif
 #endif

 #if EIGHTBIT == 0
			if (nColumn >= 0 && nColumn < (XSIZE - 16)) {
  #if XFLIP == 0
				nColour = pSpriteData[x];
				PLOT8_NOCLIP(0);
				nColour = pSpriteData[x + 1];
				PLOT8_NOCLIP(8);
  #else
				nColour = pSpriteData[x + 1];
				PLOT8_NOCLIP(8);
				nColour = pSpriteData[x];
				PLOT8_NOCLIP(0);
  #endif
			} else {
  #if XFLIP == 0
				nColour = pSpriteData[x];
				PLOT8_CLIP(0);
				nColour = pSpriteData[x + 1];
				PLOT8_CLIP(8);
  #else
				nColour = pSpriteData[x + 1];
				PLOT8_CLIP(8);
				nColour = pSpriteData[x];
				PLOT8_CLIP(0);
  #endif
 #else
			if (nColumn >= 0 && nColumn < (XSIZE - 16)) {
  #if XFLIP == 0
				nColour = pSpriteData[x];
				PLOT4_NOCLIP(0);
				nColour = pSpriteData[x + 1];
				PLOT4_NOCLIP(4);
				nColour = pSpriteData[x + 2];
				PLOT4_NOCLIP(8);
				nColour = pSpriteData[x + 3];
				PLOT4_NOCLIP(12);
  #else
				nColour = pSpriteData[x + 3];
				PLOT4_NOCLIP(12);
				nColour = pSpriteData[x + 2];
				PLOT4_NOCLIP(8);
				nColour = pSpriteData[x + 1];
				PLOT4_NOCLIP(4);
				nColour = pSpriteData[x];
				PLOT4_NOCLIP(0);
  #endif
			} else {
  #if XFLIP == 0
				nColour = pSpriteData[x];
				PLOT4_CLIP(0);
				nColour = pSpriteData[x + 1];
				PLOT4_CLIP(4);
				nColour = pSpriteData[x + 2];
				PLOT4_CLIP(8);
				nColour = pSpriteData[x + 3];
				PLOT4_CLIP(12);
  #else
				nColour = pSpriteData[x + 3];
				PLOT4_CLIP(12);
				nColour = pSpriteData[x + 2];
				PLOT4_CLIP(8);
				nColour = pSpriteData[x + 1];
				PLOT4_CLIP(4);
				nColour = pSpriteData[x];
				PLOT4_CLIP(0);
  #endif
 #endif
			}
		}
	}
}

#undef PLOT4_CLIP
#undef PLOT4_NOCLIP
#undef PLOT8_CLIP
#undef PLOT8_NOCLIP
#undef OFFSET
#undef PIXELOFFSET
#undef FLIP
#undef PLOTPIXEL
#undef CLIP
#undef TESTCOLOUR
#undef ADVANCEZROW
#undef ADVANCEZWORD
#undef ADVANCEROW
#undef ADVANCEWORD
#undef TESTZBUF
#undef WRITEZBUF
#undef ZBUF
#undef ZOOMMODE
#undef DEPTH
#undef FUNCTIONNAME
#undef FN

