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

#include "FFont.h"
#include "FillMask.h"

int layout = LAYOUT_LEFT;

int FSaveFont(FFont* font, char fileName[]){
	font -> ver = 2;
	FILE *fp;
	fp = fopen(fileName, "wb");
	fwrite(font, sizeof(*font), 1, fp);
	fclose(fp);
	return 1;
}

int FLoadFont(FFont* font, char fileName[]){
	FILE *fp;
	int yn;
	if ((fp = fopen(fileName, "rb")) == NULL){
		printf("ファイル%sが見つかりません\n", fileName);
		return 0;
	}
	fread(font, sizeof(*font), 1, fp);
	fclose(fp);
	return 1;
}

int FLoadOldFont(FFont* font, char fileName[]){
	FILE *fp;
	FOldFont oldFont;
	int yn;
	if ((fp = fopen(fileName, "rb")) == NULL){
		printf("ファイル%sが見つかりません\n", fileName);
		return 0;
	}
	fread(&oldFont, sizeof(oldFont), 1, fp);
	fclose(fp);
	
	font -> length = oldFont.length;
	font -> height = oldFont.height;
	font -> marge = oldFont.marge;
	for(int i = 0; i < FONT_CHARS_LENGTH; i++){
		font -> chars[i].width = oldFont.chars[i].width;
		for(int j = 0; j < 64; j++){
			for(int k = 0; k < 64; k++){
				font -> chars[i].map[j][k] = oldFont.chars[i].map[j][k];
			}
		}
		wchar_t* cws = (wchar_t *)malloc(sizeof(wchar_t) * 2);
		for(int j = 1; j < 6; j++){
			oldFont.chars[i].ch[j] = 0;
		}
		int result = mbstowcs(cws, oldFont.chars[i].ch, 2);
		font -> chars[i].ch = cws[0];
	}
	return 1;
}

void FPrintAllChar(FFont font){
	for(int i = 0; i < font.length; i++){
		printf("%lc ", font.chars[i].ch);
	}
	printf("\n");
}

int getFontHeight(FFont font, int scale){
	return font.height * scale;
}

int getStringWidth(FFont font, char cs[], int scale, int margin){
	size_t capacity = strlen(cs) + 1;
	wchar_t *cws = (wchar_t *)malloc(sizeof(wchar_t) * capacity);
	int result = mbstowcs(cws, cs, capacity);
	if(result <= 0){
		printf("マルチバイト文字列の変換に失敗\n");
		return 0;
	}
	int ans = 0;
	for(int i = 0; i < result; i++){
		int num = -1;
		for(int j = 0; j < font.length; j++){
			if(cws[i] == font.chars[j].ch){
				num = j;
				break;
			}
		}
		if(num != -1){
			ans += scale * (font.chars[num].width + font.marge) + margin;
		}
	}
	ans -= margin + scale * font.marge;
	return ans;
}

void FSetLayout(int l){
	layout = l;
}

