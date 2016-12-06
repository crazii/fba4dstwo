#include "burnint.h"
#include "psp.h"
#include "pspadhoc.h"
struct GameInp {
	unsigned char *pVal;  // Destination for the Input Value
	unsigned char nType;  // 0=binary (0,1) 1=analog (0x01-0xFF) 2=dip switch
	unsigned char nConst;
	unsigned char nop[2]; // 
	int nBit;   // bit offset of Keypad data
};
struct keyDef
{
	const char* keyString;
	unsigned char keyValue;
};
#define KEY_DEF_ARRAY_SIZE 12
static keyDef keyDefArray[KEY_DEF_ARRAY_SIZE]={
	{"coin",	0x0},
	{"start",	0x3},
	{"up",		0x4},
	{"right",	0x5},
	{"down",	0x6},
	{"left",	0x7},
	{"fire 1",	0xE},
	{"fire 2",	0xD},
	{"fire 3",	0xF},
	{"fire 4",	0xC},
	{"fire 5",	0x9},
	{"fire 6",	0x8}
};
// Mapping of PC inputs to game inputs
struct GameInp *GameInp = NULL;
unsigned int nGameInpCount = 0;
short currentInp=1;
static bool bInputOk = false;

static unsigned char hex2int(char c)
{
	switch( c ){
	case '1': return  1;
	case '2': return  2;
	case '3': return  3;
	case '4': return  4;
	case '5': return  5;
	case '6': return  6;
	case '7': return  7;
	case '8': return  8;
	case '9': return  9;
	case 'a': 
	case 'A': return 10;
	case 'b': 
	case 'B': return 11;
	case 'c': 
	case 'C': return 12;
	case 'd': 
	case 'D': return 13;
	case 'e': 
	case 'E': return 14;
	case 'f': 
	case 'F': return 15;
//	case '0': return  0;
	default : return  0;
	}
}

void loadDefaultInput()
{
	//Read customized input info from ini file.
	
	monoSound=0;
	gameSpeedCtrl=1;
	screenMode=0;
	FILE * fp = fopen("fba4psp.ini", "r");
	
	char IniLine[256];
	char FindName[32]={'\0',};
	if(fp)
	{
		fseek(fp, 0, SEEK_SET);
		while ( fgets( IniLine, 255, fp ) )
		{
			char *p;
			if(IniLine[0]=='#'||(IniLine[0]=='/'&&IniLine[1]=='/')) continue; 
			if (p = strstr(IniLine, "input")) 
			{
				p = strstr(p, "\"");
				p++;
				strncpy(FindName,p,30);
				p = strstr(FindName, "\"");
				if ( p)
				{
					*p='\0';
				}
				
				p = strstr(IniLine, "switch 0x");
				if(!p){
					continue;
				}
				for (int i=0; i<KEY_DEF_ARRAY_SIZE; i++)
	  			{
	   			    if (strstr(FindName, keyDefArray[i].keyString))
					{	
			    		keyDefArray[i].keyValue  = hex2int(p[9]);
				    }   	
				  
	  			}
			}else if(p = strstr(IniLine, "gameSpeedCtrl"))
			{
				p = strstr(p, "0x");
				if(p==0||p[2]=='\0')
					continue;
				int value=hex2int(p[2]);
				if(value>=0&&value<=8)
					gameSpeedCtrl=value;
			}
			else if(p = strstr(IniLine, "hotButtons"))
			{
				int value=0;
				hotButtons=0;
				while(p = strstr(p, "0x"))
				{
					p=p+2;
					value=hex2int(*p);
					hotButtons=hotButtons|(0x1<<value);
				}
				
			}else if(p = strstr(IniLine, "screenMode"))
			{
				p = strstr(p, "0x");
				if(p==0||p[2]=='\0')
					continue;
				int value=hex2int(p[2]);
				if(value>=0&&value<=8)
					screenMode=value;
			}else if(p = strstr(IniLine, "gameScreenWidth"))
			{
				p = strstr(p, " ");
				if(p==0||p[1]=='\0')
					continue;
				do{
					p++;
				}while(*p==' ');
				int value=0;
				for(int i=0;i<3&&*p>='0'&&*p<='9';i++)
				{
					value=value*10+*p-'0';
					p++;
				}		
				
				if(value>0&&value<=SCREEN_WIDTH)
					gameScreenWidth=value;
			}else if(p = strstr(IniLine, "gameScreenHeight"))
			{
				p = strstr(p, " ");
				if(p==0||p[1]=='\0')
					continue;
				do{
					p++;
				}while(*p==' ');
				int value=0;
				for(int i=0;i<3&&*p>='0'&&*p<='9';i++)
				{
					value=value*10+*p-'0';
					p++;
				}		
				
				if(value>0&&value<=SCREEN_HEIGHT)
					gameScreenHeight=value;
			}
		}
		fclose(fp);
	}			
}

