
/**
 * 
 */
#if 0
lnBasicDelayTimer *bTimer;
float f;
void foo(void *)
{
    f+=0.1;
    bTimer->arm(10);
}

void setup()
{
    Logger("Timer test\n");
    bTimer=new lnBasicDelayTimer(0);
    bTimer->setInterrupt(foo,NULL);
    bTimer->arm(5);
    while(1)
    {
        xDelay(100);
        Logger("bTimer\n");
    }
    xMutex *PortBMutex=new xMutex;
    arbitrer=new DSO_portArbitrer(1,PortBMutex);
    
    control=new DSOControl(NULL); // control must be initialised after ili !
    control->setup();
    
    testSignal=new DSO_testSignal(PIN_TEST_SIGNAL,PIN_TEST_SIGNAL_AMP);
    testSignal->setFrequency(1*1000);
    DSOCapture::initialize(PA0);
    
    nvm=new lnNvmGd32();
    if(!nvm->begin())
    {
        Logger("Nvm not operational, reformating... \n");
        nvm->format();
    }
}
#else