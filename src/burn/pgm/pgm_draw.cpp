#include "pgm.h"
#include "UniCache.h"
extern unsigned long nPGMTileROMOffset;
extern unsigned long nPGMSPRColROMOffset;
extern unsigned long nPGMSPRMaskROMOffset;
static unsigned short * pgm_sprite_source; 
SpriteCacheIndex *spriteCacheArray=0;
unsigned int spriteCacheArrayFreeP=0;

inline static unsigned char* getBlockTile(unsigned long offset, unsigned long size)
{
	return getBlock(nPGMTileROMOffset+offset,size);
}
inline static unsigned char* getBlockSPRMask(unsigned long offset, unsigned long size)
{
	return getBlock(nPGMSPRMaskROMOffset+offset,size);
}
inline static unsigned char* getBlockSPRCol(unsigned long offset, unsigned long size)
{
	return getBlock(nPGMSPRColROMOffset+offset,size);
}
/* PGM Palette */ 

#ifndef BUILD_PSP

#define PGM_WIDTH	448

#else

#define PGM_WIDTH	512

#endif

void __fastcall pgmOnDestroy(unsigned char* addr)
{
	//test use
	//debugValue1++;
	//panicMsg="pgmOnDestroy in";
	SpriteCacheHead* destroyHead=(SpriteCacheHead*)addr;
	unsigned short cacheIndex=destroyHead->cacheIndex;

	SpriteCacheIndex *cacheIndexPtr=&spriteCacheArray[cacheIndex];
	
	SpriteCacheHead* head=cacheIndexPtr->src;
	SpriteCacheHead* head2=0;
	while(head!=0)
	{
		if(head==destroyHead)
			break;
		else
		{
			head2=head;
			head=head->nextSrcPtr;
		}
	}
	if(head==0) return;
	if(head2==0)
		cacheIndexPtr->src=head->nextSrcPtr;
	else
		head2->nextSrcPtr=head->nextSrcPtr;
	
	BoffsetHead* boffsetHead=destroyHead->sprMaskHeadPtr;
	if	(	boffsetHead->magicChar2==0xC2&&boffsetHead->magicChar1==0xCC&&
		(	boffsetHead->cacheIndexHigh<<8|boffsetHead->cacheIndexLow )==cacheIndex)
	{
		if(cacheIndexPtr->src==0)
		{
			unsigned char* bdat=(unsigned char*)boffsetHead;
			unsigned int aoff=spriteCacheArray[cacheIndex].aoff;
			bdat[0]=(aoff)&0xFF;
			bdat[1]=(aoff>>8)&0xFF;
			bdat[2]=(aoff>>16)&0xFF;
			bdat[3]=aoff>>24;
			//debugValue2++;
		}
	}else
	{
		cacheIndexPtr->src=0;
	}
	//test use
	//panicMsg="pgmOnDestroy out";
}