void FDrawChar(Display* dpy, Window w, GC gc, FFont font, wchar_t cw, int x, int y, int pixSize){
	int num = -1;
	for(int i = 0; i < font.length; i++){
		if(cw == font.chars[i].ch){
			num = i;
		}
	}
	if(num == -1){
		return;
	}
	CharFont cf = font.chars[num];
	XSetLineAttributes(dpy, gc, 1, LineSolid, CapProjecting, JoinMiter);
	for(int i = 0; i < font.height; i++){
		if(cf.map[i][0]){
			XDrawLine(dpy, w, gc, x, y + pixSize*i, x, y + pixSize*(i+1) - 1);
		}
		if(cf.map[i][cf.width-1]){
			XDrawLine(dpy, w, gc, x + pixSize*cf.width - 1, y + pixSize*i, x + pixSize*cf.width - 1, y + pixSize*(i+1) - 1);
		}
	}
	for(int i = 0; i < cf.width; i++){
		if(cf.map[0][i]){
			XDrawLine(dpy, w, gc, x + pixSize*i, y, x + pixSize*(i+1) - 1, y);
		}
		if(cf.map[font.height-1][i]){
			XDrawLine(dpy, w, gc, x + pixSize*i, y + pixSize*font.height - 1, x + pixSize*(i+1) - 1, y + pixSize*font.height - 1);
		}
	}
	for(int i = 0; i < font.height; i++){
		for(int j = 0; j < cf.width-1; j++){
			if(cf.map[i][j] && !cf.map[i][j+1]){
				XDrawLine(dpy, w, gc, x + pixSize*(j+1) - 1, y + pixSize*i, x + pixSize*(j+1) - 1, y + pixSize*(i+1) - 1);
			}
			if(!cf.map[i][j] && cf.map[i][j+1]){
				XDrawLine(dpy, w, gc, x + pixSize*(j+1), y + pixSize*i, x + pixSize*(j+1), y + pixSize*(i+1) - 1);
			}
		}
	}
	for(int i = 0; i < font.height-1; i++){
		for(int j = 0; j < cf.width; j++){
			if(cf.map[i][j] && !cf.map[i+1][j]){
				XDrawLine(dpy, w, gc, x + pixSize*j, y + pixSize*(i+1) - 1, x + pixSize*(j+1) - 1, y + pixSize*(i+1) - 1);
			}
			if(!cf.map[i][j] && cf.map[i+1][j]){
				XDrawLine(dpy, w, gc, x + pixSize*j, y + pixSize*(i+1), x + pixSize*(j+1) - 1, y + pixSize*(i+1));
			}
		}
	}
	for(int i = 0; i < font.height-1; i++){
		for(int j = 0; j < cf.width-1; j++){
			if(cf.map[i][j] && cf.map[i+1][j] && cf.map[i][j+1] && !cf.map[i+1][j+1]){
				XDrawPoint(dpy, w, gc, x + pixSize*(j+1) - 1, y + pixSize*(i+1) - 1);
			}
			if(!cf.map[i][j] && cf.map[i+1][j] && cf.map[i][j+1] && cf.map[i+1][j+1]){
				XDrawPoint(dpy, w, gc, x + pixSize*(j+1), y + pixSize*(i+1));
			}
			if(cf.map[i][j] && !cf.map[i+1][j] && cf.map[i][j+1] && cf.map[i+1][j+1]){
				XDrawPoint(dpy, w, gc, x + pixSize*(j+1), y + pixSize*(i+1) - 1);
			}
			if(cf.map[i][j] && cf.map[i+1][j] && !cf.map[i][j+1] && cf.map[i+1][j+1]){
				XDrawPoint(dpy, w, gc, x + pixSize*(j+1) - 1, y + pixSize*(i+1));
			}
		}
	}
}

void FFillChar(Display* dpy, Window w, GC gc, FFont font, wchar_t cw, int x, int y, int pixSize){
	int num = -1;
	for(int i = 0; i < font.length; i++){
		if(cw == font.chars[i].ch){
			num = i;
		}
	}
	if(num == -1){
		printf("登録されていない文字だよ\n");
		return;
	}
	CharFont cf = font.chars[num];
	for(int i = 0; i < font.height; i++){
		for(int j = 0; j < cf.width; j++){
			if(cf.map[i][j]){
				XFillRectangle(dpy, w, gc, x + pixSize*j, y + pixSize*i, pixSize, pixSize);
			}
		}
	}
}

void FFillMaskChar(Display* dpy, Window w, GC gc, FFont font, wchar_t cw, int x, int y, int pixSize, FillMask mask){
	int num = -1;
	for(int i = 0; i < font.length; i++){
		if(cw == font.chars[i].ch){
			num = i;
		}
	}
	if(num == -1){
		return;
	}
	CharFont cf = font.chars[num];
	for(int i = 0; i < font.height; i++){
		for(int j = 0; j < cf.width; j++){
			if(cf.map[i][j]){
				fillMaskRectangle(dpy, w, gc, x + pixSize*j, y + pixSize*i, pixSize, pixSize, mask);
			}
		}
	}
}

