/* xdraw.c */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <unistd.h>
#include <wchar.h>
#include <time.h>

#include "Flib/FillMask.h"
#include "Flib/FFont.h"

#define BORDER 2
#define WIDTH  900
#define HEIGHT 600
#define PI 3.141592

typedef struct{
	int x;
	int y;
}Point;

typedef struct{
	int r;
	int g;
	int b;
}Color;

typedef struct{
	int min;
	int max;
	int value;
	int r;
	Point point;
	int length;
}Slider;

typedef struct{
	Point point;
	int width;
	int height;
	int isHovering;	//マウスが乗ってるかどうか
	int isHolding;	//マウスがクリックしている途中かどうか
	int isClicked;
	char text[256];
}Button;

float getRand(int min, int max);
void setColor(Display* dpy, GC gc, int r, int g, int b);
char numTo16Base(int num);

void updateMousePoint(XEvent, Point*);

void drawSlider(Display*, Window, GC, Slider);
int isMouseOnSlider(Point mousePoint, Slider slider);
void updateSliderValue(Point, Slider *slider);

void drawButton(Display* dpy, Window w, GC gc, Button button);
int isMouseOnButton(Point mousePoint, Button button);

static void addChar(FFont* font);
static int removeChar(FFont* font, int num);

void drawMyChar(Display* dpy, Window w, GC gc, char c, int x, int y, Color color);
void fillMyChar(Display* dpy, Window w, GC gc, char c, int x, int y, Color color, FillMask mask);

static void saveFont(FFont* font);
static void loadFont(FFont* font);



FFont systemFont;

int main(int argc,char **argv)
{
	FLoadFont(&systemFont, "fonts/CoolBitFont.dat");
//	printf("CoolbitFont.length : %d\n", systemFont.length);
	
	Display *dpy;
	Window w;
	Window root;
	Pixmap buffer;
	int    screen;
	GC       gc;
	char moji[256];            /* 日本語メッセージが入る場所 */

	XFontSet fs;             /* XFontsetで日本語フォントを選ぶ（日） */
	char **miss,*def;
	int n_miss;              /* 変数として宣言しておく（日） */

	setlocale(LC_ALL, "");        /* 1st. lacaleをセットして（日） */

	dpy = XOpenDisplay("");	//ディスプレイサーバーと接続

	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);

	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, 0x000000, 0xffffff);
	buffer = XCreatePixmap( dpy, w, WIDTH, HEIGHT, 24 );
	gc = XCreateGC(dpy, w, 0, NULL);
	XSetGraphicsExposures( dpy, gc, False );
	
	fs=XCreateFontSet(dpy,"-*-*-medium-r-normal-*-16-*",&miss,&n_miss,&def);
	/* 少なくともdpyに値がはいってからでないと駄目（日） */

	XMapWindow(dpy, w);
	
	Point mousePoint = {0, 0};
	int r = 0;
	int g = 0;
	int b = 0;
	
	FFont font;
	font.height = 10;
	font.length = 37;
	font.marge = 1;
	{
		wchar_t chars[] = L" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for(int i = 0; i < 256; i++){
			CharFont cf;
			cf.ch = chars[i];
			cf.width = 10;
			for(int j = 0; j < 64; j++){
				for(int k = 0; k < 64; k++){
					cf.map[j][k] = 0;
				}
			}
			font.chars[i] = cf;
		}
	}
	
	int hold = 0;
	_Bool isHolding = 0;
	int isHoldDrawArea = 0;
	int drawColor = 1;
	
	int selectChar = 0;
	int drawAreaX = 30;
	int drawAreaY = 150;
	int pixSize = 20;
	int previewSize = 10;
	int previewMode = 0;
		
//	Slider sliders[3] = {{0, 255, 100, 20, 50, 200, 500}, {0, 255, 255, 20, 50, 250, 500}, {0, 255, 100, 20, 50, 300, 500}};
	