int DoInputBlank(int bDipSwitch)
{
  unsigned int i=0; 
  struct GameInp *pgi = NULL;
  // Reset all inputs to undefined (even dip switches, if bDipSwitch==1)
  if (GameInp==NULL) return 1;
  int bVert;
  if(screenMode<2||(screenMode>3&&screenMode<6)||screenMode>7)
  	bVert = BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL;
  else
  	bVert = (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)?0:BDF_ORIENTATION_VERTICAL;

  // Get the targets in the library for the Input Values
  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    struct BurnInputInfo bii;
    memset(&bii,0,sizeof(bii));
    BurnDrvGetInputInfo(&bii,i);
    
    pgi->nType = bii.nType; // store input type
    pgi->pVal  = bii.pVal;  // store input pointer to value
    pgi->nBit  = -1;

    //if ( (bDipSwitch==0) && (bii.nType==2) ) {
    //	continue; // Don't blank the dip switches
    //}

    if (pgi->nType == BIT_DIGITAL) 
    {  	if (strcmp(bii.szInfo, "reset") == 0)
	    {	
	    	pgi->nBit  = 31;
	    	continue;
	    }   	
    	if(currentInp!=0&&bii.szInfo[1]-'0'!=currentInp)
    	{
    		for(int i=0;i<=11;i++)
    		{
    			if (strcmp(bii.szInfo+3, keyDefArray[i].keyString) == 0)		
	    			pgi->nBit  = 127; //invalid Value
    		}
    	}else{
    	// map my keypad def

	    if (strcmp(bii.szInfo+3, keyDefArray[0].keyString) == 0)		// PSP_CTRL_SELECT = 0x000001,
	    	pgi->nBit  = keyDefArray[0].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[1].keyString) == 0)	// PSP_CTRL_START = 0x000008,
	    	pgi->nBit  = keyDefArray[1].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[2].keyString) == 0)		//PSP_CTRL_UP = 0x000010,
	    	pgi->nBit  = (bVert) ? keyDefArray[3].keyValue : keyDefArray[2].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[4].keyString) == 0)		// PSP_CTRL_DOWN = 0x000040,
	    	pgi->nBit  = (bVert) ? keyDefArray[5].keyValue : keyDefArray[4].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[5].keyString) == 0)		// PSP_CTRL_LEFT = 0x000080,
	    	pgi->nBit  = (bVert) ? keyDefArray[2].keyValue : keyDefArray[5].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[3].keyString) == 0)	// PSP_CTRL_RIGHT = 0x000020,
	    	pgi->nBit  = (bVert) ? keyDefArray[4].keyValue : keyDefArray[3].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[6].keyString) == 0)	// PSP_CTRL_CROSS = 0x004000,
	    	pgi->nBit  = keyDefArray[6].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[7].keyString) == 0)	// PSP_CTRL_CIRCLE = 0x002000,
	    	pgi->nBit  = keyDefArray[7].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[8].keyString) == 0)	// PSP_CTRL_SQUARE = 0x008000
	    	pgi->nBit  = keyDefArray[8].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[9].keyString) == 0)	// PSP_CTRL_TRIANGLE = 0x001000,
	    	pgi->nBit  = keyDefArray[9].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[10].keyString) == 0)	// PSP_CTRL_RTRIGGER = 0x000200
	    	pgi->nBit  = keyDefArray[10].keyValue;
	    else
	    if (strcmp(bii.szInfo+3, keyDefArray[11].keyString) == 0)	// PSP_CTRL_LTRIGGER = 0x000100,
	    	pgi->nBit  = keyDefArray[11].keyValue;
    	}
		    	

	} else
	if (pgi->nType == BIT_DIPSWITCH) 
	{  // black DIP switch
	
	//	if (pgi->pVal)
	//		* (pgi->pVal) = 0;
	
    }

