#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

uint16_t superpixel[SCREEN_H][SCREEN_W];
void_fn *graph_line_callback;
uint16_t bg_color;

void graph_frame() {}
// --------------------------------------------------------------

void graph_line() 
{
    if (vga_line % 4 < 2)
    {
        uint32_t *dst = (uint32_t*)draw_buffer;
        int j = vga_line/4;
        for (int i=0; i<SCREEN_W; ++i)
        {
            uint32_t current_color = superpixel[j][i];
            current_color |= (current_color<<16);

            *dst++ = current_color;
            *dst++ = current_color;
        }
    }
    else if (vga_line % 4 == 2)
        if (graph_line_callback)
            graph_line_callback();
    }

// --------------------------------------------------------------
// utilities 

void clear() 
{
   memset(superpixel, 0, sizeof(superpixel));
}

void background_decay()
{
    int j = vga_line/4;
    if (vga_frame % 2 == j % 2) // do odd lines on odd frames, even ...
    {
        for (int i=0; i<SCREEN_W; ++i)
        {
            int newcolor = 1*(superpixel[j][i]-bg_color);
            if (i > 0)
                newcolor += (superpixel[j][i-1]-bg_color);
            if (i < SCREEN_W-1)
                newcolor += (superpixel[j][i+1]-bg_color);
            if (j > 0)
                newcolor += (superpixel[j-1][i]-bg_color);
            if (j < SCREEN_H-1)
                newcolor += (superpixel[j+1][i]-bg_color);
            superpixel[j][i] = newcolor/5 + bg_color;
        }
    }
}

void background_color_decay()
{
    int j = vga_line/4;
    if (vga_frame % 2 == j % 2) // do odd lines on odd frames, even ...
    switch ((vga_frame/2) % 3)
    {
    case 0:
        for (int i=0; i<SCREEN_W; ++i)
        {
            uint8_t bg_r = (bg_color >> 10)&31;

            int r = 4*(((superpixel[j][i] >> 10)&31) - bg_r);
           
            if (i > 0)
                r += ((superpixel[j][i-1] >> 10)&31) - bg_r;
            if (i < SCREEN_W-1)
                r += ((superpixel[j][i+1] >> 10)&31) - bg_r;
            if (j > 0)
                r += ((superpixel[j-1][i] >> 10)&31) - bg_r;
            if (j < SCREEN_H-1)
                r += ((superpixel[j+1][i] >> 10)&31) - bg_r;

            r = r/8 + bg_r;
            if (r < 0)
                r = 0;
            else if (r > 31)
                r = 31;
            
            superpixel[j][i] = (r<<10) | (superpixel[j][i] & 1023);
        }
        break;
    case 1:

        for (int i=0; i<SCREEN_W; ++i)
        {
            uint8_t bg_g = (bg_color >> 5)&31;

            int g = 4*(((superpixel[j][i] >> 5)&31) - bg_g);
           
            if (i > 0)
                g += ((superpixel[j][i-1] >> 5)&31) - bg_g;
            if (i < SCREEN_W-1)
                g += ((superpixel[j][i+1] >> 5)&31) - bg_g;
            if (j > 0)
                g += ((superpixel[j-1][i] >> 5)&31) - bg_g;
            if (j < SCREEN_H-1)
                g += ((superpixel[j+1][i] >> 5)&31) - bg_g;

            g = g/8 + bg_g;
            if (g < 0)
                g = 0;
            else if (g > 31)
                g = 31;

            superpixel[j][i] = (g<<5) | (superpixel[j][i] & 31775);
        }
        break;
    case 2:
        for (int i=0; i<SCREEN_W; ++i)
        {
            uint8_t bg_b = (bg_color)&31;

            int b = 4*(((superpixel[j][i])&31) - bg_b);
           
            if (i > 0)
                b += ((superpixel[j][i-1])&31) - bg_b;
            if (i < SCREEN_W-1)
                b += ((superpixel[j][i+1])&31) - bg_b;
            if (j > 0)
                b += ((superpixel[j-1][i])&31) - bg_b;
            if (j < SCREEN_H-1)
                b += ((superpixel[j+1][i])&31) - bg_b;

            b = b/8 + bg_b;
            if (b < 0)
                b = 0;
            else if (b > 31)
                b = 31;

            superpixel[j][i] = (superpixel[j][i] & 32736) | b;
        }
        break;
    }
}
