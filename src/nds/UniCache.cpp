#include "UniCache.h"
#include "burner.h"

UniCacheIndex uniCacheIndex[CACHE_INDEX_SIZE];
char filePathName[256];
unsigned char *uniCacheHead = 0;
unsigned char *lastVisitedCacheBlock = 0;
SceUID cacheFile = -1;
unsigned int cacheFileSize = 0;
unsigned int totalMemBlocks = 0;
unsigned int requestAddrOffsetHigh = 0,requestAddrEndOffsetHigh=0, headBlockIndexOffsetHigh=0, magicFreeSpaceOffsetHigh=0;
unsigned int  requestAddrOffsetLow = 0;
unsigned short indexRecycleListHead=SHORT_INVALID_VALUE, indexRecycleListEnd=SHORT_INVALID_VALUE;
unsigned int uniCacheSpaceStatus[CACHE_SPACE_STATUS_SIZE]={0xffffffff,};
bool needCreateCache=false;
bool fillExtendData=false;
//test
//int debugValue1=0, debugValue2=0;
//void (*fillCacheDataFuncP)(unsigned char* freeCacheSpaceHead);

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
		if(cacheFile<0) 
			cacheFile = sceIoOpen( filePathName, PSP_O_RDONLY, 0777);
			
		sceIoLseek( cacheFile, blockOffset, SEEK_SET );
		sceIoRead( cacheFile, uniCacheHead+(uniCacheIndex[requestAddrOffsetHigh].cacheSpaceHeadOffsetHigh<<CACHE_INDEX_SHIFT), requestBlockCount<<CACHE_INDEX_SHIFT );
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

void initCacheStructure(float ratio)
{
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
	for (iCount=0;iCount<CACHE_INDEX_SIZE;iCount++)
	{
	    //set the call back func ptr to 0
		uniCacheIndex[iCount].onDestroyFuncPtr=0;
	}
	
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
		cacheFile = sceIoOpen( filePathName, PSP_O_RDONLY, 0777);
		if (cacheFile<0)
		{
			needCreateCache = true;
			cacheFile = sceIoOpen( filePathName, PSP_O_RDWR|PSP_O_CREAT, 0777 );
		}else if(sceIoLseek(cacheFile,0,SEEK_END)!=cacheSize)
		{
			needCreateCache = true;
			sceIoClose(cacheFile);
			cacheFile = sceIoOpen( filePathName, PSP_O_RDWR|PSP_O_TRUNC, 0777 );
		}
}
void destroyUniCache()
{
	destroyCacheStructure();
	sceIoClose( cacheFile );
	needCreateCache = false;
	cacheFile = -1;
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
void *mallocTemp(int NumBytes, void(*onDestroyFuncPtr)(unsigned char* addr))
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
	uniCacheIndex[requestAddrOffsetHigh].onDestroyFuncPtr=onDestroyFuncPtr;
	moveCurrentIndexToEnd(requestAddrOffsetHigh);
	
	return memPtr;
}