#if 0
if (pgi->pVal != NULL)
	printf("GI(%02d): %-12s 0x%02x 0x%02x %-12s, [%d]\n", i, bii.szName, bii.nType, *(pgi->pVal), bii.szInfo, pgi->nBit );
else
	printf("GI(%02d): %-12s 0x%02x N/A  %-12s, [%d]\n", i, bii.szName, bii.nType, bii.szInfo, pgi->nBit );
#endif
   
  }
  	//Read customized input info from ini file.
	char InputDIPConfigFileName[256];
	sprintf(InputDIPConfigFileName, "roms/%s.ini", BurnDrvGetTextA(DRV_NAME));
	FILE * fp = fopen(InputDIPConfigFileName, "r");
	
	char IniLine[256];
	char FindName[32]={'\0',};
	if(fp)
	{
		fseek(fp, 0, SEEK_SET);
		while ( fgets( IniLine, 255, fp ) )
		{
			char *p;
			if(IniLine[0]=='#'||(IniLine[0]=='/'&&IniLine[1]=='/')) continue; 
			if (p = strstr(IniLine, "input")) 
			{
				p = strstr(p, "\"");
				p++;
				strncpy(FindName,p,30);
				p = strstr(FindName, "\"");
				if ( p)
				{
					*p='\0';
				}
				
				p = strstr(IniLine, "switch 0x");
				if(!p){
					continue;
				}
				for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
	  			{
				    struct BurnInputInfo bii;
				    memset(&bii,0,sizeof(bii));
				    BurnDrvGetInputInfo(&bii,i);
	   			    if (pgi->nType == BIT_DIGITAL) 
				    {  	if (strcmp(bii.szInfo, FindName) == 0)
					    {	
					    	pgi->nBit  = hex2int(p[9]);
					    }   	
				  
					}
	  			}
			}else if(p = strstr(IniLine, "gameSpeedCtrl"))
			{
				p = strstr(p, "0x");
				if(p==0||p[2]=='\0')
					continue;
				int value=hex2int(p[2]);
				if(value>=0&&value<=8)
					gameSpeedCtrl=value;
			}
			else if(p = strstr(IniLine, "hotButtons"))
			{
				int value=0;
				hotButtons=0;
				while(p = strstr(p, "0x"))
				{
					p=p+2;
					value=hex2int(*p);
					hotButtons=hotButtons|(0x1<<value);
				}
				
			}else if(p = strstr(IniLine, "screenMode"))
			{
				p = strstr(p, "0x");
				if(p==0||p[2]=='\0')
					continue;
				int value=hex2int(p[2]);
				if(value>=0&&value<=8)
					screenMode=value;
			}else if(p = strstr(IniLine, "monoSound"))
			{
				p = strstr(p, "0x");
				if(p==0||p[2]=='\0')
					continue;
				int value=hex2int(p[2]);
				monoSound=value;
			}else if(p = strstr(IniLine, "gameScreenWidth"))
			{
				p = strstr(p, " ");
				if(p==0||p[1]=='\0')
					continue;
				do{
					p++;
				}while(*p==' ');
				int value=0;
				for(int i=0;i<3&&*p>='0'&&*p<='9';i++)
				{
					value=value*10+*p-'0';
					p++;
				}		
				
				if(value>0&&value<=SCREEN_WIDTH)
					gameScreenWidth=value;
			}else if(p = strstr(IniLine, "gameScreenHeight"))
			{
				p = strstr(p, " ");
				if(p==0||p[1]=='\0')
					continue;
				do{
					p++;
				}while(*p==' ');
				int value=0;
				for(int i=0;i<3&&*p>='0'&&*p<='9';i++)
				{
					value=value*10+*p-'0';
					p++;
				}		
				
				if(value>0&&value<=SCREEN_HEIGHT)
					gameScreenHeight=value;
			}
		}
		fclose(fp);
	}		
  	return 0;
}

