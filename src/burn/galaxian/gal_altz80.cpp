#include "gal.h"

// This module is used to support using the MAME Z80 core in place of Doze
// This is mainly used when games use certain protection which Doze seems to have problems with

unsigned char GalUseAltZ80;

Z80_Regs Gal_Z80_0;
Z80_Regs Gal_Z80_1;

Gal_CPU_Config Gal_Z80_0_Config;
Gal_CPU_Config Gal_Z80_1_Config;

void GalOpenCPU(int nCPU)
{
	switch (nCPU) {
		case 0: {
			Z80SetContext(&Gal_Z80_0);
			
			Z80SetIOReadHandler(Gal_Z80_0_Config.Z80In);
			Z80SetIOWriteHandler(Gal_Z80_0_Config.Z80Out);
			Z80SetProgramReadHandler(Gal_Z80_0_Config.Z80Read);
			Z80SetProgramWriteHandler(Gal_Z80_0_Config.Z80Write);
			Z80SetCPUOpArgReadHandler(Gal_Z80_0_Config.Z80ReadOpArg);
			Z80SetCPUOpReadHandler(Gal_Z80_0_Config.Z80ReadOp);
			return;
		}
		
		case 1: {
			Z80SetContext(&Gal_Z80_1);
			
			Z80SetIOReadHandler(Gal_Z80_1_Config.Z80In);
			Z80SetIOWriteHandler(Gal_Z80_1_Config.Z80Out);
			Z80SetProgramReadHandler(Gal_Z80_1_Config.Z80Read);
			Z80SetProgramWriteHandler(Gal_Z80_1_Config.Z80Write);
			Z80SetCPUOpArgReadHandler(Gal_Z80_1_Config.Z80ReadOpArg);
			Z80SetCPUOpReadHandler(Gal_Z80_1_Config.Z80ReadOp);
			return;
		}
	}
}

void GalCloseCPU(int nCPU)
{
	switch (nCPU) {
		case 0: {
			Z80GetContext(&Gal_Z80_0);
			return;
		}
		
		case 1: {
			Z80GetContext(&Gal_Z80_1);
			return;
		}
	}
}