static void pgm_drawsprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int boffset, int flip, unsigned int /*xzoom*/, int /*xgrow*/, unsigned int /*yzoom*/, int /*ygrow*/ )
{
	if ( boffset >= 0x1000000 ) return;
	
	int wideHigh= wide*high;
	unsigned char * bdat =  getBlockSPRMask (boffset, ((wideHigh)<<1)+4);
	if(bdat==0)	return;
	unsigned int aoff;
	unsigned char * adat;
	unsigned short msk;
	unsigned char * src;
	SpriteCacheHead* head;
	SpriteCacheHead* head2=0;
	bool needReCache=true;
	unsigned int spriteCacheArrayCurrentOffset;
	int newtiledatasize =sizeof(SpriteCacheHead)+((wideHigh)<<4);
	BoffsetHead* boffsetHead=(BoffsetHead*)bdat;
	
	
	if (boffsetHead->magicChar2==0xC2&&boffsetHead->magicChar1==0xCC)
	{
		//test use
		//panicMsg="boffsetHead->magicChar2==0xC2 in";
		
		spriteCacheArrayCurrentOffset=boffsetHead->cacheIndexHigh<<8|boffsetHead->cacheIndexLow;
		//test use
		//panicMsg="step 1";
		head = spriteCacheArray[spriteCacheArrayCurrentOffset].src;
		aoff = spriteCacheArray[spriteCacheArrayCurrentOffset].aoff;
		//test use
		//panicMsg="step 2";
		while(head!=0)
		{
			if(head->wide==wide&&head->high==high)
			{
				//test use
				//panicMsg="step 2.1";
				if(visitMem((unsigned long)head+cacheFileSize))
					needReCache=false;
				else
					//test use
					panicMsg="visitMem return false!";
				break;
			}else
			{
				//test use
				//panicMsg="step 2.3";
				head2=head;
				head=head->nextSrcPtr;
				//test use
				//panicMsg="step 2.4";
			}
		}
		//test use
		//panicMsg="step 3";
		if(needReCache)
		{
			//test use
			//panicMsg="step 4";
			adat= getBlockSPRCol(aoff>>1, wideHigh*12);
			if(adat==0)	return;
			//test use
			//panicMsg="step 5";
			head=(SpriteCacheHead* )mallocTemp(newtiledatasize,pgmOnDestroy);
			if (head == 0) return;
			//test use
			//panicMsg="step 6";
			if (head2 == 0)
				spriteCacheArray[spriteCacheArrayCurrentOffset].src=head;
			else
				head2->nextSrcPtr=head;
		}
		//test use
		//panicMsg="boffsetHead->magicChar2==0xC2 out";
	}else
	{
		//test use
		//panicMsg="boffsetHead->magicChar2!=0xC2 in";
		aoff = (bdat[3]<< 24 |bdat[2]<< 16 |bdat[1] << 8 |bdat[0]);
		int i=spriteCacheArrayFreeP;
		for(;spriteCacheArrayFreeP<SPRITE_CACHE_SIZE;spriteCacheArrayFreeP++)
		{
			if(spriteCacheArray[spriteCacheArrayFreeP].src==0)
			{
				break;
			}
		}
		if(spriteCacheArrayFreeP==SPRITE_CACHE_SIZE)
		{
			for(spriteCacheArrayFreeP=0;spriteCacheArrayFreeP<i;spriteCacheArrayFreeP++)
			{
				if(spriteCacheArray[spriteCacheArrayFreeP].src==0)
				{
					break;
				}
			}
			if(spriteCacheArrayFreeP==i)
			{
				for(spriteCacheArrayFreeP=0;spriteCacheArrayFreeP<SPRITE_CACHE_SIZE;spriteCacheArrayFreeP++)
				{
					spriteCacheArray[spriteCacheArrayFreeP].src==0;
				}
				spriteCacheArrayFreeP=0;
				mallocTemp(0x10000000,0);
				return;
			}
		}
		adat= getBlockSPRCol(aoff>>1, wideHigh*12);
		if(adat==0)	return;
		head=(SpriteCacheHead* )mallocTemp(newtiledatasize,pgmOnDestroy);
		if (head == 0) return;
		spriteCacheArrayCurrentOffset=spriteCacheArrayFreeP;
		spriteCacheArray[spriteCacheArrayCurrentOffset].aoff = aoff;
		spriteCacheArray[spriteCacheArrayCurrentOffset].src = head;
		boffsetHead->cacheIndexHigh = spriteCacheArrayCurrentOffset>>8;
		boffsetHead->cacheIndexLow = spriteCacheArrayCurrentOffset&0xFF;
		boffsetHead->magicChar1 = 0xCC;
		boffsetHead->magicChar2 = 0xC2;
			

		//test use
		//panicMsg="boffsetHead->magicChar2!=0xC2 out";
	}
	
	if(needReCache)
	{		
		//test use
		//panicMsg="needReCache in";		
		head->wide=wide;
		head->high=high;
		head->sprMaskHeadPtr=boffsetHead;
		head->cacheIndex=spriteCacheArrayCurrentOffset;
		//test use
		//panicMsg="needReCache step 1";
		head->nextSrcPtr=0;
		
		unsigned char * dest = (unsigned char*)head+sizeof(SpriteCacheHead);
	
		unsigned int gfx3data = 0;
		unsigned int gfx3datapos = 0;
		
		boffset = 4;
		aoff = 0;
		//test use
		//panicMsg="needReCache step 2";
		for (int ycnt = 0 ; ycnt < high ; ycnt++) {
			for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
				msk = (( bdat[boffset+1] << 8) | bdat[boffset]);
				for (int x=0;x<16;x++) {
					if (!(msk & 0x0001)) {
						if (gfx3datapos == 0) {
							gfx3data = adat[aoff+1]<<8|adat[aoff];
							aoff=aoff+2;
						}

						*dest++ = gfx3data & 0x1f;
						
						if (gfx3datapos == 2) {
							gfx3datapos = 0;
						} else {
							gfx3datapos ++;
							gfx3data >>= 5;
						}
					} else *dest++ = 0x80;
					msk >>=1;
				}
				boffset+=2;
			}	
		}
		//test use
		//panicMsg="needReCache out";	
	}
	src=(unsigned char*)head+sizeof(SpriteCacheHead);
	unsigned short * dst = (unsigned short *)pBurnDraw;
	int w = wide<<4;
	int h = high;
	
	unsigned int * pRamCurPal = RamCurPal + palt;
	dst += (ypos * PGM_WIDTH) + xpos;

	int xfix = w;
	unsigned char * src2;
	
	if ( flip & 0x02 ) {
		
		dst += (h-1) * PGM_WIDTH;
		
		if ((ypos + h) > 224) {
			src += (ypos + h - 224) * w;
			dst -= (ypos + h - 224) * PGM_WIDTH;
			h   -= (ypos + h - 224);
		}
		if (ypos < 0) h += ypos;

		if ( flip & 0x01 ) {

			src += w-1;
			if ( xpos < 0 ) {
				dst -= xpos;
				src += xpos;
				xfix += xpos;
				if (xfix <= 0) return;
			}
			if ((xpos + w) > 448) xfix -= (xpos + w - 448);
			if (xfix <= 0) return;
			for (int j=0; j<h; j++) {
				src2 = src;
				for(int i=0; i<xfix; i++, src2--)
					if (*src2 <= 0x1f) 
						dst[i] = pRamCurPal[ *src2 ];
				src += w;
				dst -= PGM_WIDTH;
			}

		} else {

			if ( xpos < 0 ) {
				dst -= xpos;
				src -= xpos;
				xfix += xpos;
			}
			if ((xpos + w) > 448) xfix -= (xpos + w - 448);
			if (xfix <= 0) return;
			for (int j=0; j<h; j++) {
				src2 = src;
				for(int i=0; i<xfix; i++, src2++)
					if (*src2 <= 0x1f) 
						dst[i] = pRamCurPal[ *src2 ];
				src += w;
				dst -= PGM_WIDTH;
			}

		}

	} else {
		
		if ((ypos + h) > 224) h -= (ypos + h - 224);
		
		if (ypos < 0) {
			dst -= ypos * PGM_WIDTH;
			src -= ypos * w;
			h += ypos;
			if ( h <= 0 ) return;
		}

		if ( flip & 0x01 ) {

			src += w-1;
			if ( xpos < 0 ) {
				dst -= xpos;
				src += xpos;
				xfix += xpos;
				if (xfix <= 0) return;
			}
			if ((xpos + w) > 448) xfix -= (xpos + w - 448);
			if (xfix <= 0) return;
			for (int j=0; j<h; j++) {
				src2 = src;
				for(int i=0; i<xfix; i++, src2--)
					if (*src2 <= 0x1f) 
						dst[i] = pRamCurPal[ *src2 ];
				src += w;
				dst += PGM_WIDTH;
			}

		} else {

			if ( xpos < 0 ) {
				dst -= xpos;
				src -= xpos;
				xfix += xpos;
			}
			if ((xpos + w) > 448) xfix -= (xpos + w - 448);
			if (xfix <= 0) return;
			for (int j=0; j<h; j++) {
				src2 = src;
				for(int i=0; i<xfix; i++, src2++)
					if (*src2 <= 0x1f) 
						dst[i] = pRamCurPal[ *src2 ];
				src += w;
				dst += PGM_WIDTH;
			}

		}
	}
		