int InpInit()
{
  unsigned int i=0; 
  int nRet=0;
  bInputOk = false;
  // Count the number of inputs
  nGameInpCount=0;
  for (i=0;i<0x1000;i++) {
    nRet = BurnDrvGetInputInfo(NULL,i);
    if (nRet!=0) {   // end of input list
    	nGameInpCount=i; 
    	break; 
    }
  }
  
  // Allocate space for all the inputs
  GameInp = (struct GameInp *)malloc(nGameInpCount * sizeof(struct GameInp));
  if (!GameInp) return 1;
  memset(GameInp, 0, nGameInpCount * sizeof(struct GameInp));
 
  DoInputBlank(1);
  bInputOk = true;
  
  return 0;
}

int InpExit()
{
  if (GameInp!=NULL) free(GameInp);
  GameInp = NULL;
  nGameInpCount = 0;
  bInputOk = false;
  return 0;
}

int InpMake(unsigned int key)
{
	if (!bInputOk) return 1;

#if 0	
	static int skip = 0;
	skip ++;
	if (skip > 2) skip = 0;
	if (skip != 1) return 1;
#endif
	
	unsigned int i=0;
	unsigned int down = 0;
	unsigned char currentInput,lastInput; 
	currentInput=nCurrentFrame&1U;
	lastInput=1-currentInput;
	
	inputKeys[currentInput][0]=0;
	inputKeys[currentInput][1]=0;
	inputKeys[currentInput][2]=nCurrentFrame;
	for (i=0; i<nGameInpCount; i++) {
		if (GameInp[i].pVal == NULL) continue;
		
		if ( GameInp[i].nBit >= 0 ) 
		{
			down = key & (1U << GameInp[i].nBit);
			if(i<32)
			{
				if(down)
				{	
					inputKeys[currentInput][0]=inputKeys[currentInput][0]|(1U <<i);		
				}
				down=inputKeys[lastInput][0]&(1U <<i);
			}else
			{
				int j=i-32;
				if(down)
				{	
					inputKeys[currentInput][1]=inputKeys[currentInput][1]|(1U <<j);		
				}
				down=inputKeys[lastInput][1]&(1U <<j);		
			}
			
			
			if (GameInp[i].nType!=1) {
				// Set analog controls to full
				if (down) *(GameInp[i].pVal)=0xff; else *(GameInp[i].pVal)=0x01;
			} else {
				// Binary controls
				if (down) *(GameInp[i].pVal)=1;    else *(GameInp[i].pVal)=0;
			}

			//if ( *(GameInp[i].pVal) != 0 ) printf("down key %08x (%d %d)\n", key, i, *(GameInp[i].pVal));
		} else {
			// dip switch ...
			
			*(GameInp[i].pVal) = GameInp[i].nConst;
		}
	}
	return 0;
}

