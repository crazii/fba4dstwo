

#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <psputility_netmodules.h>
#include <pspwlan.h>
#include <pspkernel.h>
#include <string.h>
#include "burner.h"
#include "psp.h"
#include "pspadhoc.h"

#define PACKET_LENGTH 24
#define MAX_PACKET_LENGTH 25600

extern unsigned int nCurrentFrame;
extern SceUID sendThreadSem,recvThreadSem;
extern unsigned char forceDelay;

static int pdpId = 0;
static unsigned char g_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static SceUID recvThreadId=0, sendThreadId=0;
//static int pdpStatLength=20;
unsigned int inputKeys[3][3]={{0,},};
static unsigned int recvBuffer[MAX_PACKET_LENGTH/4];
static char adhocInited=0;
static char needSync=0;
static unsigned char otherPspCount=3;
static unsigned int macBuf[2];
static unsigned int macList[3][2]={{0,0},};
static unsigned char* mac=(unsigned char*)macBuf+2;
inline static bool hasPspNotRecved()
{
	for(int i=0;i<otherPspCount;i++)
	{
		if(macList[i][1]==0)
			return true;
	}
	return false;
}

inline static void newPspJoined()
{
	if(otherPspCount<3)
	{		
		macList[otherPspCount][0]=macBuf[1];
		otherPspCount++;
	}
}
inline static void checkPspList()
{
	for(int i=0;i<otherPspCount;i++)
	{
		if(macList[i][0]==0)
			macList[i][0]=macBuf[1];
		if(macList[i][0]==macBuf[1])
		{
			macList[i][1]++;
			return;
		}
	}
	newPspJoined();
	
}
void removePspFromList()
{
	if(otherPspCount>1)
	{
		otherPspCount--;
		for(int i=0;i<otherPspCount;i++)
		{
			macList[i][0]=0;
			macList[i][1]=0;
		}
	}
}

void clearMacRecvCount()
{
	for(int i=0;i<otherPspCount;i++)
	{
		macList[i][1]=0;
	}
}
void sendSyncGame()
{
	needSync=1;
	wifiSend(WIFI_CMD_SYNC_REQ);
}