/*
	int wideHigh=wide * high;
	unsigned char * bdata = getBlockSPRMask (boffset, ((wideHigh)<<1)+4);
	if(bdata==0) return;
	unsigned int aoffset = bdata[3]<< 24 |bdata[2]<< 16 |bdata[1] << 8 |bdata[0];
	unsigned short* adata = (unsigned short *)getBlockSPRCol(aoffset>>1, wideHigh*12);
	if(adata==0) return;
	aoffset=0;
	boffset = 4; // because the first dword is the a data offset
	unsigned short msk;
	unsigned short * dst = (unsigned short *)pBurnDraw;
	unsigned int gfx3data = 0;
	unsigned int gfx3datapos = 0;

	dst += (ypos * PGM_WIDTH) + xpos;

	int yflip_dir = PGM_WIDTH;
	if (flip & 0x02) {
		yflip_dir = -PGM_WIDTH;
		dst += (high - 1) * PGM_WIDTH;
	} 
	
	for (int ycnt = 0 ; ycnt < high ; ycnt++) {
		int newy;
		if (flip & 0x02) {
			newy = ypos + (high - 1) - ycnt;
			if ( newy < 0) break;
		} else {
			newy = ycnt + ypos;
			if ( newy >= 224) break;
		}
		if (newy >= 0 && newy <224) {
			
			if ( flip & 0x01 ) {
				for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
					int newxb = wide*16 - 1;
					msk = (( bdata[boffset+1] << 8) | bdata[boffset]);
					for (int x=0;x<16;x++) {
						int newx = newxb - xcnt*16 - x;
						if (!(msk & 0x0001)) {
							if (gfx3datapos == 0) {
								gfx3data = adata[aoffset];
								aoffset ++;
							}
							if (((newx + xpos) >= 0) && ((newx + xpos)<448))
								dst[ newx ] = RamCurPal[ (gfx3data & 0x1f) + palt ];
							if (gfx3datapos == 2) {
								gfx3datapos = 0;
							} else {
								gfx3datapos ++;
								gfx3data >>= 5;
							}
						}
						msk >>=1;
					}
					boffset+=2;
				}
			} else {
				for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
					msk = (( bdata[boffset+1] << 8) | bdata[boffset]);
					for (int x=0;x<16;x++) {
						int newx = xcnt*16 + x;
						if (!(msk & 0x0001)) {
							if (gfx3datapos == 0) {
								gfx3data = adata[aoffset];
								aoffset++;
							}
							if (((newx + xpos) >= 0) && ((newx + xpos)<448))
								dst[ newx ] = RamCurPal[ (gfx3data & 0x1f) + palt ];
							if (gfx3datapos == 2) {
								gfx3datapos = 0;
							} else {
								gfx3datapos ++;
								gfx3data >>= 5;
							}
						}
						msk >>=1;
					}
					boffset+=2;
				}	
			}
		
		} else {
			for (int xcnt = 0 ; xcnt < wide ; xcnt++) {
				msk = (( bdata[boffset+1] << 8) | bdata[boffset]);
				for (int x=0;x<16;x++) {
					if (!(msk & 0x0001)) {
						if (gfx3datapos == 0) {
							gfx3data = adata[aoffset];
							aoffset++;
						}
						if (gfx3datapos == 2) {
							gfx3datapos = 0;
						} else {
							gfx3datapos ++;
							gfx3data >>= 5;
						}
					}
					msk >>=1;
				}
				boffset+=2;
			}
		}
		dst += yflip_dir;
	}
*/	
}

static void pgm_drawsprites(int priority)
{
	/* 10 byte per sprite, 256 sprites
	   ZZZZ Zxxx xxxx xxxx
       zzzz z-yy yyyy yyyy
       -ffp pppp Pvvv vvvv
       vvvv vvvv vvvv vvvv
       wwww wwwh hhhh hhhh */

    const unsigned short * finish = RamSpr + (0xA00 / 2);
    while( pgm_sprite_source < finish ) {
    	int pri = (pgm_sprite_source[2] & 0x0080) >>  7;
    	if ((priority == 1) && (pri == 0)) break;
    	int high = pgm_sprite_source[4] & 0x01ff;
    	if (high == 0) break; /* is this right? */
    	
    			
	    int xpos = pgm_sprite_source[0] & 0x07ff;
			int ypos = pgm_sprite_source[1] & 0x03ff;
			int xzom = (pgm_sprite_source[0] & 0x7800) >> 11;
			int xgrow = (pgm_sprite_source[0] & 0x8000) >> 15;
			int yzom = (pgm_sprite_source[1] & 0x7800) >> 11;
			int ygrow = (pgm_sprite_source[1] & 0x8000) >> 15;
			int palt = (pgm_sprite_source[2] & 0x1f00) >> 3;
			int flip = (pgm_sprite_source[2] & 0x6000) >> 13;
			int boff = ((pgm_sprite_source[2] & 0x007f) << 16) | (pgm_sprite_source[3] & 0xffff);
			int wide = (pgm_sprite_source[4] & 0x7e00) >> 9;
			
			
			unsigned int xzoom, yzoom;
			unsigned short * pgm_sprite_zoomtable = & RamVReg[0x1000 / 2];
	
			if (xgrow) {
				xzom = 0x10-xzom; // this way it doesn't but there is a bad line when zooming after the level select?
			}
			if (ygrow) {
				yzom = 0x10-yzom;
			}
			xzoom = (pgm_sprite_zoomtable[xzom*2]<<16)|pgm_sprite_zoomtable[xzom*2+1];
			yzoom = (pgm_sprite_zoomtable[yzom*2]<<16)|pgm_sprite_zoomtable[yzom*2+1];
			
			boff *= 2;
			if (xpos > 0x3ff) xpos -=0x800;
			if (ypos > 0x1ff) ypos -=0x400;
	
			pgm_drawsprite_new_zoomed(wide, high, xpos, ypos, palt, boff, flip, xzoom,xgrow, yzoom,ygrow);
			pgm_sprite_source += 5;
    }
    
}

