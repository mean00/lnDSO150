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
    int old=    (int)(ptr[1])-(int)(ptr[0]);        
    ptr+=1;
    bool lookingForMax=true;
    int lastMax=0;
    int theMin=-1;
    for(int i=1;i<samples;i++)
    {
        int xnew=    (int)(ptr[1])-(int)(ptr[0]);  
        ptr+=1;
        if(!lookingForMax && ptr[0]<lowThreshold)
            if(xnew>0 && old<=0 )  // minimum
            {
                theMin=i;
                lookingForMax=true;
                 old=xnew;
                continue;
            }
        if(lookingForMax && ptr[0] > highThreshold)
            if(xnew<0 && old>=0) 
            {
                if(lastMax)
                {
                    fdelta[nbSample++]=i-lastMax;   
                    if(nbSample>6) break;
                }
                lastMax=i;
                lookingForMax=false;
            }   
        old=xnew;
    }
    if(nbSample<2) return 0; // not enough points
    int sum=0;
    for(int i=0;i<nbSample;i++)
    {
        sum+=fdelta[i];
    }
    sum=(1000*sum+nbSample/2)/nbSample;
 
    return sum;
}
// EOF
