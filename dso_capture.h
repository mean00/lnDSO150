
/**
 */
class DSOCapture
{
public:
    enum
    {
      DSO_MODE_CONTINOUS,
      DSO_MODE_TRIGGERED_SINGLE,
      DSO_MODE_TRIGGERED_CONTINUOUS,
    };
    enum DSO_TIME_BASE 
    {
      DSO_TIME_BASE_10US,
      DSO_TIME_BASE_25US,
      DSO_TIME_BASE_50US,
      DSO_TIME_BASE_100US,
      DSO_TIME_BASE_500US,
      DSO_TIME_BASE_1MS,
      DSO_TIME_BASE_5MS,
      DSO_TIME_BASE_10MS,
      DSO_TIME_BASE_50MS,
      DSO_TIME_BASE_100MS,
      DSO_TIME_BASE_500MS,
      DSO_TIME_BASE_1S
    };
    static bool     setTimeBase(DSO_TIME_BASE timeBase);
    static bool     initiateSampling (int count);
    static uint32_t *getSamples(int &count);
    static void     reclaimSamples(uint32_t *buffer);

    
protected:
  
};