//	Button button = {100, 350, 400, 200, 0, 0, 0};
	Button nextButton = {200, 50, 80, 30, 0, 0, 0, "next char"};
	Button previousButton = {200, 90, 80, 30, 0, 0, 0, "previous char"};
	Button heightUpButton = {300, 50, 80, 30, 0, 0, 0, "height up"};
	Button heightDownButton = {300, 90, 80, 30, 0, 0, 0, "height down"};
	Button widthUpButton = {400, 50, 80, 30, 0, 0, 0, "width up"};
	Button widthDownButton = {400, 90, 80, 30, 0, 0, 0, "width down"};
	Button saveButton = {500, 50, 80, 30, 0, 0, 0, "save"};
	Button loadButton = {500, 90, 80, 30, 0, 0, 0, "load"};
	Button margeDownButton = {500, 310, 35, 80, 0, 0, 0, ""};
	Button margeUpButton = {545, 310, 35, 80, 0, 0, 0, ""};
	Button addFontButton = {500, 130, 80, 80, 0, 0, 0, "add"};
	Button removeFontButton = {500, 220, 80, 80, 0, 0, 0, "remove"};
	Button previewDownButton = {600, 10, 30, 30, 0, 0, 0, "-"};
	Button previewUpButton = {635, 10, 30, 30, 0, 0, 0, "+"};
	Button pixSizeDownButton = {400, 10, 30, 30, 0, 0, 0, "-"};
	Button pixSizeUpButton = {435, 10, 30, 30, 0, 0, 0, "+"};
	
	int buttonNum = 16;
	Button* buttons[buttonNum];
	buttons[0] = &nextButton;
	buttons[1] = &previousButton;
	buttons[2] = &heightUpButton;
	buttons[3] = &heightDownButton;
	buttons[4] = &widthUpButton;
	buttons[5] = &widthDownButton;
	buttons[6] = &saveButton;
	buttons[7] = &loadButton;
	buttons[8] = &margeUpButton;
	buttons[9] = &margeDownButton;
	buttons[10] = &addFontButton;
	buttons[11] = &removeFontButton;
	buttons[12] = &previewUpButton;
	buttons[13] = &previewDownButton;
	buttons[14] = &pixSizeUpButton;
	buttons[15] = &pixSizeDownButton;
	
	FillMask backgroundMask = {1, 1, 5, 1, 3};
	
	
	
	Pixmap background = XCreatePixmap(dpy, w, WIDTH, HEIGHT, 8);
	
	int time = 0;
	XSelectInput(dpy,w,ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
	
	while(1){
		//画面真っ黒
		setColor(dpy, gc, 0, 0, 0);
		XFillRectangle(dpy, buffer, gc, 0, 0, WIDTH, HEIGHT);
		
		//suraimu-の描画
		setColor(dpy, gc, 255, 255, 255);
		XSetLineAttributes(dpy, gc, 1, LineSolid, CapRound, JoinRound);
		{
			char suraimuMap[140][140];
			float t = time/2.0;
			float p1X = 30*cos(PI*t/37);
			float p1Y = 30*sin(PI*t/51);
			float p2X = -30*cos(PI*t/60);
			float p2Y = -30*sin(PI*t/47);
			float p3X = 10*cos(PI*t/57);
			float p3Y = 10*sin(PI*t/41);
			for(int i = -70; i < 70; i++){
				for(int j = -70; j < 70; j++){
					if(((j-p2X)*(j-p2X)+(i-p2Y)*(i-p2Y))*((j-p1X)*(j-p1X)+(i-p1Y)*(i-p1Y))*((j-p3X)*(j-p3X)+(i-p3Y)*(i-p3Y)) < 600000000){
						suraimuMap[i+70][j+70] = 1;
					}else{
						suraimuMap[i+70][j+70] = 0;
					}
				}
			}
			
			if(previewMode == 0){
				for(int i = -70; i < 70; i++){
					for(int j = -70; j < 70; j++){
						if(suraimuMap[i+70][j+70]){
							XDrawPoint(dpy, buffer, gc, j+90, i+90);
						}
					}
				}
			}else if(previewMode == 1){
				setColor(dpy, gc, 100, 100, 100);
				for(int i = -70; i < 70; i++){
					for(int j = -70; j < 70; j++){
						if(suraimuMap[i+70][j+70] && ((i+70) + (j+70))%15 == (time/3)%15){
							XDrawPoint(dpy, buffer, gc, j+90, i+90);
						}
					}
				}
			}
			
			
			if(previewMode == 1){
				setColor(dpy, gc, 255, 255, 255);
				for(int i = -70+1; i < 70-1; i++){
					for(int j = -70+1; j < 70; j++){
						if(suraimuMap[i+70-1][j+70] ^ suraimuMap[i+70][j+70] || suraimuMap[i+70][j+70] ^ suraimuMap[i+70+1][j+70]){
							XDrawPoint(dpy, buffer, gc, j+90, i+90);
						}
					}
				}
				for(int i = -70; i < 70; i++){
					for(int j = -70; j < 70-1; j++){
						if(suraimuMap[i+70][j+70-1] ^ suraimuMap[i+70][j+70] || suraimuMap[i+70][j+70] ^ suraimuMap[i+70][j+70+1]){
							XDrawPoint(dpy, buffer, gc, j+90, i+90);
						}
					}
				}
			}
			
			if(previewMode == 0){
				setColor(dpy, gc, 0, 0, 0);
			}else if(previewMode == 1){
				setColor(dpy, gc, 255, 255, 255);
			}
			XFillArc(dpy, buffer, gc, 90+p3X-4 + (int)(2*sin(PI*t/47)), 90+p3Y-3, 4, 4, 0, 360*64);
			XFillArc(dpy, buffer, gc, 90+p3X+4, 90+p3Y-3 + (int)(2*sin(PI*t/57)), 4, 4, 0, 360*64);
			XDrawArc(dpy, buffer, gc, 90+p3X-4, 90+p3Y-5, 10, 10, (270-45)*64, (90)*64);
		}
		
		//ボタンの描画
		for(int i = 0; i < buttonNum; i++){
			drawButton(dpy, buffer, gc, *buttons[i]);
		}
		
		//お絵描きエリアの描画
		if(font.length > 0){
			for(int i = 0; i < font.height; i++){
				for(int j = 0; j < font.chars[selectChar].width; j++){
					if(font.chars[selectChar].map[i][j]){
						setColor(dpy, gc, 200, 200, 200);
					}else{
						setColor(dpy, gc, 50, 50, 50);
					}
					if(pixSize > 10){
						XFillRectangle(dpy, buffer, gc, drawAreaX + pixSize*j + 1, drawAreaY + pixSize*i + 1, pixSize - 2, pixSize -2);
					}else if(pixSize > 5){
						XFillRectangle(dpy, buffer, gc, drawAreaX + pixSize*j + 1, drawAreaY + pixSize*i + 1, pixSize - 1, pixSize -1);
					}else{
						XFillRectangle(dpy, buffer, gc, drawAreaX + pixSize*j, drawAreaY + pixSize*i, pixSize, pixSize);
					}
				}
				setColor(dpy, gc, 30, 30, 30);
				for(int j = 0; j < font.marge; j++){
					XFillRectangle(dpy, buffer, gc, drawAreaX + pixSize*(j + font.chars[selectChar].width) + 1, drawAreaY + pixSize*i + 1, pixSize - 2, pixSize -2);
				}
			}
		}
		
		setColor(dpy, gc, 255, 255, 255);
		if(font.length > 0){
			snprintf(moji, 50, "Edit charactor : \'%lc\'", font.chars[selectChar].ch);
		}else{
			snprintf(moji, 50, "Edit charactor : NULL");
		}
		XmbDrawString(dpy, buffer, fs, gc, 200, 40, moji, strlen(moji));
		{
//			char s[20] = "Font!";
//			FDrawString(dpy, buffer, gc, font, s, 0, 0, 5, 0);
		}
		//プレビューの描画
		if(font.length > 0){
			if(previewMode == 0){
				setColor(dpy, gc, 255, 255, 255);
				FFillChar(dpy, buffer, gc, font, font.chars[selectChar].ch, 600, 50, previewSize);
			}else if(previewMode == 1){
				FillMask mask = {1, 1, 3, 1, 1};
				
				setColor(dpy, gc, 150, 150, 150);
				FFillMaskChar(dpy, buffer, gc, font, font.chars[selectChar].ch, 600, 50, previewSize, mask);
				
				setColor(dpy, gc, 255, 255, 255);
				FDrawChar(dpy, buffer, gc, font, font.chars[selectChar].ch, 600, 50, previewSize);
			}
		}
		
		while(XEventsQueued(dpy, QueuedAlready) != 0){
			XEvent e;
			XNextEvent(dpy,&e);
			switch (e.type){
				case ButtonPress : {
//					printf("x=%d y=%d button=%d \n",e.xbutton.x,e.xbutton.y,e.xbutton.button);
					if(e.xbutton.button == 1){
/*						for(int i = 0; i < 3; i++){
							if(isMouseOnSlider(mousePoint, sliders[i])){
								hold = i;
								isHolding = 1;
							}
						}*/
						for(int i = 0; i < buttonNum; i++){
							if(isMouseOnButton(mousePoint, *buttons[i])){
								buttons[i] -> isHolding = 1;
							}
						}
					}
					{
						int x = (mousePoint.x - drawAreaX)/pixSize;
						int y = (mousePoint.y - drawAreaY)/pixSize;
						if(font.length > 0){
							if(0 <= x && x < font.chars[selectChar].width && 0 <= y && y < font.height){
								font.chars[selectChar].map[y][x] += 1;
								font.chars[selectChar].map[y][x] %= 2;
								drawColor = font.chars[selectChar].map[y][x];
								isHoldDrawArea = 1;
							}
						}
					}
					{
						int x = mousePoint.x - 90;
						int y = mousePoint.y - 90;
						float t = time/2.0;
						float p1X = 30*cos(PI*t/37);
						float p1Y = 30*sin(PI*t/51);
						float p2X = -30*cos(PI*t/60);
						float p2Y = -30*sin(PI*t/47);
						float p3X = 10*cos(PI*t/57);
						float p3Y = 10*sin(PI*t/41);
						if(((x-p2X)*(x-p2X)+(y-p2Y)*(y-p2Y))*((x-p1X)*(x-p1X)+(y-p1Y)*(y-p1Y))*((x-p3X)*(x-p3X)+(y-p3Y)*(y-p3Y)) < 600000000){
							previewMode++;
							previewMode %= 2;
						}
					}
					break;
				}
				case ButtonRelease : {
					isHolding = 0;
					hold = -1;
					for(int i = 0; i < buttonNum; i++){
						if(buttons[i] -> isHolding && buttons[i] -> isHovering){
							buttons[i] -> isClicked = 1;
						}
						buttons[i] -> isHolding = 0;
					}
					isHoldDrawArea = 0;
					break;
				}
				case MotionNotify : {
					updateMousePoint(e, &mousePoint);
/*					if(isHolding){
						updateSliderValue(mousePoint, &sliders[hold]);
					}*/
					for(int i = 0; i < buttonNum; i++){
						if(isMouseOnButton(mousePoint, *buttons[i])){
							buttons[i] -> isHovering = 1;
						}else{
							buttons[i] -> isHovering = 0;
						}
					}
					if(isHoldDrawArea){
						int x = (mousePoint.x - drawAreaX)/pixSize;
						int y = (mousePoint.y - drawAreaY)/pixSize;
						if(0 <=  x && x < font.chars[selectChar].width && 0 <= y && y < font.height){
							font.chars[selectChar].map[y][x] = drawColor;
						}
					}
					break;
				}
			}
		}
		
		//ボタンイベントたち
		if(nextButton.isClicked == 1){
			if(font.length > 0){
				selectChar++;
				selectChar %= font.length;
			}
			nextButton.isClicked = 0;
		}
		if(previousButton.isClicked){
			if(font.length > 0){
				selectChar += font.length;
				selectChar--;
				selectChar %= font.length;
			}
			previousButton.isClicked = 0;
		}
		if(widthUpButton.isClicked == 1){
			if(font.length > 0){
				if(font.chars[selectChar].width < 64){
					font.chars[selectChar].width++;
					for(int i = 0; i < font.length; i++){
						if(i == selectChar){
							continue;
						}
						int isAllBlack = 1;
						for(int j = 0; j < font.length; j++){
							for(int k = 0; k < font.chars[i].width; k++){
								if(font.chars[i].map[j][k]){
									isAllBlack = 0;
									break;
								}
							}
							if(!isAllBlack){
								break;
							}
						}
						if(isAllBlack){
							font.chars[i].width = font.chars[selectChar].width;
						}
					}
				}
			}
			widthUpButton.isClicked = 0;
		}
		if(widthDownButton.isClicked == 1){
			if(font.length > 0){
				if(font.chars[selectChar].width > 1){
					font.chars[selectChar].width--;
					for(int i = 0; i < font.length; i++){
						if(i == selectChar){
							continue;
						}
						int isAllBlack = 1;
						for(int j = 0; j < font.height; j++){
							for(int k = 0; k < font.chars[i].width; k++){
								if(font.chars[i].map[j][k]){
									isAllBlack = 0;
									break;
								}
							}
							if(!isAllBlack){
								break;
							}
						}
						if(isAllBlack){
							font.chars[i].width = font.chars[selectChar].width;
						}
					}
				}
			}
			widthDownButton.isClicked = 0;
		}
		if(heightUpButton.isClicked == 1){
			if(font.height < 64){
				font.height++;
			}
			heightUpButton.isClicked = 0;
		}
		if(heightDownButton.isClicked == 1){
			if(font.height > 1){
				font.height--;
			}
			heightDownButton.isClicked = 0;
		}
		if(saveButton.isClicked == 1){
			saveFont(&font);
			saveButton.isClicked = 0;
		}
		if(loadButton.isClicked == 1){
			selectChar = 0;
			loadFont(&font);
			loadButton.isClicked = 0;
		}
		if(addFontButton.isClicked == 1){
			addChar(&font);
			addFontButton.isClicked = 0;
		}
		if(removeFontButton.isClicked == 1){
			if(removeChar(&font, selectChar)){
				if(selectChar > 0){
					selectChar--;
				}
			}
			removeFontButton.isClicked = 0;
		}
		if(margeUpButton.isClicked == 1){
			if(font.marge < 64){
				font.marge++;
			}
			margeUpButton.isClicked = 0;
		}
		if(margeDownButton.isClicked == 1){
			if(font.marge > 0){
				font.marge--;
			}
			margeDownButton.isClicked = 0;
		}
		if(previewUpButton.isClicked == 1){
			if(previewSize < 64){
				previewSize++;
			}
			previewUpButton.isClicked = 0;
		}
		if(previewDownButton.isClicked == 1){
			if(previewSize > 1){
				previewSize--;
			}
			previewDownButton.isClicked = 0;
		}
		if(pixSizeUpButton.isClicked == 1){
			if(pixSize < 64){
				pixSize++;
			}
			pixSizeUpButton.isClicked = 0;
		}
		if(pixSizeDownButton.isClicked == 1){
			if(pixSize > 2){
				pixSize--;
			}
			pixSizeDownButton.isClicked = 0;
		}
		for(int i = 0; i < buttonNum; i++){
			if(buttons[i] -> isClicked){
				buttons[i] -> isClicked = 0;
			}
		}
		
		XCopyArea( dpy, buffer, w, gc, 0, 0, WIDTH, HEIGHT, 0, 0 );
		XFlush(dpy);	//[重要]描画のフラッシュ(これをしないと描画が確定されない)
		usleep(1000 * 30);
		time++;
	}
}

