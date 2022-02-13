#ifndef FILL_MASK_H
#define FILL_MASK_H
//------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------
typedef struct{
	int x;
	int y;
	int plus;
	int minus;
	int pixel;
}FillMask;

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------
void fillMaskRectangle(Display* dpy, Window w, GC gc, int x, int y, int width, int height, FillMask mask);

//------------------------------------------------
#endif

