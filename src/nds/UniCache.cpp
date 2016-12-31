#include "UniCache.h"
#include "burner.h"

UniCacheIndex uniCacheIndex[CACHE_INDEX_SIZE];
char filePathName[256];
unsigned char *uniCacheHead = 0;
unsigned char *lastVisitedCacheBlock = 0;
FILE* cacheFile = NULL;
unsigned int cacheFileSize = 0;
unsigned int totalMemBlocks = 0;
unsigned int requestAddrOffsetHigh = 0,requestAddrEndOffsetHigh=0, headBlockIndexOffsetHigh=0, magicFreeSpaceOffsetHigh=0;
unsigned int  requestAddrOffsetLow = 0;
unsigned short indexRecycleListHead=SHORT_INVALID_VALUE, indexRecycleListEnd=SHORT_INVALID_VALUE;
unsigned int uniCacheSpaceStatus[CACHE_SPACE_STATUS_SIZE]={0xffffffff,};
char* panicMsg;
bool needCreateCache=false;
bool fillExtendData=false;

inline static void initTemp(bool useTemp);
inline static void clearTemp();


inline static void fillCacheData(unsigned int requestBlockCount)
{	
	//printf("requestAddrOffsetHigh:%d,\trequestBlockCount:%d\n",requestAddrOffsetHigh,requestBlockCount);
	unsigned int blockOffset = requestAddrOffsetHigh<<CACHE_INDEX_SHIFT;
	if( blockOffset>= cacheFileSize)
	{
		/*
		if(fillExtendData==true&&blockOffset<(CACHE_INDEX_SIZE<<CACHE_INDEX_SHIFT) )
		{//customize data fill function
			fillCacheDataFuncP(uniCacheHead+(uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh<<CACHE_INDEX_SHIFT));
		}*/
		return;
	}else
	{	
		if(!cacheFile) 
			cacheFile = fopen( filePathName, "rb");
			
		fseek( cacheFile, blockOffset, SEEK_SET );
		fread(uniCacheHead+(uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh<<CACHE_INDEX_SHIFT), requestBlockCount<<CACHE_INDEX_SHIFT, 1, cacheFile);
	}
/*
int i;
for(i=0;i<(requestBlockCount<<CACHE_INDEX_SHIFT);i=i+4)
{

		*(unsigned int*)(uniCacheHead+i+(uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh<<CACHE_INDEX_SHIFT))=i+blockOffset;

	
}*/
}
inline static unsigned short findFreeMem(unsigned int requestBlockCount)
{
	unsigned int i;
	unsigned int freeMemCount=0;
	
	
	for(i=magicFreeSpaceOffsetHigh;i<totalMemBlocks;i++)
	{
		if(uniCacheSpaceStatus[i>>5]&(1UL<<(i&0x1F))){
			freeMemCount=0;
		}else
		{
			freeMemCount++;
			if(freeMemCount==requestBlockCount)
			{
				return i+1-freeMemCount;
			}
		}	
	}
	freeMemCount=0;
	for(i=0;i<magicFreeSpaceOffsetHigh;i++)
	{
		if(uniCacheSpaceStatus[i>>5]&(1UL<<(i&0x1F))){
			freeMemCount=0;
		}else
		{
			freeMemCount++;
			if(freeMemCount==requestBlockCount)
			{
				return i+1-freeMemCount;
			}
		}	
	}
	return SHORT_INVALID_VALUE;
}

inline static unsigned short getMem(unsigned int requestBlockCount)
{
	unsigned int i,freeCacheSpaceOffsetHigh,j;
	do
	{
		freeCacheSpaceOffsetHigh =findFreeMem(requestBlockCount);
		if(freeCacheSpaceOffsetHigh!=SHORT_INVALID_VALUE)
		{
			j=freeCacheSpaceOffsetHigh+requestBlockCount;
			for(i=freeCacheSpaceOffsetHigh;i<j;i++)
			{
				//set the bit to 1
				uniCacheSpaceStatus[i>>5]|=(1UL<<(i&0x1F));
			}
			return freeCacheSpaceOffsetHigh;
		}
		freeMem(indexRecycleListHead);
	}while(indexRecycleListHead!=SHORT_INVALID_VALUE);
	return SHORT_INVALID_VALUE;
}

