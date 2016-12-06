#include "burnint.h"
#include "konamiic.h"

unsigned int KonamiIC_K051960InUse = 0;
unsigned int KonamiIC_K052109InUse = 0;

void KonamiICReset()
{
	if (KonamiIC_K051960InUse) K051960Reset();
	if (KonamiIC_K052109InUse) K052109Reset();
}

void KonamiICExit()
{
	KonamiIC_K051960InUse = 0;
	KonamiIC_K052109InUse = 0;
	
	K051960Exit();
	K052109Exit();
}

void KonamiICScan(int nAction)
{
	if (KonamiIC_K051960InUse) K051960Scan(nAction);
	if (KonamiIC_K052109InUse) K052109Scan(nAction);
}