static void pgm_tile_tx()
{
/* 0x904000 - 0x905fff is the Text Overlay Ram ( RamTx )
    each tile uses 4 bytes, the tilemap is 64x32?

   the layer uses 4bpp 8x8 tiles from the 'T' roms
   colours from 0xA01000 - 0xA017FF

   scroll registers are at 0xB05000 (Y) and 0xB06000 (X)

    ---- ---- ffpp ppp- nnnn nnnn nnnn nnnn

    n = tile number
    p = palette
    f = flip
*/
	int tileno, colour, flipyx;
	int mx=-1, my=0, x, y;
	unsigned int *pal = & RamCurPal[0x800];
	
	const unsigned int * finish = RamTx + 0x800;
	for (unsigned int * tiledata = RamTx; tiledata < finish; tiledata++) {
		tileno = (*tiledata >>  0) & 0xFFFF;
		colour = (*tiledata >> 13) & 0x1F0;
		flipyx = (*tiledata >> 22) & 0x03;
		
		if (tileno > 0xbfff) { tileno -= 0xc000 ; tileno += 0x20000; } // not sure about this
		
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}
		
		// is this right ? but it will save many cpu cycles ..
		if (tileno == 0) continue;

		x = mx * 8 - (signed short)RamVReg[0x6000 / 2];
		y = my * 8 - (signed short)RamVReg[0x5000 / 2];

		if ( x<=-8 || x>=448 || y<=-8 || y>= 224 ) continue;

		unsigned char *d = getBlockTile(tileno<<5,32);
		if(d==0) return;
		unsigned short * p = (unsigned short *) pBurnDraw + y * PGM_WIDTH + x;
		unsigned int v;

		if ( x >=0 && x < 440 && y >= 0 && y < 216) {
			for (int k=0;k<8;k++) {
				v = d[0] & 0xf;	if (v != 15) p[0] = pal[ v | colour ];
 				v = d[0] >> 4;	if (v != 15) p[1] = pal[ v | colour ];
 				v = d[1] & 0xf;	if (v != 15) p[2] = pal[ v | colour ];
 				v = d[1] >> 4;	if (v != 15) p[3] = pal[ v | colour ];
 				v = d[2] & 0xf;	if (v != 15) p[4] = pal[ v | colour ];
 				v = d[2] >> 4;	if (v != 15) p[5] = pal[ v | colour ];
 				v = d[3] & 0xf;	if (v != 15) p[6] = pal[ v | colour ];
 				v = d[3] >> 4;	if (v != 15) p[7] = pal[ v | colour ];
 				d += 4;
 				p += PGM_WIDTH;
 			}
		} else {
			for (int k=0;k<8;k++) {
				if ((y+k) >= 224) break;
				if ((y+k) >= 0) {
	 				v = d[0] & 0xf;	if (v != 15 && (x + 0) >= 0 && (x + 0)<448) p[0] = pal[ v | colour ];
	 				v = d[0] >> 4;	if (v != 15 && (x + 1) >= 0 && (x + 1)<448) p[1] = pal[ v | colour ];
	 				v = d[1] & 0xf;	if (v != 15 && (x + 2) >= 0 && (x + 2)<448) p[2] = pal[ v | colour ];
	 				v = d[1] >> 4;	if (v != 15 && (x + 3) >= 0 && (x + 3)<448) p[3] = pal[ v | colour ];
	 				v = d[2] & 0xf;	if (v != 15 && (x + 4) >= 0 && (x + 4)<448) p[4] = pal[ v | colour ];
	 				v = d[2] >> 4;	if (v != 15 && (x + 5) >= 0 && (x + 5)<448) p[5] = pal[ v | colour ];
	 				v = d[3] & 0xf;	if (v != 15 && (x + 6) >= 0 && (x + 6)<448) p[6] = pal[ v | colour ];
	 				v = d[3] >> 4;	if (v != 15 && (x + 7) >= 0 && (x + 7)<448) p[7] = pal[ v | colour ];
 				}
 				d += 4;
 				p += PGM_WIDTH;
 			}
		}
	}
}

#ifndef PGM_LOW_MEMORY

