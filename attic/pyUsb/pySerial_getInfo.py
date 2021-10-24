from DSO150 import DSO150
dso=DSO150()

volt=DSO150.DsoVoltage(dso.Get( DSO150.DsoTarget.VOLTAGE)) # Set 1v
timebase=DSO150.DsoTimeBase(dso.Get( DSO150.DsoTarget.TIMEBASE)) # Set 500u /div
fw=dso.Get( DSO150.DsoTarget.FIRMWARE) 
print(volt)
print(timebase)
maj=fw>>8
min=fw&0xff
print("Firmware "+str(maj)+"."+str(min))

