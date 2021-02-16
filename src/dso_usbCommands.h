#pragma once
namespace DSOUSB
{
enum DSOUSB_Command
{
    GET=1,
    SET=2,
    ACK=3,
    NACK=4,
    EVENT=5            
};
enum DSOUSB_Target
{
    VOLTAGE=1,
    TIMEBASE=2,
    TRIGGER=3,
    CAPTUREMODE=4,
    DATA=5,
    FIRMWARE=10
};

enum DSOUSB_VOLTAGE
{
        GND=0,
        mV5=1,
        mV10=2,
        mV20=3,
        mV50=4,
        mV100=5,
        mV200=6,
        mV500=7,
        V1=8,
        V5=9,
 
};
enum DSOUSB_TIMEBASE
{
        u5=0,
        u10=1,
        u25=2,
        u50=3,
        u100=4,
        u200=5,
        u500=6,
        m1=7,
        m2=8,
        m5=9,
        m10=10,
        m20=11,
        m50=12,
        m100=13,
        m200=14,
        m500=15,
        s1=16
};

};