bool doDataCache()
{
	unsigned int requestBlockCount=requestAddrEndOffsetHigh-requestAddrOffsetHigh+1;
	unsigned short freeCacheSpaceOffsetHigh;
	unsigned int iCount;
	freeCacheSpaceOffsetHigh=getMem(requestBlockCount);
	if(freeCacheSpaceOffsetHigh==SHORT_INVALID_VALUE)
		return false;
	uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh=freeCacheSpaceOffsetHigh;
	//Adjust the magic pointer to a free space head
	magicFreeSpaceOffsetHigh=freeCacheSpaceOffsetHigh+requestBlockCount;
	
	uniCacheIndex[requestAddrOffsetHigh].cacheSpaceEndOffsetHigh=magicFreeSpaceOffsetHigh-1;
	for (iCount=requestAddrOffsetHigh+1;iCount<=requestAddrEndOffsetHigh;iCount++)
	{//Fill the following index data
		if(uniCacheIndex[iCount].cacheSpaceEndOffsetHigh!=SHORT_INVALID_VALUE)
		{//We meet another block index head here
			if(uniCacheIndex[iCount].cacheSpaceEndOffsetHigh<=requestAddrEndOffsetHigh)
				freeMem(iCount);
			else 
				break;
		}
		uniCacheIndex[iCount].cacheSpaceEndOffsetHigh=SHORT_INVALID_VALUE;
		uniCacheIndex[iCount].cacheSpaceHeadOffsetHigh=requestAddrOffsetHigh;//reuse cacheSpaceHeadOffsetHigh for headBlockIndexOffsetHigh in Index
	}
	//Fill the cache data
	fillCacheData(requestBlockCount);
	

	return true;
}

void initCacheStructure(float ratio, bool needTemp/* = false*/)
{
	if (needTemp)
		initTemp(needTemp);

	unsigned int iCount;
	for(totalMemBlocks = CACHE_INDEX_SIZE; totalMemBlocks>0; totalMemBlocks--)
	{
		uniCacheHead = (unsigned char *)memalign(4,totalMemBlocks<<CACHE_INDEX_SHIFT);
		if( uniCacheHead != NULL )
			break;
	}
	free (uniCacheHead);
	
	totalMemBlocks = (unsigned int)(totalMemBlocks*((ratio<=1&&ratio>0)?ratio:0.7))-3; //reserve for other dynamic using
	uniCacheHead = (unsigned char *)malloc(totalMemBlocks<<CACHE_INDEX_SHIFT);

	indexRecycleListHead=SHORT_INVALID_VALUE; indexRecycleListEnd=SHORT_INVALID_VALUE;
	magicFreeSpaceOffsetHigh=0;
	fillExtendData=false;
	memset(uniCacheIndex,0xFF,sizeof(UniCacheIndex)*CACHE_INDEX_SIZE);
	memset(uniCacheSpaceStatus,0xFF,CACHE_SPACE_STATUS_SIZE*4);
	for (iCount=0;iCount<totalMemBlocks;iCount++)
	{
	    //set the bit to 0
		uniCacheSpaceStatus[iCount>>5]&=~(1UL<<(iCount&0x1F));
	}
}	

void destroyCacheStructure()
{
	free(uniCacheHead);
	uniCacheHead = 0;
	clearTemp();
}

void initUniCache(unsigned int cacheSize,float ratio)
{	
	cacheFileSize=cacheSize;
	initCacheStructure(ratio);
	extern char szAppCachePath[];
	
	strcpy(filePathName, szAppCachePath);
	strcat(filePathName, BurnDrvGetTextA(DRV_NAME));
	strcat(filePathName, "_LB");
	needCreateCache = false;
	cacheFile = fopen( filePathName, "rb");
	if (cacheFile<0)
	{
		needCreateCache = true;
		cacheFile = fopen( filePathName, "wb+");
	}else if(fseek(cacheFile,0,SEEK_END)!=cacheFileSize)
	{
		needCreateCache = true;
		fclose(cacheFile);
		cacheFile = fopen( filePathName, "w+b" );
	}
}
void destroyUniCache()
{
	destroyCacheStructure();
	fclose(cacheFile);
	needCreateCache = false;
	cacheFile = NULL;
}

unsigned short getUsedMemSize()
{
	unsigned int temp,blockCount=0;
	temp=indexRecycleListHead;
	if (uniCacheHead==0||temp == SHORT_INVALID_VALUE)
		return 0;
	for(int i=0;i<CACHE_INDEX_SIZE;i++)
	{
		blockCount=blockCount+uniCacheIndex[temp].cacheSpaceEndOffsetHigh-uniCacheIndex[temp].cacheSpaceHeadOffsetHigh+1;
		if (temp==indexRecycleListEnd)
			break;
		temp=uniCacheIndex[temp].nextIndex;
	}
	return blockCount*64;
}
unsigned short getFreeMemSize()
{
	int i,j,freeMemCount=0;
	if(uniCacheHead==0) return 0;
	unsigned int 	memBlockOffsetHigh=((totalMemBlocks+31)>>5)-1;
	for(i=0;i<=memBlockOffsetHigh;i++)
	{
		for(j=0;j<32;j++)
		{
			if(uniCacheSpaceStatus[i]&(1UL<<j)){
				;
			}else
				freeMemCount++;
		}		
	}
	return freeMemCount*64;

}