int wifiRecv()
{
	unsigned short port;
	
	
	unsigned int recvMaxFrame=0;
	unsigned char currentInput=nCurrentFrame&1U;
	int i,j,err,length;
	unsigned char *Def = NULL;

	while(adhocInited)
	{		
		length=MAX_PACKET_LENGTH;
		err = sceNetAdhocPdpRecv(pdpId,
					mac,
					&port,
					(unsigned char*)recvBuffer,
					&length,
					0,	// 0 in lumines
					1);	// 1 in lumines
		if(err==0&&length==PACKET_LENGTH)
		{
			if(recvBuffer[2]==0)
			{
				switch(recvBuffer[1])
				{
					case WIFI_CMD_RESET:
						resetGame();
						return 0;
					case WIFI_CMD_SYNC_REQ:
						err = BurnStateCompress(&Def, &length, 1);		// Compress block from driver and return deflated buffer
						if (Def == NULL) {
							continue;
						}
						recvBuffer[1]=WIFI_CMD_SYNC_RESP;
						recvBuffer[2]=0;
						recvBuffer[4]=length;
						recvBuffer[5]=nCurrentFrame;
						sceNetAdhocPdpSend(pdpId,
							&mac[0],
							0x309,
							&recvBuffer[0],
							PACKET_LENGTH,
							0,	// 0 in lumines
							1);	// 1 in lumines
						sceKernelDelayThread(3000);
						sceNetAdhocPdpSend(pdpId,
							&mac[0],
							0x309,
							&recvBuffer[0],
							PACKET_LENGTH,
							0,	// 0 in lumines
							1);	// 1 in lumines
						sceKernelDelayThread(3000);
						sceNetAdhocPdpSend(pdpId,
							&mac[0],
							0x309,
							&recvBuffer[0],
							PACKET_LENGTH,
							0,	// 0 in lumines
							1);	// 1 in lumines
						sceKernelDelayThread(5000);
						if(length<=MAX_PACKET_LENGTH)
						{
							sceNetAdhocPdpSend(pdpId,
								&mac[0],
								0x309,
								&Def[0],
								length,
								0,	// 0 in lumines
								1);	// 1 in lumines
							sceKernelDelayThread(10000);
							sceNetAdhocPdpSend(pdpId,
								&mac[0],
								0x309,
								&Def[0],
								length,
								0,	// 0 in lumines
								1);	// 1 in lumines
						}
						else
						{
							sceNetAdhocPdpSend(pdpId,
								&mac[0],
								0x309,
								&Def[0],
								MAX_PACKET_LENGTH,
								0,	// 0 in lumines
								1);	// 1 in lumines
							sceKernelDelayThread(10000);
							sceNetAdhocPdpSend(pdpId,
								&mac[0],
								0x309,
								&Def[MAX_PACKET_LENGTH],
								length-MAX_PACKET_LENGTH,
								0,	// 0 in lumines
								1);	// 1 in lumines
							sceKernelDelayThread(10000);
							sceNetAdhocPdpSend(pdpId,
								&mac[0],
								0x309,
								&Def[MAX_PACKET_LENGTH],
								length-MAX_PACKET_LENGTH,
								0,	// 0 in lumines
								1);	// 1 in lumines

						}
						sceKernelDelayThread(20000);
						free(Def);
						Def=NULL;							
						break;
					case WIFI_CMD_SYNC_RESP:
						if(needSync)
						{
							length=recvBuffer[4];							
							Def=(unsigned char*)memalign(4,length);
							if (Def == NULL) {
								continue;
							}
							
							for(i=0,j=0;i<256;i++)
							{
								
								length=recvBuffer[4];
								err = sceNetAdhocPdpRecv(pdpId,
									mac,
									&port,
									&Def[j],
									&length,
									0,	// 0 in lumines
									1);	// 1 in lumines
								

								if(err==0)
								{
								
									
									if(length==recvBuffer[4])
									{
										
										BurnStateDecompress(Def, length+j, 1);
										nCurrentFrame=recvBuffer[5];
										needSync=0;
										free(Def);
										Def=NULL;
										memset(inputKeys,0,36);
										inputKeys[currentInput][2]=nCurrentFrame;
										inputKeys[currentInput][5]=nCurrentFrame-1;
										return -1;
									}else if(length==MAX_PACKET_LENGTH&&recvBuffer[4]>MAX_PACKET_LENGTH)
									{
										recvBuffer[4]=recvBuffer[4]-MAX_PACKET_LENGTH;
										j=MAX_PACKET_LENGTH;
									}
								}
								sceKernelDelayThread(5000);
							}
							if(Def)
							{
								free(Def);
								Def=NULL;
							}
							
						}
						break;
					default:
						checkPspList();
						inputKeys[currentInput][0]=inputKeys[currentInput][0]|recvBuffer[0];
						inputKeys[currentInput][1]=inputKeys[currentInput][1]|recvBuffer[1];
				}
			}else if(recvBuffer[2]==nCurrentFrame)
			{
				checkPspList();
				inputKeys[currentInput][0]=inputKeys[currentInput][0]|recvBuffer[0];
				inputKeys[currentInput][1]=inputKeys[currentInput][1]|recvBuffer[1];
			}else if(recvBuffer[2]==nCurrentFrame+1)
			{
				checkPspList();
				//if(recvBuffer[5]==nCurrentFrame)
				{	
					inputKeys[currentInput][0]=inputKeys[currentInput][0]|recvBuffer[3];
					inputKeys[currentInput][1]=inputKeys[currentInput][1]|recvBuffer[4];
				}
				/*
				else
				{
					needSync=1;
					wifiSend(WIFI_CMD_SYNC_REQ);
					return -1;
				}
				*/
			}
			
			
		}else
		{
			break;
		}
			
	}
	if(hasPspNotRecved())
	{
		return -1;
	}
	return 0;
}

int wifiSend(unsigned int wifiCMD)
{
	unsigned char currentInput;
	
	if(wifiCMD)
	{
		recvBuffer[1]=wifiCMD;
		recvBuffer[2]=0;

		sceNetAdhocPdpSend(pdpId,
			&g_mac[0],
			0x309,
			&recvBuffer[0],
			PACKET_LENGTH,
			0,	// 0 in lumines
			1);	// 1 in lumines
		sceKernelDelayThread(5000);
		/*
		sceNetAdhocPdpSend(pdpId,
			&g_mac[0],
			0x309,
			&recvBuffer[0],
			PACKET_LENGTH,
			0,	// 0 in lumines
			1);	// 1 in lumines
		sceKernelDelayThread(5000);
		sceNetAdhocPdpSend(pdpId,
			&g_mac[0],
			0x309,
			&recvBuffer[0],
			PACKET_LENGTH,
			0,	// 0 in lumines
			1);	// 1 in lumines
		sceKernelDelayThread(5000);
		*/
	}else{
		//sceKernelWaitSema(sendThreadSem, 1, 0);
		currentInput=nCurrentFrame&1U;
		
		//if(inputKeys[currentInput][0]!=0||inputKeys[currentInput][1]!=0)
		{
			//inputKeys[currentInput][2]=nCurrentFrame;
			//inputKeys[currentInput][3]=inputKeys[currentInput][0]+inputKeys[currentInput][1]+inputKeys[currentInput][2];
			if(currentInput==1)
			{
				inputKeys[2][0]=inputKeys[0][0];
				inputKeys[2][1]=inputKeys[0][1];
				inputKeys[2][2]=inputKeys[0][2];
			}
			sceNetAdhocPdpSend(pdpId,
			&g_mac[0],
			0x309,
			&inputKeys[currentInput],
			PACKET_LENGTH,
			0,	// 0 in lumines
			1);	// 1 in lumines
			//sceKernelDelayThread(3000);
		}
	}
	
	return 0;
}