float getRand(int min, int max){
	static int isFirst;

	if (isFirst == 0) {
		srand((unsigned int)time(NULL));
		isFirst = 1;
	}

	return min + (rand() / (0.0 + RAND_MAX) * (max - min));
}

//(0, 0, 0)が黒、(255,255,255)が白
void setColor(Display* dpy, GC gc, int r, int g, int b){
	XSetForeground(dpy, gc, 0x010000 * r + 0x0100 * g + b);
}

char numTo16Base(int num){
	if(0 <= num && num < 10){
		return '0' + num;
	}else{
		return 'A' + num - 10;
	}
}

void drawSlider(Display* dpy,Window w, GC gc, Slider slider){
	setColor(dpy, gc, 200, 200, 200);
	XSetLineAttributes(dpy, gc, slider.r, LineSolid, CapRound, JoinRound);
	XDrawLine(dpy, w, gc, slider.point.x, slider.point.y, slider.point.x + slider.length, slider.point.y);
	
	setColor(dpy, gc, 0, 0, 0);
	XSetLineAttributes(dpy, gc, slider.r - 2, LineSolid, CapRound, JoinRound);
	XDrawLine(dpy, w, gc, slider.point.x, slider.point.y, slider.point.x + slider.length, slider.point.y);
	
	setColor(dpy, gc, 0, 0, 0);
	XFillArc(dpy, w, gc, slider.length * (0.0 + slider.value - slider.min) / (slider.max - slider.min) + slider.point.x - slider.r, slider.point.y - slider.r, slider.r*2, slider.r*2, 0, 360*64);
	
	setColor(dpy, gc, 200, 200, 200);
	XSetLineAttributes(dpy, gc, 1, LineSolid, CapRound, JoinRound);
	XDrawArc(dpy, w, gc, slider.length * (0.0 + slider.value - slider.min) / (slider.max - slider.min) + slider.point.x - slider.r, slider.point.y - slider.r, slider.r*2, slider.r*2, 0, 360*64);
}