void InpDIPSetOne(int dipOffset, struct BurnDIPInfo * bdi)
{
	struct GameInp* pgi = GameInp + bdi->nInput + dipOffset;
	
	printf( "0x%p -> 0x%02x  \n", pgi->pVal, *( pgi->pVal) );
	if ( pgi->pVal ) {
		*(pgi->pVal) &= ~(bdi->nMask);
		*(pgi->pVal) |= (bdi->nSetting & bdi->nMask);
	}
	printf( "0x%p -> 0x%02x  \n", pgi->pVal, *( pgi->pVal) );
	//struct BurnInputInfo bii;
	//memset(&bii,0,sizeof(bii));
    //if (BurnDrvGetInputInfo(&bii, bdi->nInput + dipOffset) == 0) {
    	//printf("%s\n", bii.szName);
    //}
	//printf(" DIP Offset 0x%02x\n", bdi->nInput + dipOffset);
}



void InpDIP()
{
	
	struct BurnDIPInfo bdi;
	struct GameInp* pgi;
	struct BurnInputInfo bii;
	
	int i;
	
	// get dip switch offset 
	int nDIPOffset = 0;
	for (i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
		if (bdi.nFlags == 0xF0) {
			nDIPOffset = bdi.nInput;
			break;
		}
	//printf("nDIPOffset = %d\n", nDIPOffset);
	
	char InputDIPConfigFileName[256];
	sprintf(InputDIPConfigFileName, "roms/%s.ini", BurnDrvGetTextA(DRV_NAME));
	FILE * fp = fopen(InputDIPConfigFileName, "r");
	
	char IniLine[256];
	char FindName[64];

	i = 0;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			pgi->nConst = (pgi->nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			
			BurnDrvGetInputInfo(&bii, bdi.nInput + nDIPOffset);
			sprintf(FindName, "\"%s\"", bii.szName);
			
			if (fp) {
				fseek(fp, 0, SEEK_SET);
				while ( fgets( IniLine, 255, fp ) )
							
					if (strstr(IniLine, FindName)) {
						char *p = strstr(IniLine, "constant 0x");
						if ( p )
							pgi->nConst = ((hex2int(p[11]) << 4) | hex2int(p[12])) & bdi.nMask;
					}
			}
			
		}
		i++;
	}
	
	if (fp) fclose(fp);
	
	InpMake( 0 );

#if 0	
	bool bContinue = false;
	bool bFreePlay = false;
	
	// find dip need set to default
	i = 0;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if ((bdi.nFlags & 0xF0) == 0xF0) {
		   	if (bdi.nFlags == 0xFE || bdi.nFlags == 0xFD) {
		   		if (bdi.szText) {
		   			
		   			printf("DIP: %3d %s\n", i, bdi.szText);
		   			
		   			if (strcmp(bdi.szText, "Continue") == 0)
		   				//bContinue = true;
		   				;
		   			else
		   			if (strcmp(bdi.szText, "Free play") == 0)
		   				// neogeo free play disable
		   				//bFreePlay = true;
		   				;
		   			else
		   			;
		   		} else printf("DIP: %3d [null]\n", i);
			}
			i++;
		} else {
			if ( bdi.szText ) {
				
				printf("     %3d %s", i, bdi.szText);
				
				struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;
				
				if ( pgi->pVal ) {
					
					if ((*(pgi->pVal) & bdi.nMask) == bdi.nSetting )
						printf(" <<<");
				
				}
				
				
				printf("\n");
				
				if (bFreePlay && (strcmp(bdi.szText, "Off") == 0)) {
					// printf(" --- Free play set to 'Off'\n");
					InpDIPSetOne( nDIPOffset, &bdi );
					bFreePlay = false;
				} else
				if (bContinue && (strcmp(bdi.szText, "On") == 0)) {
					// printf(" --- Continue set to 'On'\n");
					InpDIPSetOne( nDIPOffset, &bdi );
					bContinue = false;
				}
				
			}
		
			i += (bdi.nFlags & 0x0F);
		}
	}
#endif
}

