from DSO150 import DSO150

dso=DSO150()
v=dso.Get( DSO150.DsoTarget.VOLTAGE)
print("Voltage = "+str(v))
dso.Set( DSO150.DsoTarget.VOLTAGE,DSO150.DsoVoltage.V1) # Set 1v
dso.Set( DSO150.DsoTarget.TIMEBASE,DSO150.DsoTimeBase.u500) # Set 500u /div
t=dso.Get( DSO150.DsoTarget.TIMEBASE)
print("timebase = "+str(t))

