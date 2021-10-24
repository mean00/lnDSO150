from DSO150 import DSO150
import time
dso=DSO150()

for v in DSO150.DsoVoltage:
    print("Setting "+v.name)
    dso.SetVoltage( v)
    volt=dso.GetVoltage() 
    print("Getting "+volt.name)
    time.sleep(2)
    

