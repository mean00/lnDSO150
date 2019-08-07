/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#pragma once




/**
 */
class DSODisplay
{
public:  
/**
 */
typedef enum              
{
            INVALID_MODE=0,
            VOLTAGE_MODE=1,
            TIME_MODE=2,
            TRIGGER_MODE=3,
            VOLTAGE_MODE_ALT=VOLTAGE_MODE+0x80,
            TIME_MODE_ALT=TIME_MODE+0x80,
            TRIGGER_MODE_ALT=TRIGGER_MODE+0x80,
}MODE_TYPE;

public:
            
public:
            static void  init();
            static void  drawWaveForm(int count,const uint8_t *data);
            static void  drawGrid(void);
            static void  drawVerticalTrigger(bool drawOrErase,int column);
            static void  drawVoltageTrigger(bool drawOrErase, int line);
  
            static void  drawStats(CaptureStats &stats);
            static void  drawStatsBackGround();
            static void  printVoltTimeTriggerMode(const char *volt, const char *time,DSOCapture::TriggerMode mode);
            static void  drawMode(MODE_TYPE mode);
            static MODE_TYPE getMode();
            static void  setMode(MODE_TYPE t);
            static void  printTriggerValue(float volt); 
            static void  printOffset(float volt); 
            
            static void  drawAutoSetup();
            static void  drawAutoSetupStep(int x);
            static void  triggered(bool gotIt);
};