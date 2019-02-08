#include <Wire.h>
/**
 * 
 */
class DSOADC
{
public:
                    DSOADC();
            bool    setTimeScale(int timeScaleUs);
            void    takeSamples (int count);
            uint32_t *getSamples(int &count);
protected:
            void setADCs ();
    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;
    static  void DMA1_CH1_Event();
            int  _sampled;
};

// EOF