int isMouseOnSlider(Point mousePoint, Slider slider){
	if(	mousePoint.x > slider.point.x - slider.r && 
		mousePoint.x < slider.point.x + slider.length + slider.r &&
		mousePoint.y > slider.point.y - slider.r &&
		mousePoint.y < slider.point.y + slider.r
	){
		return 1;
	}else{
		return 0;
	}
}

void updateSliderValue(Point mousePoint, Slider *slider){
	int x = (0.0 + mousePoint.x - (slider -> point.x))/(slider -> length) * (slider -> max - slider -> min) + slider -> min;
	if(x < slider -> min){
		x = slider -> min;
	}
	if(x > slider -> max){
		x = slider -> max;
	}
	slider -> value = x;
}

void updateMousePoint(XEvent e, Point *mousePoint){
	mousePoint -> x = e.xmotion.x;
	mousePoint -> y = e.xmotion.y;
}

void drawButton(Display* dpy, Window w, GC gc, Button button){
	if(button.isHovering && button.isHolding){
		setColor(dpy, gc, 170, 170, 170);
	}else if(button.isHovering){
		setColor(dpy, gc, 190, 190, 190);
	}else{
		setColor(dpy, gc, 200, 200, 200);
	}
	XFillRectangle(dpy, w, gc, button.point.x, button.point.y, button.width, button.height);
	setColor(dpy, gc, 50, 50, 50);
	XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);
	XDrawRectangle(dpy, w, gc, button.point.x, button.point.y, button.width, button.height);
	if(button.isHovering && button.isHolding){
		setColor(dpy, gc, 150, 150, 150);
	}else{
		setColor(dpy, gc, 230, 230, 230);
	}
	XSetLineAttributes(dpy, gc, 3, LineSolid, CapButt, JoinMiter);
	XDrawLine(dpy, w, gc, button.point.x+4, button.point.y+2, button.point.x + button.width-4, button.point.y+2);
	XDrawLine(dpy, w, gc, button.point.x+2, button.point.y+4, button.point.x+2, button.point.y-4 + button.height);
	if(button.isHovering && button.isHolding){
		setColor(dpy, gc, 200, 200, 200);
	}else{
		setColor(dpy, gc, 170, 170, 170);
	}
	XDrawLine(dpy, w, gc, button.point.x + button.width-3, button.point.y+4, button.point.x + button.width-3, button.point.y-4 + button.height);
	XDrawLine(dpy, w, gc, button.point.x+4, button.point.y-3 + button.height, button.point.x + button.width-4, button.point.y-3 + button.height);
	
	setColor(dpy, gc, 0, 0, 0);
	FSetLayout(LAYOUT_CENTER);
	FFillString(dpy, w, gc, systemFont, button.text, button.point.x + button.width/2, button.point.y + 5, 1, 0); 
}