static void pgm_tile_bg()
{
	int tileno, colour, flipx, flipy;
	int mx=-1, my=0, x, y;
	unsigned int *pal = & RamCurPal[0x400];
	
	const unsigned int * finish = RamBg + 0x1000;
	for (unsigned int * tiledata = RamBg; tiledata < finish; tiledata ++) {
		tileno = (*tiledata) & 0xFFFF;
		
		if (tileno > 0x7ff)
			tileno += 0x1000;	 // Tiles 0x800+ come from the GAME Roms
	
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}
		
		if (tileno == 0) continue;

		colour = (*tiledata >> 12) & 0x3E0;
		flipx = (*tiledata >> 22) & 0x01;
		flipy = (*tiledata >> 23) & 0x01;

		x = mx * 32 - (signed short)RamVReg[0x3000 / 2];
		if (x <= (448 - 64 * 32)) x += (64 * 32);
		
		y = my * 32 - (signed short)RamVReg[0x2000 / 2];
		if (y <= (224 - 64 * 32)) y += (64 * 32);
		
		if ( x<=-32 || x>=448 || y<=-32 || y>= 224 ) 
			continue;
		
		if ( x >=0 && x < (448-32) && y >= 0 && y < (224-32)) {
			unsigned char * d = PGMTileROMExp + tileno * 1024;
 			unsigned short * p = (unsigned short *) pBurnDraw + y * PGM_WIDTH + x;
 			
 			if ( flipy ) {
 				
 				p += 31 * PGM_WIDTH;
 				
 				if ( flipx ) {
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[31] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[30] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[29] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[28] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[27] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[26] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[25] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[24] = pal[ d[ 7] | colour ];
			
		 				if (d[ 8] != 31) p[23] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[22] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[21] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[20] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[19] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[18] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[17] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[16] = pal[ d[15] | colour ];
			
		 				if (d[16] != 31) p[15] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[14] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[13] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[12] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[11] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[10] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[ 9] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[ 8] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[ 7] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[ 6] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[ 5] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[ 4] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[ 3] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[ 2] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[ 1] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[ 0] = pal[ d[31] | colour ];
		 				d += 32;
		 				p -= PGM_WIDTH;
		 			}
 					
 				} else {
 					// not flip x , flip y
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[ 0] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[ 1] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[ 2] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[ 3] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[ 4] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[ 5] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[ 6] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[ 7] = pal[ d[ 7] | colour ];
		
		 				if (d[ 8] != 31) p[ 8] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[ 9] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[10] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[11] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[12] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[13] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[14] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[15] = pal[ d[15] | colour ];
		
		 				if (d[16] != 31) p[16] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[17] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[18] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[19] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[20] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[21] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[22] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[23] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[24] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[25] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[26] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[27] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[28] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[29] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[30] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[31] = pal[ d[31] | colour ];
		 				d += 32;
		 				p -= PGM_WIDTH;
		 			}
 				}			
 			
 			} else {
 				
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[31] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[30] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[29] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[28] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[27] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[26] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[25] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[24] = pal[ d[ 7] | colour ];
		
		 				if (d[ 8] != 31) p[23] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[22] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[21] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[20] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[19] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[18] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[17] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[16] = pal[ d[15] | colour ];
		
		 				if (d[16] != 31) p[15] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[14] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[13] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[12] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[11] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[10] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[ 9] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[ 8] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[ 7] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[ 6] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[ 5] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[ 4] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[ 3] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[ 2] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[ 1] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[ 0] = pal[ d[31] | colour ];
		 				d += 32;
		 				p += PGM_WIDTH;
		 			}
 				} else {
 					// not flip x , not flip y
 					for (int k=0;k<32;k++) {
						if (d[ 0] != 31) p[ 0] = pal[ d[ 0] | colour ];
		 				if (d[ 1] != 31) p[ 1] = pal[ d[ 1] | colour ];
		 				if (d[ 2] != 31) p[ 2] = pal[ d[ 2] | colour ];
		 				if (d[ 3] != 31) p[ 3] = pal[ d[ 3] | colour ];
		 				if (d[ 4] != 31) p[ 4] = pal[ d[ 4] | colour ];
		 				if (d[ 5] != 31) p[ 5] = pal[ d[ 5] | colour ];
		 				if (d[ 6] != 31) p[ 6] = pal[ d[ 6] | colour ];
		 				if (d[ 7] != 31) p[ 7] = pal[ d[ 7] | colour ];
		
		 				if (d[ 8] != 31) p[ 8] = pal[ d[ 8] | colour ];
		 				if (d[ 9] != 31) p[ 9] = pal[ d[ 9] | colour ];
		 				if (d[10] != 31) p[10] = pal[ d[10] | colour ];
		 				if (d[11] != 31) p[11] = pal[ d[11] | colour ];
		 				if (d[12] != 31) p[12] = pal[ d[12] | colour ];
		 				if (d[13] != 31) p[13] = pal[ d[13] | colour ];
		 				if (d[14] != 31) p[14] = pal[ d[14] | colour ];
		 				if (d[15] != 31) p[15] = pal[ d[15] | colour ];
		
		 				if (d[16] != 31) p[16] = pal[ d[16] | colour ];
		 				if (d[17] != 31) p[17] = pal[ d[17] | colour ];
		 				if (d[18] != 31) p[18] = pal[ d[18] | colour ];
		 				if (d[19] != 31) p[19] = pal[ d[19] | colour ];
		 				if (d[20] != 31) p[20] = pal[ d[20] | colour ];
		 				if (d[21] != 31) p[21] = pal[ d[21] | colour ];
		 				if (d[22] != 31) p[22] = pal[ d[22] | colour ];
		 				if (d[23] != 31) p[23] = pal[ d[23] | colour ];
		
		 				if (d[24] != 31) p[24] = pal[ d[24] | colour ];
		 				if (d[25] != 31) p[25] = pal[ d[25] | colour ];
		 				if (d[26] != 31) p[26] = pal[ d[26] | colour ];
		 				if (d[27] != 31) p[27] = pal[ d[27] | colour ];
		 				if (d[28] != 31) p[28] = pal[ d[28] | colour ];
		 				if (d[29] != 31) p[29] = pal[ d[29] | colour ];
		 				if (d[30] != 31) p[30] = pal[ d[30] | colour ];
		 				if (d[31] != 31) p[31] = pal[ d[31] | colour ];
		 				d += 32;
		 				p += PGM_WIDTH;
		 			}
 				}
  			}
 		} else {
			
			unsigned char * d = PGMTileROMExp + tileno * 1024;
 			unsigned short * p = (unsigned short *) pBurnDraw + y * PGM_WIDTH + x;
 			
 			if ( flipy ) {
 				
 				p += 31 * PGM_WIDTH;
 				
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=31;k>=0;k--) {
						if ((y+k) < 0) break;
						if ((y+k) < 224) {
			 				if (d[31] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[31] | colour ];
			 				if (d[30] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[30] | colour ];
			 				if (d[29] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[29] | colour ];
			 				if (d[28] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[28] | colour ];
			 				if (d[27] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[27] | colour ];
			 				if (d[26] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[26] | colour ];
			 				if (d[25] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[25] | colour ];
			 				if (d[24] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[24] | colour ];
			
			 				if (d[23] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[23] | colour ];
			 				if (d[22] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[22] | colour ];
			 				if (d[21] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[21] | colour ];
			 				if (d[20] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[20] | colour ];
			 				if (d[19] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[19] | colour ];
			 				if (d[18] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[18] | colour ];
			 				if (d[17] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[17] | colour ];
			 				if (d[16] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[16] | colour ];
			
			 				if (d[15] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[15] | colour ];
			 				if (d[14] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[14] | colour ];
			 				if (d[13] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[13] | colour ];
			 				if (d[12] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[12] | colour ];
			 				if (d[11] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[11] | colour ];
			 				if (d[10] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[10] | colour ];
			 				if (d[ 9] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[ 9] | colour ];
			 				if (d[ 8] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[ 8] | colour ];
			
			 				if (d[ 7] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[ 7] | colour ];
			 				if (d[ 6] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[ 6] | colour ];
			 				if (d[ 5] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[ 5] | colour ];
			 				if (d[ 4] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[ 4] | colour ];
			 				if (d[ 3] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[ 3] | colour ];
			 				if (d[ 2] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[ 2] | colour ];
			 				if (d[ 1] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[ 1] | colour ];
			 				if (d[ 0] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[ 0] | colour ];
						}
		 				d += 32;
		 				p -= PGM_WIDTH;
		 			}
 				} else {
 					for (int k=31;k>=0;k--) {
						if ((y+k) < 0) break;
						if ((y+k) < 224) {
			 				if (d[ 0] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[ 0] | colour ];
			 				if (d[ 1] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[ 1] | colour ];
			 				if (d[ 2] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[ 2] | colour ];
			 				if (d[ 3] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[ 3] | colour ];
			 				if (d[ 4] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[ 4] | colour ];
			 				if (d[ 5] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[ 5] | colour ];
			 				if (d[ 6] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[ 6] | colour ];
			 				if (d[ 7] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[ 7] | colour ];
			
			 				if (d[ 8] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[ 8] | colour ];
			 				if (d[ 9] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[ 9] | colour ];
			 				if (d[10] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[10] | colour ];
			 				if (d[11] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[11] | colour ];
			 				if (d[12] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[12] | colour ];
			 				if (d[13] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[13] | colour ];
			 				if (d[14] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[14] | colour ];
			 				if (d[15] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[15] | colour ];
			
			 				if (d[16] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[16] | colour ];
			 				if (d[17] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[17] | colour ];
			 				if (d[18] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[18] | colour ];
			 				if (d[19] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[19] | colour ];
			 				if (d[20] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[20] | colour ];
			 				if (d[21] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[21] | colour ];
			 				if (d[22] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[22] | colour ];
			 				if (d[23] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[23] | colour ];
			
			 				if (d[24] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[24] | colour ];
			 				if (d[25] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[25] | colour ];
			 				if (d[26] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[26] | colour ];
			 				if (d[27] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[27] | colour ];
			 				if (d[28] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[28] | colour ];
			 				if (d[29] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[29] | colour ];
			 				if (d[30] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[30] | colour ];
			 				if (d[31] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[31] | colour ];
						}
		 				d += 32;
		 				p -= PGM_WIDTH;
		 			}
 				}			
 			} else {
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=0;k<32;k++) {
						if ((y+k) >= 224) break;
						if ((y+k) >= 0) {
			 				if (d[31] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[31] | colour ];
			 				if (d[30] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[30] | colour ];
			 				if (d[29] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[29] | colour ];
			 				if (d[28] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[28] | colour ];
			 				if (d[27] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[27] | colour ];
			 				if (d[26] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[26] | colour ];
			 				if (d[25] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[25] | colour ];
			 				if (d[24] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[24] | colour ];
			
			 				if (d[23] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[23] | colour ];
			 				if (d[22] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[22] | colour ];
			 				if (d[21] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[21] | colour ];
			 				if (d[20] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[20] | colour ];
			 				if (d[19] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[19] | colour ];
			 				if (d[18] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[18] | colour ];
			 				if (d[17] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[17] | colour ];
			 				if (d[16] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[16] | colour ];
			
			 				if (d[15] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[15] | colour ];
			 				if (d[14] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[14] | colour ];
			 				if (d[13] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[13] | colour ];
			 				if (d[12] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[12] | colour ];
			 				if (d[11] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[11] | colour ];
			 				if (d[10] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[10] | colour ];
			 				if (d[ 9] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[ 9] | colour ];
			 				if (d[ 8] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[ 8] | colour ];
			
			 				if (d[ 7] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[ 7] | colour ];
			 				if (d[ 6] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[ 6] | colour ];
			 				if (d[ 5] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[ 5] | colour ];
			 				if (d[ 4] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[ 4] | colour ];
			 				if (d[ 3] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[ 3] | colour ];
			 				if (d[ 2] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[ 2] | colour ];
			 				if (d[ 1] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[ 1] | colour ];
			 				if (d[ 0] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[ 0] | colour ];
						}
		 				d += 32;
		 				p += PGM_WIDTH;
		 			}
 				} else {
 					for (int k=0;k<32;k++) {
						if ((y+k) >= 224) break;
						if ((y+k) >= 0) {
			 				if (d[ 0] != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ d[ 0] | colour ];
			 				if (d[ 1] != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ d[ 1] | colour ];
			 				if (d[ 2] != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ d[ 2] | colour ];
			 				if (d[ 3] != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ d[ 3] | colour ];
			 				if (d[ 4] != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ d[ 4] | colour ];
			 				if (d[ 5] != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ d[ 5] | colour ];
			 				if (d[ 6] != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ d[ 6] | colour ];
			 				if (d[ 7] != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ d[ 7] | colour ];
			
			 				if (d[ 8] != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ d[ 8] | colour ];
			 				if (d[ 9] != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ d[ 9] | colour ];
			 				if (d[10] != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ d[10] | colour ];
			 				if (d[11] != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ d[11] | colour ];
			 				if (d[12] != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ d[12] | colour ];
			 				if (d[13] != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ d[13] | colour ];
			 				if (d[14] != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ d[14] | colour ];
			 				if (d[15] != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ d[15] | colour ];
			
			 				if (d[16] != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ d[16] | colour ];
			 				if (d[17] != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ d[17] | colour ];
			 				if (d[18] != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ d[18] | colour ];
			 				if (d[19] != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ d[19] | colour ];
			 				if (d[20] != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ d[20] | colour ];
			 				if (d[21] != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ d[21] | colour ];
			 				if (d[22] != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ d[22] | colour ];
			 				if (d[23] != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ d[23] | colour ];
			
			 				if (d[24] != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ d[24] | colour ];
			 				if (d[25] != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ d[25] | colour ];
			 				if (d[26] != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ d[26] | colour ];
			 				if (d[27] != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ d[27] | colour ];
			 				if (d[28] != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ d[28] | colour ];
			 				if (d[29] != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ d[29] | colour ];
			 				if (d[30] != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ d[30] | colour ];
			 				if (d[31] != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ d[31] | colour ];
						}
		 				d += 32;
		 				p += PGM_WIDTH;
		 			}
 				}
 			}
		}
	}
}

