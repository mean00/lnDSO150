#pragma once

class StopWatch
{
public:
    StopWatch()
    {
      armed=false;
      
    }
    void ok();
    bool elapsed(int threshold);
  
protected:
    bool armed;
    uint32_t mil;
    uint32_t newMil;
};