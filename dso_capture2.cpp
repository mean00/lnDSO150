/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#define DEFAULT_VALUE  0x400*0x10000
static int fdelta[16];
static int dex=0;
/**
 * Compute # of samples between two local maximum  , that gives ~ the frequency
 * This is not very accurate
 * 
 * @param shifted
 * @param xsamples
 * @param data
 * @return 
 */
int DSOCapturePriv::computeFrequency(bool shifted,int xsamples,uint32_t *data)
{
    // This is done after transform, we can scatch input
    int samples=xsamples-1;
    int neg=-1,pos=-1;
    int nbSample=0;
    int sum=0,xmin;
    static int t;
    t=micros();
    int16_t *ptr=(int16_t *)data;
    if(shifted) ptr++;
    int32_t old=    (int32_t)(ptr[2])-(int32_t)(ptr[0]);        
    ptr+=2;
    for(int i=1;i<samples;i++)
    {
        int32_t xnew=    (int32_t)(ptr[2])-(int32_t)(ptr[0]);  
        ptr+=2;
        if(xnew<=0 && old>0 ) 
        {
            if(pos>0 && neg>0 && (i-pos)>2)
            {
                fdelta[nbSample++]=i-pos;
                if(nbSample>10) break;
            }
            pos=i;
        }
        if(xnew>=0 && old<0 && (i-neg)>2) 
        {
            neg=i;
        }
        old=xnew;
    }
    if(!nbSample)
        return 0;  
        
    for(int i=0;i<nbSample;i++)
    {
        sum+=fdelta[i];
    }
    sum=(1000*sum+nbSample/2)/nbSample;
    return sum;
}
// EOF
