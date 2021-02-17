from DSO150 import DSO150
import time
dso=DSO150()

# For 
for i in range(-2000,2000,200):
    v=float(i)
    v/=1000.
    print("Setting "+str(v))
    dso.SetTriggerLevel( v)
    tb=dso.GetTriggerLevel() 
    print("Getting "+str(tb))
    time.sleep(2)
    

