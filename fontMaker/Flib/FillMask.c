#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#include "FillMask.h"

void fillMaskRectangle(Display* dpy, Window w, GC gc, int x, int y, int width, int height, FillMask mask){
	for(int i = y; i < y + height; i++){
		int p = i/mask.pixel;
		for(int j = x; j < x + width; j++){
			int q = j/mask.pixel;
			if(q%mask.x == 0 && p%mask.y == 0 && (q+p)%mask.plus == 0 && (q-p)%mask.minus == 0){
				XDrawPoint(dpy, w, gc, j, i);
			}
		}
	}
}