int adhocLoadDrivers()
{

	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);         // AHMAN
	sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC);          // AHMAN

	return 0;
}

int adhocInit(char* netWorkName)
{
	if ( netWorkName==0||adhocInited)
		return -1;
	otherPspCount=4;
	adhocInited=1;	
	u8 macTemp[6];
	struct productStruct product;

	strcpy(product.product, "ULUS99999");
	product.unknown = 0;

    u32 err;
	////printf2("sceNetInit()\n");
    err = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000);
    if (err != 0)
        return err;
	//g_NetInit = true;

	//printf2("sceNetAdhocInit()\n");
    err = sceNetAdhocInit();
    if (err != 0)
        return err;
	//g_NetAdhocInit = true;

	//printf2("sceNetAdhocctlInit()\n");
    err = sceNetAdhocctlInit(0x2000, 0x20, &product);
    if (err != 0)
        return err;
	//g_NetAdhocctlInit = true;

    // Connect
    
    //printf2("sceNetAdhocctlConnect()\n");
    err = sceNetAdhocctlConnect(netWorkName);
    if (err != 0)
        return err;
	//g_NetAdhocctlConnect = true;

    int stateLast = -1;
    //printf2("Connecting...\n");
    int i;
    for(i=0;i<100;i++)
    {
        int state;
        err = sceNetAdhocctlGetState(&state);
        if (err != 0)
        {
        	//pspDebugScreenInit();
            //printf("sceNetApctlGetState returns $%x\n", err);
            sceKernelDelayThread(10*1000000); // 10sec to read before exit
			return -1;
        }
        if (state > stateLast)
        {
            //printf2("  connection state %d of 1\n", state);
            stateLast = state;
        }
        if (state == 1)
            break;  // connected

        // wait a little before polling again
        sceKernelDelayThread(50*1000); // 50ms
    }
    if(i>=100) return -1;
    
    //printf2("Connected!\n");
  
    
	sceWlanGetEtherAddr(macTemp);
	
    //printf2("sceNetAdhocPdpCreate\n");
    
    
	pdpId = sceNetAdhocPdpCreate(macTemp,
		     0x309,		// 0x309 in lumines
		     MAX_PACKET_LENGTH, 	// 0x400 in lumines
		     0);		// 0 in lumines
	if(pdpId <= 0)
	{
		
		//pspDebugScreenInit();
		//printf("pdpId = %x\n", pdpId);
		return -1;
	}

	return 0;
}


int adhocTerm()
{
    if(adhocInited==0) return -1;
    
    u32 err;

	{
		////printf2("sceNetAdhocPdpDelete\n");
		err = sceNetAdhocPdpDelete(pdpId,0);
		if(err != 0)
		{
			//pspDebugScreenInit();
			//printf("sceNetAdhocPdpDelete returned %x\n", err);
		}

	}

	sceNetAdhocctlDisconnect();
	
	{
		//printf2("sceNetAdhocctlTerm\n");
		err = sceNetAdhocctlTerm();
		if(err != 0)
		{
			//pspDebugScreenInit();
			//printf("sceNetAdhocctlTerm returned %x\n", err);
		}
		//g_NetAdhocctlInit = false;
	}

	{
		////printf2("sceNetAdhocTerm\n");
		err = sceNetAdhocTerm();
		if(err != 0)
		{
			//pspDebugScreenInit();
			//printf("sceNetAdhocTerm returned %x\n", err);
		}
		//g_NetAdhocInit = false;
	}


	{
		////printf2("sceNetTerm\n");
		err = sceNetTerm();
		if(err != 0)
		{
			//pspDebugScreenInit();
			//printf("sceNetTerm returned %x\n", err);
		}
		//g_NetInit = false;
	}
	adhocInited=0;
    return 0; // assume it worked
}

