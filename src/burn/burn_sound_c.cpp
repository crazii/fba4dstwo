#include "burnint.h"
#include "burn_sound.h"

#define CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

void BurnSoundCopyClamp_C(int *Src, short *Dest, int Len)
{
	if(Len&0x1)
	{
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
	}
	Len = Len >> 1;
	
	while (Len--) {
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
		
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
	}
}

void BurnSoundCopyClamp_Add_C(int *Src, short *Dest, int Len)
{
	if(Len&0x1)
	{
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
	}
	Len = Len >> 1;
	
	while (Len--) {
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
		
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
	}
}

void BurnSoundCopyClamp_Mono_C(int *Src, short *Dest, int Len)
{
	if(Len&0x1)
	{
		Dest[0] = CLIP((*Src >> 8));
		Dest[1] = CLIP((*Src >> 8));
		Dest+=2;
		Src++;
	}
	Len = Len >> 1;
	
	while (Len--) {
		Dest[0] = CLIP((*Src >> 8));
		Dest[1] = CLIP((*Src >> 8));
		Src++;
		
		Dest[2] = CLIP((*Src >> 8));
		Dest[3] = CLIP((*Src >> 8));
		Src++;
		
		Dest += 4;
	}
}

void BurnSoundCopyClamp_Mono_Add_C(int *Src, short *Dest, int Len)
{
	if(Len&0x1)
	{
		Dest[0] = CLIP((*Src >> 8) + Dest[0]);
		++Dest;
		Dest[1] = CLIP((*Src >> 8) + Dest[1]);
		++Dest;
		Src++;
	}
	Len = Len >> 1;
	
	while (Len--) {
		Dest[0] = CLIP((*Src >> 8) + Dest[0]);
		++Dest;
		Dest[1] = CLIP((*Src >> 8) + Dest[1]);
		++Dest;
		Src++;
		
		Dest[0] = CLIP((*Src >> 8) + Dest[0]);
		++Dest;
		Dest[1] = CLIP((*Src >> 8) + Dest[1]);
		++Dest;
		Src++;
	}
}

#undef CLIP
