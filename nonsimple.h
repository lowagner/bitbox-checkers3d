// nonsimple.h : a simple framebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

//#define SCREEN_W 640 // number of regular pixels
//#define SCREEN_H 480
#define SCREEN_W 160      // number of super pixels (4x4 pixels)
#define SCREEN_H 120

typedef void (void_fn)(void);

extern uint16_t superpixel[SCREEN_H][SCREEN_W];
extern uint16_t bg_color;
extern void_fn *graph_line_callback;

void clear();

void background_decay();
void background_color_decay();

#endif
