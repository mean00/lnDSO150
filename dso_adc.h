#include <Wire.h>


// Sampling Queue

#define SAMPLING_QUEUE_SIZE 4
class SampleingQueue
{
public:
    SampleingQueue()
    {
        count=0;
    }
    void addFromIsr(uint32_t *ptr)
    {
        data[count++]=ptr;
    }
    void add(uint32_t *ptr)
    {
        noInterrupts();
        addFromIsr(ptr);
        interrupts();
    }
    bool empty()
    {
        return !count;
    }
    uint32_t *takeFromIsr()
    {
        if(!count) return NULL;
        uint32_t *out=data[0];
        memmove(data,data+1,(count-1)*4);
        count--;
        return out;
    }
    uint32_t *take()
    {
        uint32_t *out;
        noInterrupts();
        out=takeFromIsr();
        interrupts();
        return out;
    }

protected:
    uint32_t *data[SAMPLING_QUEUE_SIZE];
    int      count;
    
    
};

/**
 * 
 */
class DSOADC
{
public:
                    DSOADC();
            bool    setTimeScale(adc_smp_rate one, adc_prescaler two);
            bool    initiateSampling (int count);
            uint32_t *getSamples(int &count);
            void     reclaimSamples(uint32_t *buffer);
            
            
            
protected:
            bool startSampling (int count,uint32_t *buffer);
            void setADCs ();
    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;    
    static  void DMA1_CH1_Event();
            void captureComplete();
            int  _sampled;
            
            
            SampleingQueue availableBuffers;
            SampleingQueue capturedBuffers;
            
};

// EOF
