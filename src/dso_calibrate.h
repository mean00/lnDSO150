/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
class DSOCalibrate
{
  public:
    static bool loadCalibrationData();
    static bool zeroCalibrate();
    static bool decalibrate();
    static bool voltageCalibrate();
    static bool vccAdcMenu();
    static bool fiveVoltCalibrate();
    static bool formatNvm();

  protected:
    static bool voltageCalibrate_();
    static bool zeroCalibrate_();
    static bool fiveVoltCalibrate_();
};