/*

void *mallocHook(int NumBytes)
{
	unsigned int freeCacheSpaceOffsetHigh;
	unsigned int requestBlockCount=(NumBytes+65535)>>CACHE_INDEX_SHIFT;
	void* memPtr;
	freeCacheSpaceOffsetHigh=getMem(requestBlockCount);
	if(freeCacheSpaceOffsetHigh==SHORT_INVALID_VALUE)
		return 0;
	memPtr =(void*)(uniCacheHead + (freeCacheSpaceOffsetHigh<<CACHE_INDEX_SHIFT));
	requestAddrOffsetHigh=((unsigned int)memPtr+cacheFileSize)>>CACHE_INDEX_SHIFT;
	uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh=freeCacheSpaceOffsetHigh;
	//Adjust the magic pointer to a free space head
	magicFreeSpaceOffsetHigh=freeCacheSpaceOffsetHigh+requestBlockCount;
	uniCacheIndex[requestAddrOffsetHigh].cacheSpaceEndOffsetHigh=magicFreeSpaceOffsetHigh-1;
	return memPtr;
}
void freeHook(void *FirstByte)
{	int i;
	unsigned int blockIndex=((unsigned int)FirstByte+cacheFileSize)>>CACHE_INDEX_SHIFT;
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
	
	uniCacheIndex[blockIndex].cacheSpaceEndOffsetHigh=SHORT_INVALID_VALUE;
	uniCacheIndex[blockIndex].cacheSpaceHeadOffsetHigh=SHORT_INVALID_VALUE;
	return;
}
*/
//void *mallocTemp(int NumBytes, void(*onDestroyFuncPtr)(unsigned char* addr))
//{
//	unsigned int freeCacheSpaceOffsetHigh;
//	unsigned int requestBlockCount=(NumBytes+65535)>>CACHE_INDEX_SHIFT;
//	void* memPtr;
//	freeCacheSpaceOffsetHigh=getMem(requestBlockCount);
//	if(freeCacheSpaceOffsetHigh==SHORT_INVALID_VALUE)
//		return 0;
//	memPtr =(void*)(uniCacheHead + (freeCacheSpaceOffsetHigh<<CACHE_INDEX_SHIFT));
//	requestAddrOffsetHigh=((unsigned int)memPtr+cacheFileSize)>>CACHE_INDEX_SHIFT;
//	uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh=freeCacheSpaceOffsetHigh;
//	//Adjust the magic pointer to a free space head
//	magicFreeSpaceOffsetHigh=freeCacheSpaceOffsetHigh+requestBlockCount;
//	uniCacheIndex[requestAddrOffsetHigh].cacheSpaceEndOffsetHigh=magicFreeSpaceOffsetHigh-1;
//	uniCacheIndex[requestAddrOffsetHigh].onDestroyFuncPtr=onDestroyFuncPtr;
//	moveCurrentIndexToEnd(requestAddrOffsetHigh);
//	
//	return memPtr;
//}

#define TEMP_MEMORY_SIZE (4*1024*1024)
#define TEMP_BLOCK_MAX 4096
#define TEMP_SIZE_SHIFT 8	//align to 256bytes
static unsigned char* tempMemory = 0;
static unsigned int tempLeft = 0;
static unsigned short tempAddr2Index[TEMP_MEMORY_SIZE >> TEMP_SIZE_SHIFT];

struct TempIndex
{
	void* addr;
	void(*onDestroyFuncPtr)(unsigned char*);
	unsigned int size;
	short prev;
	short next;
};

static TempIndex tempIndex[TEMP_BLOCK_MAX];
static int tempHead;		//MRU list
static int tempTail;
static int tempAddrTail;	//tail of last address: merge from tail

static void initTemp(bool needTemp)
{
	if (needTemp)
	{
		tempMemory = (unsigned char*)memalign(4, TEMP_MEMORY_SIZE);
		tempLeft = TEMP_MEMORY_SIZE;
		tempAddrTail = 0;
	}
	else
	{
		tempMemory = NULL;
		tempLeft = 0;
		tempAddrTail = TEMP_BLOCK_MAX;
	}
	memset(tempAddr2Index, -1, sizeof(tempAddr2Index));
	memset(tempIndex, -1, sizeof(tempIndex));
	tempHead = tempTail = -1;
}

static void clearTemp()
{
	if (tempMemory)
		free(tempMemory);
	tempMemory = NULL;
	tempHead = tempTail = tempAddrTail = -1;
}

static void removeFromList(short id)
{
	if(tempIndex[id].prev != -1)
		tempIndex[tempIndex[id].prev].next = tempIndex[id].next;
	else
		tempHead = tempIndex[id].next;

	if(tempIndex[id].next != -1)
		tempIndex[tempIndex[id].next].prev = tempIndex[id].prev;
	else
		tempTail = tempIndex[id].prev;
}

