#ifndef FFONT_H
#define FFONT_H
//------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "FillMask.h"

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------
#define LAYOUT_LEFT 0
#define LAYOUT_CENTER 1
#define LAYOUT_RIGHT 2

#define FONT_CHARS_LENGTH 256

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------
typedef struct{
	wchar_t ch;
	int width;
	unsigned char map[64][64];
}CharFont;

typedef struct{
	int ver;
	int length;
	int height;
	int marge;
	CharFont chars[FONT_CHARS_LENGTH];
}FFont;

typedef struct{
	char ch[6];
	int width;
	unsigned char map[64][64];
}OldCharFont;

typedef struct{
	int length;
	int height;
	int marge;
	OldCharFont chars[FONT_CHARS_LENGTH];
}FOldFont;

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------
int FSaveFont(FFont* font, char fileName[]);
int FLoadFont(FFont* font, char fileName[]);
int FLoadOldFont(FFont* font, char fileName[]);
void FPrintAllChar(FFont font);
int getFontHeight(FFont font, int scale);
int getStringWidth(FFont font, char cs[], int scale, int margin);
void FSetLayout(int layout);
void FDrawChar(Display* dpy, Window w, GC gc, FFont font, wchar_t cw, int x, int y, int pixSize);
void FFillChar(Display* dpy, Window w, GC gc, FFont font, wchar_t cw, int x, int y, int pixSize);
void FFillMaskChar(Display* dpy, Window w, GC gc, FFont font, wchar_t cw, int x, int y, int pixSize, FillMask mask);
void FDrawString(Display* dpy, Window w, GC gc, FFont font, char c[], int x, int y, int pixSize, int addMarge);
void FFillString(Display* dpy, Window w, GC gc, FFont font, char* cs, int x, int y, int pixSize, int addMarge);
void FFillMaskString(Display* dpy, Window w, GC gc, FFont font, char cs[], int x, int y, int pixSize, int addMarge, FillMask mask);

//------------------------------------------------
#endif

