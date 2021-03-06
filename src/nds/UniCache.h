/*
 * Universal Cache System 
 * For small memory device system using.
 * Created by Bill Li
 * Email: lbicelyne@msn.com
 */
#ifndef UNI_CACHE_H
	#define UNI_CACHE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDS
	#include <fs_api.h>
	#include <ds2_malloc.h>
#else
	#include "malloc.h"
#endif

#ifdef malloc
#undef malloc
#endif
#define malloc(size) memalign(4, size)

#define CACHE_BLOCK_SIZE 0xFFFF
#define CACHE_INDEX_SHIFT 16

#define SHORT_INVALID_VALUE 0xFFFF
#define CACHE_INDEX_SIZE 8192
#define CACHE_SPACE_STATUS_SIZE (CACHE_INDEX_SIZE+31)/32

bool doDataCache();
void initCacheStructure(float ratio, bool needTemp = false);
void destroyCacheStructure();
void initUniCache(unsigned int cacheSize,float ratio);
void destroyUniCache();
unsigned short getUsedMemSize();
unsigned short getFreeMemSize();
struct UniCacheIndex{
	unsigned short nextIndex;
	unsigned short preIndex;
	unsigned short cacheSpaceHeadOffsetHigh;
	unsigned short cacheSpaceEndOffsetHigh;
};
extern unsigned short indexRecycleListHead, indexRecycleListEnd;
extern UniCacheIndex uniCacheIndex[CACHE_INDEX_SIZE]; 
extern unsigned char *uniCacheHead;
extern unsigned char *lastVisitedCacheBlock;
extern FILE* cacheFile;
extern unsigned int cacheFileSize;
extern unsigned int totalMemBlocks;
extern unsigned int requestAddrOffsetHigh,requestAddrOffsetLow,requestAddrEndOffsetHigh, headBlockIndexOffsetHigh, magicFreeSpaceOffsetHigh;
extern bool needCreateCache;
extern char filePathName[256];
extern unsigned int uniCacheSpaceStatus[CACHE_SPACE_STATUS_SIZE]; 
extern bool fillExtendData;
//extern void *mallocHook(int NumBytes);
//extern void freeHook(void *FirstByte);
extern void *mallocTemp(unsigned int NumBytes,void(*onDestroyFuncPtr)(unsigned char* addr));
bool visitMemTemp(unsigned int offset);
extern char* panicMsg;

//test
//extern int debugValue1, debugValue2;
////////////////////////////////
inline static void removeCurrentIndex(unsigned short offsetHighLocal)
{
	if(indexRecycleListEnd==offsetHighLocal||indexRecycleListHead==offsetHighLocal)	
	{
		if(indexRecycleListEnd==offsetHighLocal)
		{
			indexRecycleListEnd=uniCacheIndex[offsetHighLocal].preIndex;
			if(indexRecycleListEnd!=SHORT_INVALID_VALUE)
			{
				uniCacheIndex[indexRecycleListEnd].nextIndex=SHORT_INVALID_VALUE;
			}
		}
		if(indexRecycleListHead==offsetHighLocal)
		{
			indexRecycleListHead=uniCacheIndex[offsetHighLocal].nextIndex;
			if(indexRecycleListHead!=SHORT_INVALID_VALUE)
			{
				uniCacheIndex[indexRecycleListHead].preIndex=SHORT_INVALID_VALUE;
			}
		}
	}else
	{
		uniCacheIndex[uniCacheIndex[offsetHighLocal].nextIndex].preIndex=uniCacheIndex[offsetHighLocal].preIndex;
		uniCacheIndex[uniCacheIndex[offsetHighLocal].preIndex].nextIndex=uniCacheIndex[offsetHighLocal].nextIndex;
	}		
	uniCacheIndex[offsetHighLocal].nextIndex = SHORT_INVALID_VALUE;
	uniCacheIndex[offsetHighLocal].preIndex = SHORT_INVALID_VALUE;

}
inline static void moveCurrentIndexToEnd(unsigned short offsetHighLocal)
{
	if(indexRecycleListEnd!=offsetHighLocal)
	{	
		if(indexRecycleListHead==SHORT_INVALID_VALUE)
		{//First use
			indexRecycleListHead=offsetHighLocal;
			indexRecycleListEnd=offsetHighLocal;
			return;
		}
		if(indexRecycleListHead==offsetHighLocal&&indexRecycleListHead!=indexRecycleListEnd)
		{
			indexRecycleListHead=uniCacheIndex[offsetHighLocal].nextIndex;	
			uniCacheIndex[indexRecycleListHead].preIndex=SHORT_INVALID_VALUE;
				
		}else if(uniCacheIndex[offsetHighLocal].preIndex==SHORT_INVALID_VALUE)
		{
			;//not in the list, nothing to do
		}else
		{//take this node out the list
			uniCacheIndex[uniCacheIndex[offsetHighLocal].nextIndex].preIndex=uniCacheIndex[offsetHighLocal].preIndex;
			uniCacheIndex[uniCacheIndex[offsetHighLocal].preIndex].nextIndex=uniCacheIndex[offsetHighLocal].nextIndex;
				
		}
		uniCacheIndex[indexRecycleListEnd].nextIndex=offsetHighLocal;
		uniCacheIndex[offsetHighLocal].preIndex=indexRecycleListEnd;	
		uniCacheIndex[offsetHighLocal].nextIndex=SHORT_INVALID_VALUE;	
		indexRecycleListEnd=offsetHighLocal;
	}
}