static void moveToTail(short id)
{
	if(tempTail == id)
		return;
	removeFromList(id);

	tempIndex[tempTail].next = id;
	tempIndex[id].prev = tempTail;
	tempIndex[id].next = -1;
	tempTail = id;
}


void *mallocTemp(unsigned int NumBytes, void(*onDestroyFuncPtr)(unsigned char* addr))
{
	NumBytes = (NumBytes + (1 << TEMP_SIZE_SHIFT) - 1) & (~((1 << TEMP_SIZE_SHIFT) - 1));
	if(NumBytes > TEMP_MEMORY_SIZE)
		return NULL;

	if (tempLeft >= (unsigned)NumBytes && tempAddrTail < TEMP_BLOCK_MAX)
	{
		unsigned char* addr = (tempMemory + TEMP_MEMORY_SIZE - tempLeft);
		tempLeft -= NumBytes;

		TempIndex* index = &tempIndex[tempAddrTail];
		index->addr = addr;
		index->onDestroyFuncPtr = onDestroyFuncPtr;
		index->size = NumBytes;
		index->next = -1;

		//append to tail (MRU)
		if (tempTail != -1)
		{
			tempIndex[tempTail].next = tempAddrTail;
			index->prev = tempTail;
			tempTail = tempAddrTail;
		}
		else
		{
			tempHead = tempTail = tempAddrTail;
			index->prev = -1;
		}
		tempAddr2Index[(addr - tempMemory) >> TEMP_SIZE_SHIFT] = tempAddrTail++;
		return addr;
	}

	//reuse block with matched size
	//try head first (LRU)
	{
		int count = 0;
		short match = -1;
		short match2nd = -1;
		short matchbigger = -1;
		int halfcount = tempAddrTail >> 1;
		int quatercount = tempAddrTail >> 2;
		unsigned short match2ndsize = NumBytes + (NumBytes+3) / 4;
		unsigned short matchbiggersize = NumBytes + (NumBytes+1) / 2;

		for (int i = tempHead; /*count < tempAddrTail && */i != -1; i=tempIndex[i].next,++count)
		{
			if (tempIndex[i].size < NumBytes)
				continue;
			if (tempIndex[i].size == NumBytes)
			{
				match = i;
				break;
			}

			if (count < halfcount && tempIndex[i].size < match2ndsize)
			{
				match2ndsize = tempIndex[i].size;
				match2nd = i;
			}
			else if (count < quatercount && tempIndex[i].size < matchbiggersize)
			{
				matchbiggersize = tempIndex[i].size;
				matchbigger = i;
			}
		}

		int bestmatch = match != -1 ? match : (match2nd != -1 ? match2nd : (matchbigger));

		if (bestmatch != -1)
		{
			//clean up last
			tempIndex[bestmatch].onDestroyFuncPtr((unsigned char*)tempIndex[bestmatch].addr);
			tempIndex[bestmatch].onDestroyFuncPtr = onDestroyFuncPtr;
			//move to tail
			moveToTail(bestmatch);
			return tempIndex[bestmatch].addr;
		}
	}

	//free blocks from tail & merge
	int addrTail = tempAddrTail - 1;
	int mergeSize = tempIndex[addrTail].size + tempLeft;
	while (mergeSize < NumBytes)
	{
		removeFromList(addrTail);
		tempIndex[addrTail].prev = tempIndex[addrTail].next = -1;
		tempAddr2Index[((unsigned int)tempIndex[addrTail].addr - (unsigned int)tempMemory) >> TEMP_SIZE_SHIFT] = -1;
		tempIndex[addrTail].onDestroyFuncPtr((unsigned char*)tempIndex[addrTail].addr);
		tempIndex[addrTail].onDestroyFuncPtr = NULL;
		tempIndex[addrTail].addr = NULL;
		tempIndex[addrTail].size = -1;
		--addrTail;
		mergeSize += tempIndex[addrTail].size;
		tempIndex[addrTail].size = mergeSize;
	}
	tempLeft = mergeSize - NumBytes;
	tempIndex[addrTail].size = NumBytes;
	tempIndex[addrTail].onDestroyFuncPtr((unsigned char*)tempIndex[addrTail].addr);
	tempIndex[addrTail].onDestroyFuncPtr = onDestroyFuncPtr;
	tempAddrTail = addrTail + 1;
	//move to tail
	moveToTail(addrTail);
	return tempIndex[addrTail].addr;
}

bool visitMemTemp(unsigned int offset)
{
	short id = tempAddr2Index[(offset - (unsigned int)tempMemory) >> TEMP_SIZE_SHIFT];
	bool ret = id != -1;
	if (ret)
	{
		//move to tail
		moveToTail(id);
	}
	return ret;
}