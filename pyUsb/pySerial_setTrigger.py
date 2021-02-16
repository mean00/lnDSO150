from DSO150 import DSO150
import time
dso=DSO150()

for v in DSO150.DsoTrigger:
    print("Setting "+v.name)
    dso.SetTrigger( v)
    tb=dso.GetTrigger() 
    print("Getting "+tb.name)
    time.sleep(2)
    