int isMouseOnButton(Point mousePoint, Button button){
	if(button.point.x < mousePoint.x && mousePoint.x < button.point.x + button.width &&
	   button.point.y < mousePoint.y && mousePoint.y < button.point.y + button.height){
		return 1;
	}else{
		return 0;
	}
}

static void addChar(FFont* font){
	printf("文字を追加するよ！(最大256文字, 現在%d文字)\n", font -> length);
	char cs[256 * 6];
	printf("追加する文字(複数可)\n-->");
	scanf("%s", cs);
	if(cs[0] == '\0'){
		printf("キャンセルしたよ\n");
		return;
	}
//		printf("キャンセルしたよ\n");
//		return;
//	}
	
	size_t capacity = strlen(cs) + 1;
	wchar_t *cws = (wchar_t *)malloc(sizeof(wchar_t) * capacity);

	// char -> wchar_tの変換
	int result = mbstowcs(cws, cs, capacity);

	if(result <= 0){
		printf("マルチバイト文字列の変換に失敗\n");
		return;
	}
	
	for(int i = 0; i < result; i++){
		wchar_t cw = cws[i];
		
		int isOk = 1;
		for(int j = 0; j < font -> length; j++){
			if(cw == font -> chars[j].ch){
				isOk = 0;
				break;
			}
		}
		if(!isOk){
			continue;
		}
		CharFont cf;
		cf.ch = cw;
		cf.width = 10;
		for(int j = 0; j < 64; j++){
			for(int k = 0; k < 64; k++){
				cf.map[j][k] = 0;
			}
		}
		font -> chars[font -> length] = cf;
		font -> length++;
		printf("%lc", cw);
	}
	printf("が追加されました\n");
}

