#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <unistd.h>
#include <time.h>

#include "Flib/FillMask.h"
#include "Flib/FFont.h"

#define BORDER 2
#define WIDTH  900
#define HEIGHT 600

void setColor(Display* dpy, GC gc, int r, int g, int b);

int main(int argc,char **argv)
{
	Display *dpy;
	Window w;
	Window root;
	int    screen;
	GC       gc;
	char moji[256];            /* 日本語メッセージが入る場所 */

	XFontSet fs;             /* XFontsetで日本語フォントを選ぶ（日） */
	char **miss,*def;
	int n_miss;              /* 変数として宣言しておく（日） */

	setlocale(LC_ALL,"");        /* 1st. lacaleをセットして（日） */

	dpy = XOpenDisplay("");	//ディスプレイサーバーと接続

	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);

	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, 0x000000, 0xffffff);

	gc = XCreateGC(dpy, w, 0, NULL);
	XSetGraphicsExposures( dpy, gc, False );
	
	/* 少なくともdpyに値がはいってからでないと駄目（日） */

	XMapWindow(dpy, w);
	
	
//	FFont kanafont;
//	FLoadFont(&kanafont, "fonts/CoreKanaBitmap.dat");
//	FLoadFont(&kanafont, "fonts/CoolBitFont.dat");

	FFont font;
	FLoadFont(&font, "fonts/CoolBitFont.dat");
	
	FFont font2;
	FLoadOldFont(&font2, "fonts/WinCourier19px.dat");
	
	FFont font3;
	FLoadFont(&font3, "fonts/henbibun");
	
	FillMask mask = {1, 1, 3, 1, 3};
	
	int time = 0;
	XSelectInput(dpy,w,ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
	
	while(1){
		setColor(dpy, gc, 0, 0, 0);
		XFillRectangle(dpy, w, gc, 0, 0, WIDTH, HEIGHT);
		FSetLayout(LAYOUT_LEFT);
		setColor(dpy, gc, 50, 50, 50);
		FFillString(dpy, w, gc, font, "foo bar", 100, 200, 10, 10);
		setColor(dpy, gc, 200, 200, 200);
		FFillMaskString(dpy, w, gc, font, "foo bar", 100, 200, 10, 10, mask);
		setColor(dpy, gc, 255, 255, 255);
		FDrawString(dpy, w, gc, font, "foo bar", 100, 200, 10, 10);
		FFillString(dpy, w, gc, font3, "∂∂∂∂∂∂", 300, 450, 10, 0);
		
		XFlush(dpy);
		usleep(1000 * 1000);
		
		setColor(dpy, gc, 0, 0, 0);
		XFillRectangle(dpy, w, gc, 0, 0, WIDTH, HEIGHT);
		
		setColor(dpy, gc, 255, 255, 255);
		XDrawLine(dpy, w, gc, 450, 0, 450, 600);
		FSetLayout(LAYOUT_LEFT);
		FFillString(dpy, w, gc, font, "Hello World!!", 450, 100, 10, 0);
		FSetLayout(LAYOUT_CENTER);
		FFillString(dpy, w, gc, font, "Hello World!!", 450, 300, 10, 0);
		FSetLayout(LAYOUT_RIGHT);
		FFillString(dpy, w, gc, font, "Hello World!!", 450, 500, 10, 0);
		
		XFlush(dpy);
		usleep(1000 * 1000);
		
		setColor(dpy, gc, 0, 0, 0);
		XFillRectangle(dpy, w, gc, 0, 0, WIDTH, HEIGHT);
		
		setColor(dpy, gc, 255, 255, 255);
		FSetLayout(LAYOUT_CENTER);
		FFillString(dpy, w, gc, font, "This is a demo", 450, 100, 7, 0);
		FDrawString(dpy, w, gc, font, "Font Test", 450, 300, 5, 0);
		FFillString(dpy, w, gc, font2, "CENTER", 450, 400, 5, 20);
		
		XFlush(dpy);
		usleep(1000 * 1000);
		time++;
	}
}

//(0, 0, 0)が黒、(255,255,255)が白
void setColor(Display* dpy, GC gc, int r, int g, int b){
	XSetForeground(dpy, gc, 0x010000 * r + 0x0100 * g + b);
}
