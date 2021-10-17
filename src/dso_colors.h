#pragma once


#define ILI_MK_COLOR(b,g,r) (((r&0x1f)<<11)+((g&0x3f)<<5)+(b&0x1F))
#define BLACK       0
#define WHITE       ILI_MK_COLOR(255,255,255)
#define GREEN       ILI_MK_COLOR(0,255,0)
#define YELLOW      ILI_MK_COLOR(0x1f,0x3f,0)
#define RED         ILI_MK_COLOR(0x1F,0,0)
#define BLUE        ILI_MK_COLOR(0,0,0x1F)