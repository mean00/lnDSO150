from DSO150 import DSO150
import time
dso=DSO150()

for v in reversed(DSO150.DsoArmingMode):
    print("Setting "+v.name)
    dso.SetArmingMode( v)
    tb=dso.GetArmingMode() 
    print("Getting "+tb.name)
    time.sleep(2)
    

