
#include <pspiofilemgr.h>

#include <stdio.h>
#include <string.h>

#include "psp.h"
#include "burnint.h"

#define MAX_ROM_COUNT	512

typedef struct rom_ent {
	char name[256];
	int stat;
} ROM_FILE_ENT;

static ROM_FILE_ENT * pRom_ent = NULL;
static int rom_count = 0;
static SceIoDirent fi;

static int FindDrvInfoByName(char * fn)
{
	char sfn[256];
	strcpy( sfn, fn );
	sfn[strlen(fn)-4] = 0;
	for (nBurnDrvSelect=0; nBurnDrvSelect<nBurnDrvCount; nBurnDrvSelect++) 
		if ( stricmp(sfn, BurnDrvGetTextA(DRV_NAME)) == 0 )
			return nBurnDrvSelect;
	return -1;
}

static int isValidFile(SceIoDirent * pfd, ROM_FILE_ENT * pre)
{
	if (pfd->d_stat.st_attr == FIO_SO_IFDIR) {
		if ( strcmp(".", pfd->d_name) == 0 ) return 0;
		if ( pre ) {
			strcpy( pre->name, pfd->d_name );
			pre->stat = -1;
		}
		return 2;
	}
	char * ext = strrchr(pfd->d_name, '.');
	if ( stricmp(ext, ".zip") ) return 0;
	
	if ( pre ) {
		strcpy( pre->name, pfd->d_name );
		pre->stat = FindDrvInfoByName( pfd->d_name );;
		if ( (unsigned int)pre->stat >= nBurnDrvCount ) pre->stat = -2;
	}
	return 1;
}

static int findloop( ROM_FILE_ENT * pre )
{
	int count = 0;
	
	SceUID fd = sceIoDopen(ui_current_path);
	if (fd > 0) {
		memset(&fi, 0, sizeof(fi));
		while ( sceIoDread( fd, &fi ) > 0 ) {
			if ( isValidFile(&fi, pre) ) {
				count++;
				if (pre) pre++;
			}
			memset(&fi, 0, sizeof(fi));
		}
		sceIoDclose(fd);
	}
	return count;
}

int findRomsInDir(bool force)
{
#if 0
	if ( force && pRom_ent ) {
		free( pRom_ent );
		pRom_ent = NULL;
	}
	if ( pRom_ent ) return rom_count;
	rom_count = findloop( NULL );	
	if ( rom_count ) {
		pRom_ent = (ROM_FILE_ENT *) malloc ( rom_count * sizeof(ROM_FILE_ENT) );
		if (!pRom_ent) {
			rom_count = 0;
			return 0; // error alloc memory
		}
		rom_count = findloop( pRom_ent );
	}
	return rom_count;
#else
	// alloc enough memory now !!! for PSP doesn't have TLB or MMU
	if (!pRom_ent) pRom_ent = (ROM_FILE_ENT *) malloc ( MAX_ROM_COUNT * sizeof(ROM_FILE_ENT) );
	if (force || (rom_count <= 0))
		rom_count = findloop( pRom_ent );
	return rom_count;
#endif
}

char * getRomsFileName(int idx)
{
	if (idx >= rom_count) return NULL;
	return &(pRom_ent[idx].name[0]);
}

int getRomsFileStat(int idx)
{
	if (idx >= rom_count) return -1;
	return pRom_ent[idx].stat;
}
