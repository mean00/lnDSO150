from DSO150 import DSO150
import time
dso=DSO150()

# For 
for v in range(-2.,2.,0.200):
    print("Setting "+str(v))
    dso.SetTriggerValue( v)
    tb=dso.GetTriggerValue() 
    print("Getting "+str(tb))
    time.sleep(2)
    