#else

static void pgm_tile_bg()
{
	int tileno, /*colour,*/ flipx, flipy;
	int mx=-1, my=0, x, y;
	
	const unsigned int * finish = RamBg + 0x1000;
	for (unsigned int * tiledata = RamBg; tiledata < finish; tiledata ++) {
		tileno = (*tiledata) & 0xFFFF;
		
		if (tileno > 0x7ff)
			tileno += 0x1000;	 // Tiles 0x800+ come from the GAME Roms
	
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}
		
		if (tileno == 0) continue;

		//colour = (*tiledata >> 12) & 0x3E0;
		unsigned int *pal = &RamCurPal[0x400 + ((*tiledata >> 12)&0x3E0)];
		
		flipx = (*tiledata >> 22) & 0x01;
		flipy = (*tiledata >> 23) & 0x01;

		x = mx * 32 - (signed short)RamVReg[0x3000 / 2];
		if (x <= (448 - 64 * 32)) x += (64 * 32);
		
		y = my * 32 - (signed short)RamVReg[0x2000 / 2];
		if (y <= (224 - 64 * 32)) y += (64 * 32);
		
		if ( x<=-32 || x>=448 || y<=-32 || y>= 224 ) 
			continue;

		unsigned int * d = (unsigned int*)getBlockTile(tileno * 640,640);
		if(d==0) return;
		unsigned int dd, ddd;
		unsigned short * p = (unsigned short *) pBurnDraw + y * PGM_WIDTH + x;
		
		if ( x >=0 && x < (448-32) && y >= 0 && y < (224-32)) {
 			
 				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=0;k<32;k++) {
 						dd = d[0];
 						ddd = dd & 0x1f; if (ddd != 31) p[31] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[30] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[29] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[28] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[27] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[26] = pal[ ddd ]; dd >>= 5;
						dd |= d[1] << 2;
 						ddd = dd & 0x1f; if (ddd != 31) p[25] = pal[ ddd ]; dd=d[1]>>3;
 						ddd = dd & 0x1f; if (ddd != 31) p[24] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[23] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[22] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[21] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[20] = pal[ ddd ]; dd >>= 5;
 						dd |= d[2] << 4;
 						ddd = dd & 0x1f; if (ddd != 31) p[19] = pal[ ddd ]; dd=d[2]>>1;
 						ddd = dd & 0x1f; if (ddd != 31) p[18] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[17] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[16] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[15] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[14] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[13] = pal[ ddd ]; dd >>= 5;
 						dd |= d[3] << 1;
 						ddd = dd & 0x1f; if (ddd != 31) p[12] = pal[ ddd ]; dd=d[3]>>4;
 						ddd = dd & 0x1f; if (ddd != 31) p[11] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[10] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 9] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 8] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 7] = pal[ ddd ]; dd >>= 5;
 						dd |= d[4] << 3;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 6] = pal[ ddd ]; dd=d[4]>>2;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 5] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 4] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 3] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 2] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 1] = pal[ ddd ]; dd >>= 5;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 0] = pal[ ddd ]; dd >>= 5;

		 				d += 5;
		 				p += PGM_WIDTH;
		 			}
		 			 					
 				} else {
 					// not flip x , not flip y
 					for (int k=0;k<32;k++) {
 						dd = d[0];
 						ddd = dd & 0x1f; if (ddd != 31) p[ 0] = pal[ ddd ]; dd >>= 5;	// -------- -------- -------- ---43210
 						ddd = dd & 0x1f; if (ddd != 31) p[ 1] = pal[ ddd ]; dd >>= 5;	// -------- -------- ------43 210-----
 						ddd = dd & 0x1f; if (ddd != 31) p[ 2] = pal[ ddd ]; dd >>= 5;	// -------- -------- -43210-- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[ 3] = pal[ ddd ]; dd >>= 5;	// -------- ----4321 0------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[ 4] = pal[ ddd ]; dd >>= 5;	// -------4 3210---- -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[ 5] = pal[ ddd ]; dd >>= 5;	// --43210- -------- -------- --------
						dd |= d[1] << 2;
 						ddd = dd & 0x1f; if (ddd != 31) p[ 6] = pal[ ddd ]; dd=d[1]>>3;// 10------ -------- -------- -----432
 						ddd = dd & 0x1f; if (ddd != 31) p[ 7] = pal[ ddd ]; dd >>= 5;	// -------- -------- -------- 43210---
 						ddd = dd & 0x1f; if (ddd != 31) p[ 8] = pal[ ddd ]; dd >>= 5;	// -------- -------- ---43210 --------
 						ddd = dd & 0x1f; if (ddd != 31) p[ 9] = pal[ ddd ]; dd >>= 5;	// -------- ------43 210----- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[10] = pal[ ddd ]; dd >>= 5;	// -------- -43210-- -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[11] = pal[ ddd ]; dd >>= 5;	// ----4321 0------- -------- --------
 						dd |= d[2] << 4;
 						ddd = dd & 0x1f; if (ddd != 31) p[12] = pal[ ddd ]; dd=d[2]>>1;// 3210---- -------- -------- -------4
 						ddd = dd & 0x1f; if (ddd != 31) p[13] = pal[ ddd ]; dd >>= 5;	// -------- -------- -------- --43210-
 						ddd = dd & 0x1f; if (ddd != 31) p[14] = pal[ ddd ]; dd >>= 5;	// -------- -------- -----432 10------
 						ddd = dd & 0x1f; if (ddd != 31) p[15] = pal[ ddd ]; dd >>= 5;	// -------- -------- 43210--- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[16] = pal[ ddd ]; dd >>= 5;	// -------- ---43210 -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[17] = pal[ ddd ]; dd >>= 5;	// ------43 210----- -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[18] = pal[ ddd ]; dd >>= 5;	// -43210-- -------- -------- --------
 						dd |= d[3] << 1;
 						ddd = dd & 0x1f; if (ddd != 31) p[19] = pal[ ddd ]; dd=d[3]>>4;// 0------- -------- -------- ----4321
 						ddd = dd & 0x1f; if (ddd != 31) p[20] = pal[ ddd ]; dd >>= 5;	// -------- -------- -------4 3210----
 						ddd = dd & 0x1f; if (ddd != 31) p[21] = pal[ ddd ]; dd >>= 5;	// -------- -------- --43210- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[22] = pal[ ddd ]; dd >>= 5;	// -------- -----432 10------ --------
 						ddd = dd & 0x1f; if (ddd != 31) p[23] = pal[ ddd ]; dd >>= 5;	// -------- 43210--- -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[24] = pal[ ddd ]; dd >>= 5;	// ---43210 -------- -------- --------
 						dd |= d[4] << 3;
 						ddd = dd & 0x1f; if (ddd != 31) p[25] = pal[ ddd ]; dd=d[4]>>2;// 210----- -------- -------- ------43
 						ddd = dd & 0x1f; if (ddd != 31) p[26] = pal[ ddd ]; dd >>= 5;	// -------- -------- -------- -43210--
 						ddd = dd & 0x1f; if (ddd != 31) p[27] = pal[ ddd ]; dd >>= 5;	// -------- -------- ----4321 0-------
 						ddd = dd & 0x1f; if (ddd != 31) p[28] = pal[ ddd ]; dd >>= 5;	// -------- -------4 3210---- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[29] = pal[ ddd ]; dd >>= 5;	// -------- --43210- -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[30] = pal[ ddd ]; dd >>= 5;	// -----432 10------ -------- --------
 						ddd = dd & 0x1f; if (ddd != 31) p[31] = pal[ ddd ]; dd >>= 5;	// 43210--- -------- -------- --------

		 				d += 5;
		 				p += PGM_WIDTH;
		 			}
		 		}
 		
 		} else {

				if ( flipx ) {
 					// flip x , not flip y
 					for (int k=0;k<32;k++) {
 						if ((y+k) >= 224) break;
						if ((y+k) >= 0) {
	 						dd = d[0];
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ ddd ]; dd >>= 5;
							dd |= d[1] << 2;                                                   
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ ddd ]; dd=d[1]>>3;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ ddd ]; dd >>= 5;
	 						dd |= d[2] << 4;                                                   
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ ddd ]; dd=d[2]>>1;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ ddd ]; dd >>= 5;
	 						dd |= d[3] << 1;                                                   
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ ddd ]; dd=d[3]>>4;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ ddd ]; dd >>= 5;
	 						dd |= d[4] << 3;                                                   
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ ddd ]; dd=d[4]>>2;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ ddd ]; dd >>= 5;
						}
		 				d += 5;
		 				p += PGM_WIDTH;
		 			}
		 								
				} else {
 					// not flip x , not flip y
 					for (int k=0;k<32;k++) {
 						if ((y+k) >= 224) break;
						if ((y+k) >= 0) {
	 						dd = d[0];
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 0) >= 0 && (x + 0)<448) p[ 0] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 1) >= 0 && (x + 1)<448) p[ 1] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 2) >= 0 && (x + 2)<448) p[ 2] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 3) >= 0 && (x + 3)<448) p[ 3] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 4) >= 0 && (x + 4)<448) p[ 4] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 5) >= 0 && (x + 5)<448) p[ 5] = pal[ ddd ]; dd >>= 5;
							dd |= d[1] << 2;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 6) >= 0 && (x + 6)<448) p[ 6] = pal[ ddd ]; dd=d[1]>>3;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 7) >= 0 && (x + 7)<448) p[ 7] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 8) >= 0 && (x + 8)<448) p[ 8] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x + 9) >= 0 && (x + 9)<448) p[ 9] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +10) >= 0 && (x +10)<448) p[10] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +11) >= 0 && (x +11)<448) p[11] = pal[ ddd ]; dd >>= 5;
	 						dd |= d[2] << 4;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +12) >= 0 && (x +12)<448) p[12] = pal[ ddd ]; dd=d[2]>>1;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +13) >= 0 && (x +13)<448) p[13] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +14) >= 0 && (x +14)<448) p[14] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +15) >= 0 && (x +15)<448) p[15] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +16) >= 0 && (x +16)<448) p[16] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +17) >= 0 && (x +17)<448) p[17] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +18) >= 0 && (x +18)<448) p[18] = pal[ ddd ]; dd >>= 5;
	 						dd |= d[3] << 1;               
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +19) >= 0 && (x +19)<448) p[19] = pal[ ddd ]; dd=d[3]>>4;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +20) >= 0 && (x +20)<448) p[20] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +21) >= 0 && (x +21)<448) p[21] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +22) >= 0 && (x +22)<448) p[22] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +23) >= 0 && (x +23)<448) p[23] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +24) >= 0 && (x +24)<448) p[24] = pal[ ddd ]; dd >>= 5;
	 						dd |= d[4] << 3;              
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +25) >= 0 && (x +25)<448) p[25] = pal[ ddd ]; dd=d[4]>>2;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +26) >= 0 && (x +26)<448) p[26] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +27) >= 0 && (x +27)<448) p[27] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +28) >= 0 && (x +28)<448) p[28] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +29) >= 0 && (x +29)<448) p[29] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +30) >= 0 && (x +30)<448) p[30] = pal[ ddd ]; dd >>= 5;
	 						ddd = dd & 0x1f; if (ddd != 31 && (x +31) >= 0 && (x +31)<448) p[31] = pal[ ddd ]; dd >>= 5;
						}
		 				d += 5;
		 				p += PGM_WIDTH;
		 			}
				}
 			
 		}
	}
}

#endif

int pgmDraw()
{
#ifndef BUILD_PSP
	for(int i=0;i<PGM_WIDTH*224*2/4;i++)
	{
		((unsigned int *)pBurnDraw)[i]=0;
	}
	//memset(pBurnDraw, 0, PGM_WIDTH*224*2);
#else
	extern void clear_gui_texture(int color, int w, int h);
	clear_gui_texture(0, 448, 224);
#endif
	pgm_sprite_source = RamSpr;	
	pgm_drawsprites(1);
	pgm_tile_bg();
	pgm_drawsprites(0);
	pgm_tile_tx();
	
	return 0;
}
