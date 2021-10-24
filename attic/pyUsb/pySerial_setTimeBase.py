from DSO150 import DSO150
import time
dso=DSO150()

for v in DSO150.DsoTimeBase:
    print("Setting "+v.name)
    dso.SetTimeBase( v)
    tb=dso.GetTimeBase() 
    print("Getting "+tb.name)
    time.sleep(2)
    

