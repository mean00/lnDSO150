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
    int  msSinceOk();
protected:
    bool armed;
    uint32_t mil;
    uint32_t newMil;
};