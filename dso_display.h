/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#pragma once


typedef enum              
{
            VOLTAGE_MODE,
            TIME_MODE,
            TRIGGER_MODE
}MODE_TYPE;



/**
 */
class DSODisplay
{
public:
            static void  init();
            static void  drawWaveForm(int count,const uint8_t *data);
            static void  drawGrid(void);
            static void  drawVerticalTrigger(bool drawOrErase,int column);
            static void  drawVoltageTrigger(bool drawOrErase, int line);
  
            static void  drawStats(CaptureStats &stats);
            static void  drawStatsBackGround();
            static void  drawVoltTime(const char *volt, const char *time);
            static void  drawMode(MODE_TYPE mode);
            static MODE_TYPE getMode();
            static void  setMode(MODE_TYPE t);
};