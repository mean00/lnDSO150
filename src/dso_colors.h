#pragma once


#define ILI_MK_COLOR(b,g,r) (((r&0x1f)<<11)+((g&0x3f)<<5)+(b&0x1F))
#define ILI_BLACK       0
#define ILI_WHITE       ILI_MK_COLOR(255,255,255)
#define ILI_GREEN       ILI_MK_COLOR(0,255,0)
#define ILI_YELLOW      ILI_MK_COLOR(0x1f,0x3f,0)
#define ILI_RED         ILI_MK_COLOR(0x1F,0,0)
#define ILI_BLUE        ILI_MK_COLOR(0,0,0x1F)
#define ILI_PINK        ILI_MK_COLOR(255,105,180)


#define BLACK ILI_BLACK
#define WHITE ILI_WHITE
#define GREEN ILI_GREEN
#define YELLOW ILI_YELLOW
#define RED   ILI_RED
#define BLUE  ILI_BLUE
#define PINK  ILI_PINK
