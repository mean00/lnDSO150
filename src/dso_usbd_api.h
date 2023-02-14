



class DSO_API {
public:
        static void init( void);
        static bool setVoltage(int ) ; //DSOCapture::DSO_VOLTAGE_RANGE r);
        static bool setTimeBase(int) ; //DSOCapture::DSO_TIME_BASE gb);
        static bool setTrigger(int) ; //DSOCapture::TriggerMode tm);
        static bool setTriggerValue(float);

        static int getVoltage();
        static int getTimeBase();
        static int getTrigger();
        static float getTriggerValue();
};
