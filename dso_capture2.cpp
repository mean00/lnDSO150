/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#define DEFAULT_VALUE  0x400*0x10000
static int fdelta[16];
static int dex=0;
int DSOCapture::computeFrequency(bool shifted,int xsamples,uint32_t *data)
{
    // This is done after transform, we can scatch input
    int samples=xsamples-1;
    int neg=-1,pos=-1;
    int32_t xmin=DEFAULT_VALUE;

    static int t;
    t=micros();
    
    int32_t old=    (int32_t)(data[1]>>(16*shifted))-(int32_t)(data[0]>>(16*shifted));        
    for(int i=1;i<samples;i++)
    {
        int32_t xnew=    (int32_t)(data[i+1]>>(16*shifted))-(int32_t)(data[i]>>(16*shifted));  
        if(xnew<=0 && old>0 ) 
        {
            if(pos>0 && neg>0 && (i-pos)>2)
            {
                xmin=i-pos;
                break;
            }
            pos=i;
        }
        if(xnew>=0 && old<0 && (i-neg)>2) 
        {
            neg=i;
        }
        old=xnew;
    }
    if(xmin==DEFAULT_VALUE)
        xmin=0;   
    fdelta[dex]= (micros()-t);
    dex++;
    dex%=16;
    return xmin;
}
// EOF