static int removeChar(FFont* font, int num){
	if(font -> length == 0){
		printf("けす もじ が ないよ\n");
		return 0;
	}
/*	printf("\'%s\' を さくし゛ょ する？(Yes:1 / No:0)\n-->", font -> chars[num].ch);
	int yn;
	scanf("%d", &yn);
	if(yn != 1){
		printf("キャンセル したよ\n");
		return 0;
	}*/
	
	char space = ' ';
	wchar_t* wspace;
	mbtowc(wspace, &space, 2);
	int isSpace = 1;
	if(font -> chars[num].ch == wspace[0]){
		printf("スペースは消せないよ\n");
		return 0;
	}
	
	for(int i = num; i < font -> length-1; i++){
		font -> chars[i] = font -> chars[i+1];
	}
	font -> length--;
	printf("\'%lc\' を さくし゛ょ したよ\n", font -> chars[num].ch);
	return 1;
}

static void saveFont(FFont* font){
	printf("ほそ゛ん、する？(Yes:1 / No:0)\n-->");
	int yn;
	scanf("%d", &yn);
	if(yn != 1){
		printf("キャンセル、した！\n");
		return;
	}
	printf("ファイル　に、かきこむ！\n");
	printf("ほそ゛ん　する　ファイル　なまえ？\n-->");
	char s[256];
	scanf("%s", s);
	FSaveFont(font, s);
	printf("%s　に　ほそ゛ん　した！\n", s);
}

static void loadFont(FFont* font){
	int yn;
	printf("現在の編集を破棄してファイルから読み込みますか?(Yes:1 / No:0)\n-->");
	scanf("%d", &yn);
	if(yn != 1){
		printf("ロードをキャンセルしました\n");
		return;
	}
	
	int type;
	printf("旧形式のフォントを読み込みますか?(Yes:1 / No:0)\n-->");
	scanf("%d", &type);
	
	char fileName[256];
//	scanf("%[^\n]%*c", fileName);
	printf("ロードするファイル名\n");
	printf("-->");
	scanf("%s", fileName);
	
	int isOk;
	if(type == 1){
		isOk = FLoadOldFont(font, fileName);
	}else{
		isOk = FLoadFont(font, fileName);
	}
	
	if (!isOk){
		printf("ファイルがないよ！\n");
		printf("拡張子もつけて入力してね！\n");
		return;
	}
	printf("%sをロードしたよ\n", fileName);
}
