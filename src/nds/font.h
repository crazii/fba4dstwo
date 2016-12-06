#ifndef _FONT_H_
#define _FONT_H_

#define R8G8B8_to_B5G6R5(c) ((c & 0xf80000) >> 19) | ((c & 0x00fc00) >> 5) | ((c & 0x0000f8) <<8)
//#define R8G8B8_to_B5G6R5(c) ((c & 0xf80000) >> 8) | ((c & 0x00fc00) >> 5) | ((c & 0x0000f8) >>3)

void drawString(const char *s, unsigned short *screenbuf, int x, int y, unsigned short c, int w = 0);
int getDrawStringLength(const char *s);

void drawRect(unsigned short *screenbuf, int x, int y, int w, int h, unsigned short c, unsigned char alpha = 0xFF);
void drawImage(unsigned short *screenbuf, int x, int y, int w,int h, unsigned short *imgBuf, int imgW, int imgH);

#endif	// _FONT_H_