void FDrawString(Display* dpy, Window w, GC gc, FFont font, char cs[], int x, int y, int pixSize, int addMarge){
	size_t capacity = strlen(cs) + 1;
	wchar_t *cws = (wchar_t *)malloc(sizeof(wchar_t) * capacity);
	int result = mbstowcs(cws, cs, capacity);
	if(result <= 0){
		printf("マルチバイト文字列の変換に失敗\n");
		return;
	}
	
	int sx = 0;
	if(layout == LAYOUT_LEFT){
		sx = x;
	}else if(layout == LAYOUT_RIGHT){
		sx = x - getStringWidth(font, cs, pixSize, addMarge);
	}else if(layout == LAYOUT_CENTER){
		sx = x - getStringWidth(font, cs, pixSize, addMarge)/2;
	}
	for(int i = 0; i < result; i++){
		int num = -1;
		for(int j = 0; j < font.length; j++){
			if(cws[i] == font.chars[j].ch){
				num = j;
				break;
			}
		}
		if(num != -1){
//			printf("%s ", c);
			FDrawChar(dpy, w, gc, font, cws[i], sx, y, pixSize);
			sx += pixSize * (font.chars[num].width + font.marge) + addMarge;
		}
	}
}

void FFillString(Display* dpy, Window w, GC gc, FFont font, char cs[], int x, int y, int pixSize, int addMarge){
//	printf("%s\n", cs);
//	cs = "asdfghjkl";
	size_t capacity = strlen(cs) + 1;
	if(capacity == 1){
		return;
	}
	wchar_t *cws = (wchar_t *)malloc(sizeof(wchar_t) * capacity);
	int result = mbstowcs(cws, cs, capacity);
	if(result <= 0){
		printf("マルチバイト文字列の変換に失敗\n");
		return;
	}
	
	int sx = 0;
	if(layout == LAYOUT_LEFT){
		sx = x;
	}else if(layout == LAYOUT_RIGHT){
		sx = x - getStringWidth(font, cs, pixSize, addMarge);
	}else if(layout == LAYOUT_CENTER){
		sx = x - getStringWidth(font, cs, pixSize, addMarge)/2;
	}
	for(int i = 0; i < result; i++){
		int num = -1;
//		printf("length : %d\n", font.length);
		for(int j = 0; j < font.length; j++){
//			printf("(%lc,%lc)\n", cws[i], font.chars[j].ch);
			if(cws[i] == font.chars[j].ch){
				num = j;
//				printf("@\n");
				break;
			}
		}
		if(num != -1){
//			printf("%s ", c);
			FFillChar(dpy, w, gc, font, cws[i], sx, y, pixSize);
			sx += pixSize * (font.chars[num].width + font.marge) + addMarge;
		}
	}
}

void FFillMaskString(Display* dpy, Window w, GC gc, FFont font, char cs[], int x, int y, int pixSize, int addMarge, FillMask mask){
	size_t capacity = strlen(cs) + 1;
	wchar_t *cws = (wchar_t *)malloc(sizeof(wchar_t) * capacity);
	int result = mbstowcs(cws, cs, capacity);
	if(result <= 0){
		printf("マルチバイト文字列の変換に失敗\n");
		return;
	}
	
	int sx = 0;
	if(layout == LAYOUT_LEFT){
		sx = x;
	}else if(layout == LAYOUT_RIGHT){
		sx = x - getStringWidth(font, cs, pixSize, addMarge);
	}else if(layout == LAYOUT_CENTER){
		sx = x - getStringWidth(font, cs, pixSize, addMarge)/2;
	}
	for(int i = 0; i < strlen(cs); i++){
		int num = -1;
		for(int j = 0; j < font.length; j++){
			if(cws[i] == font.chars[j].ch){
				num = j;
				break;
			}
		}
		if(num != -1){
//			printf("%s ", c);
			FFillMaskChar(dpy, w, gc, font, cws[i], sx, y, pixSize, mask);
			sx += pixSize * (font.chars[num].width + font.marge) + addMarge;
		}
	}
}
