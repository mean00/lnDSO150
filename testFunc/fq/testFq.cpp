#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "math.h"


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
int computeFrequency(int xsamples,uint16_t *data)
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
    bool lookingForMax=false;
   
    //printf("Threshold %d %d\n",lowThreshold,highThreshold);
 
    for(int i=1;i<samples && nbSample<15;i++)
    {
        //printf("i:%d old=%d new=%d val=%d\n",i,old,xnew,ptr[0]); 
        ptr+=1;
        if(!lookingForMax) // looking for min
        {
            if(ptr[0]>lowThreshold && ptr[1]<=lowThreshold)
            {
            //printf("MIN AT %d\n",i);
            lookingForMax=true;
            continue;
            }
        }else
        {
            if(ptr[0] <= highThreshold && ptr[1]>highThreshold)
            {
                fdelta[nbSample++]=i;
                //printf("MAX at %d\n",i);
                lookingForMax=false;
            }

            continue;
        }
    }
    if(nbSample<2) return 0; // not enough points
    int sum=0;
    for(int i=1;i<nbSample;i++)
    {
        sum+=fdelta[i]-fdelta[i-1];
    }
    sum=(1000*sum+nbSample/2)/nbSample;
 
    return sum;
}
#define NB_SAMPLES 240
#define cycles 10.
int main(int a, char **b)
{
   uint16_t samples[NB_SAMPLES]; 

    for(int i=0;i<NB_SAMPLES;i++)
    {
        float f=sin((i*2.*3.1415*cycles)/NB_SAMPLES);
        int noise=rand()&0xff;
        noise>>=4;
        samples[i]=f*150+2048+noise;
    }
   printf("1:%d\n",  computeFrequency(NB_SAMPLES,samples));
    int halfcycle=NB_SAMPLES/cycles;
    for(int i=0;i<NB_SAMPLES;i++)
    {
        int h=i%halfcycle;
        if(h<(halfcycle/2)) h=0;
            else h=1;
        int noise=rand()&0xff;
        noise>>=4;
        samples[i]=h*150+2048+noise;
    }
/*
    for(int i=0;i<NB_SAMPLES;i++)
      printf("%d: %d\n",i,samples[i]);
*/
   printf("2:%d\n",  computeFrequency(NB_SAMPLES,samples));
   return 0;
}



