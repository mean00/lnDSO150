#if 0
/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"

#include "DSO_config.h"
#define DEFAULT_VALUE  0x400*0x10000
static int fdelta[16];
static int dex=0;

static void getMinMax(int xsamples, uint16_t *data, int *xmi, int *xma)
{
    *xmi=4096+1;
    *xma=0;
    for(int i=0;i<xsamples;i++)
    {
        int v=(int)data[i];
        if(v>*xma) *xma=v;
        if(v<*xmi) *xmi=v;
    }
}

/**
 * Compute # of samples between two local maximum  , that gives ~ the frequency
 * This is not very accurate
 * 
 * @param shifted
 * @param xsamples
 * @param data
 * @return 
 */
int DSOCapturePriv::computeFrequency(int xsamples,uint16_t *data)
{
    int xmin,xmax;
    getMinMax(xsamples, data, &xmin,&xmax);
    int third=(xmax-xmin)/3;
    if(third<10) return 0; // they are too close
    int lowThreshold=xmin+third;
    int highThreshold=xmax-third;
    
    int samples=xsamples-1;
    int nbSample=0;
 
    uint16_t *ptr=(uint16_t *)data;    
    bool lookingForMax=false;
     
    for(int i=0;i<samples && nbSample<15;i++)
    {
        if(!lookingForMax) // looking for min
        {
            if(ptr[0]>lowThreshold && ptr[1]<=lowThreshold)
            {
                lookingForMax=true;
            }
        }else
        {
            if(ptr[0] <= highThreshold && ptr[1]>highThreshold)
            {
                fdelta[nbSample++]=i;
                lookingForMax=false;
            }
        }
        ptr++;
    }

    if(nbSample<3) return 0; // not enough points
    int sum=0;
    int *v=fdelta;
    int usableSamples=nbSample-1;
    for(int i=0;i<usableSamples;i++)
    {
        sum+=v[1]-v[0];
        v++;
    }
    sum=(1000*sum)/usableSamples; 
    return sum;
}
// EOF

#endif