#pragma once
/**
 */
class singleButton
{
public:
    singleButton()
    {
        _state=DSOControl::StateIdle;
        _events=0;
        _holdOffCounter=0;
        _pinState=0;
        _pinCounter=0;
    }
    bool holdOff() // Return true if in holdoff mode
    {
        if(_state!=DSOControl::StateHoldOff)
            return false;
        _holdOffCounter++;
        if(_holdOffCounter>HOLDOFF_THRESHOLD)
        {
            _state=DSOControl::StateIdle;
            return false;
        }
        return true;
    }
    void goToHoldOff()
    {
        _state=DSOControl::StateHoldOff;
        _holdOffCounter=0;
    }
    void reset()
    {
        _pinCounter=0;
        _events=0;
        _pinState=0;
    }
    void integrate(bool k)
    {         
        // Integrator part
        if(k)
        {
            _pinCounter++;
        }else
        {
            if(_pinCounter) 
            {
                if(_pinCounter>COUNT_MAX) _pinCounter=COUNT_MAX;
                else
                    _pinCounter--;
            }
        }
    }

    int runMachine(int oldCount)
    {

        int oldPin=_pinState;
        int newPin=_pinCounter>(COUNT_MAX-1);
        int r=0;
        int s=oldPin+oldPin+newPin;
        switch(s)
        {
            default:
                xAssert(0);
                break;
            case 0: // flat
                break;
            case 2:
            { // released
                if(_state==DSOControl::StatePressed)
                {                        
                    if(oldCount>SHORT_PRESS_THRESHOLD)
                    {
                        _events|=EVENT_SHORT_PRESS;
                        r++;
                    }
                }
                goToHoldOff();
                break;
            }
            case 1: // Pressed
                _state=DSOControl::StatePressed;
                break;
            case 3: // Still pressed
                if(_pinCounter>LONG_PRESS_THRESHOLD && _state==DSOControl::StatePressed) // only one long
                {
                    _state=DSOControl::StateLongPressed;
                    _events|=EVENT_LONG_PRESS;   
                    r++;
                }
                break;
        }                       
        _pinState=newPin;
        return r;
    }        
                    
    DSOControl::DSOButtonState _state;
    int            _events;
    int            _holdOffCounter;
    int            _pinState;
    int            _pinCounter;
};
//