inline void freeMem(unsigned int blockIndex)
{
	int i;
	unsigned int endBlockIndex;

	removeCurrentIndex(blockIndex);
	magicFreeSpaceOffsetHigh = uniCacheIndex[blockIndex].cacheSpaceHeadOffsetHigh;
	for(i=magicFreeSpaceOffsetHigh;i<=uniCacheIndex[blockIndex].cacheSpaceEndOffsetHigh;i++)
	{
		//set the bit to 0
		uniCacheSpaceStatus[i>>5]&=~(1UL<<(i&0x1F));
	}
	//Adjust the magic pointer to a free space head
	for(i=magicFreeSpaceOffsetHigh-1;i>=0;i--)
	{
		if(uniCacheSpaceStatus[i>>5]&(1UL<<(i&0x1F)))
			break;
	}
	magicFreeSpaceOffsetHigh=i+1;
	
	endBlockIndex=blockIndex+(uniCacheIndex[blockIndex].cacheSpaceEndOffsetHigh-uniCacheIndex[blockIndex].cacheSpaceHeadOffsetHigh);
	uniCacheIndex[blockIndex].cacheSpaceEndOffsetHigh=SHORT_INVALID_VALUE;

	for (i=blockIndex;i<=endBlockIndex;i++)
	{
		if(uniCacheIndex[i].cacheSpaceEndOffsetHigh!=SHORT_INVALID_VALUE)
		{
			break;
		}
		uniCacheIndex[i].cacheSpaceHeadOffsetHigh=SHORT_INVALID_VALUE;
	}
}

inline unsigned char* getBlock(unsigned int offset, unsigned int size)
{//do not support Reentry
	if(offset>=cacheFileSize)
		return 0;
	requestAddrOffsetHigh = (offset>>CACHE_INDEX_SHIFT);	
	requestAddrEndOffsetHigh =( (offset+size-1)>>CACHE_INDEX_SHIFT);
	
	headBlockIndexOffsetHigh=requestAddrOffsetHigh;
	if ( uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh == SHORT_INVALID_VALUE )
	{//No cached data in memory		
		if(!doDataCache())
			return 0;
	}else if(uniCacheIndex[requestAddrOffsetHigh].cacheSpaceEndOffsetHigh == SHORT_INVALID_VALUE)
	{//Request addr's head is in part of an exist cache block
		headBlockIndexOffsetHigh = uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh; //reuse cacheSpaceHeadOffsetHigh for headBlockIndexOffsetHigh in Index
		if	(headBlockIndexOffsetHigh+
				(uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceEndOffsetHigh-uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceHeadOffsetHigh)
		 	< requestAddrEndOffsetHigh)
		 //|| uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceEndOffsetHigh >= totalMemBlocks)
		{//Request block size is larger than cache block or data is invalid
			headBlockIndexOffsetHigh = requestAddrOffsetHigh;
			if(!doDataCache())
				return 0;
		}			 
	}else //Request addr's head is the head of an exist cache block
	if	(headBlockIndexOffsetHigh+
			(uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceEndOffsetHigh-uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceHeadOffsetHigh)
		 < requestAddrEndOffsetHigh)
		//|| uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceEndOffsetHigh >= totalMemBlocks)
	{//Request block size is larger than cache block or data is invalid	
		freeMem(headBlockIndexOffsetHigh);
		if(!doDataCache())
			return 0;
	}
	moveCurrentIndexToEnd(headBlockIndexOffsetHigh);
	lastVisitedCacheBlock=uniCacheHead+(uniCacheIndex[headBlockIndexOffsetHigh].cacheSpaceHeadOffsetHigh<<CACHE_INDEX_SHIFT);
	return lastVisitedCacheBlock+offset-(headBlockIndexOffsetHigh<<CACHE_INDEX_SHIFT);
}
inline unsigned char* getBlockSmallData(unsigned int offset)
{
	if(headBlockIndexOffsetHigh==(offset>>CACHE_INDEX_SHIFT))
	{
		return lastVisitedCacheBlock+(unsigned short)offset;
	}else
		return getBlock(offset,4);
}

#endif
