#define bool short
#define true 1
#define false 0
#define SceUID int
#include "UniCache.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int testi,testj;
unsigned char uniCacheHead2[13*1024*1024];

int main()
{
       int i,j,k,l,m=0;
        cacheFileSize=0x4000000;
uniCacheHead=uniCacheHead2;
        memset(uniCacheHead, 0x3a,13*1024*1024);
        totalMemBlocks=13*1024/64;
        memset(uniCacheIndex,0xFF,sizeof(UniCacheIndex)*CACHE_INDEX_SIZE);
        memset(uniCacheSpaceStatus,0xFF,CACHE_SPACE_STATUS_SIZE*4);
        for (i=0;i<13*1024/64;i++)
        {
                //set the bit to 0
		uniCacheSpaceStatus[i>>5]&=~(1UL<<(i&0x1F));
        }
              
        indexRecycleListHead=SHORT_INVALID_VALUE; indexRecycleListEnd=SHORT_INVALID_VALUE;
        //fillCacheDataFuncP=fillCacheDataCu;
        //for(i=0;i<cacheFileSize;i=i+4)
        while(true)
        {
                m++;
                i=rand();
                i=((((i<<16)|rand())&0x7ffffff)>>4)<<4;
                j=rand();
                j=(((j<<16)|rand())&0x3fffff)/4;
                if(i+j>cacheFileSize) continue;
                //printf("rand:%d\n",rand());
                //j=1;
                testi=i;testj=j;
                unsigned long* test=(unsigned long*)getBlock(i,j);
                
                if(test==0)
                        printf("pointer is 0,i:%d,j:%d\n",i,j);
                else {
               mallocTemp(3*1024*1024);
                for( l=i,k=0;l<(i+j);l=l+4,k++)
                if(test[k]!=l)
                {
                        printf("value error.i=%u,\tl=%u,\t*test=%u\n",i,l,test[k]);
                        return 0;
                }
                }
                if(!(m&0x3FF))
                        printf("Used Mem:%d\tFree Mem:%d\n",getUsedMemSize(),getFreeMemSize());
        }             
        
        return 0;
}
