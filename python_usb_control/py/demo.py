import sys
import time
from abc  import *
from typing import Any
import importlib

sys.path.append('../tmp_py')
sys.path.append('../tmp_py/defs')

import dso_io
import dso_protocol
import dso_api

from defs import defines_pb2



 
#n = dso_io.NetworkingIO()
n = dso_io.UsbCDC()
if n.init() is False:
    print("Cannot connect")
    exit(1)

if n.handshake()==False:
    print("Handshake failure")
    exit(1)
print("Handshake ok")

api = dso_api.DSO_API(n)

#default configuration
print("setup\n")
api.set_trigger(defines_pb2.DSO_TRIGGER_FALLING)
api.set_time_base(defines_pb2.DSO_TIME_BASE_1MS)
api.set_voltage_range(defines_pb2.DSO_VOLTAGE_1V)

print("Asking for data")
data = api.get_data()
print(str(len(data))+str(data))
exit(1)

print("setting trigger value\n")
if False:
    trigger_voltage = -2.
    while trigger_voltage < 2.:
        print("Setting trigger value to "+str(trigger_voltage))
        api.set_trigger_value(trigger_voltage)
        print( "  Reading back : "+ str(api.get_trigger_value()))
        time.sleep(2)
        trigger_voltage = trigger_voltage +0.4
    print("getting timebase\n")



print("setting timebase\n")
for i in range(11):
    print("Setting timebase to "+str(defines_pb2.TIMEBASE.Name(i)))
    api.set_time_base(i)
    print( "  Reading back : "+ str(api.get_time_base()))
    time.sleep(1.)
print("getting timebase\n")

api.set_time_base(defines_pb2.DSO_TIME_BASE_1MS)

print("setting voltage\n")
for i in range(10):
    print("Setting voltage to "+str(defines_pb2.VOLTAGE.Name(i)))
    api.set_voltage_range(i)
    print( "  Reading back : "+ str(api.get_voltage_range()))
    time.sleep(1.)

api.set_voltage_range(defines_pb2.DSO_VOLTAGE_1V)
print("getting voltage\n")

for i in range(4):
    print("Setting trigger to "+str(defines_pb2.TRIGGER.Name(i)))
    api.set_trigger(i)
    print( "  Reading back : "+ str(api.get_trigger()))
    time.sleep(1.)


print("setting trigger\n")
api.set_trigger(defines_pb2.DSO_TRIGGER_FALLING)
print("getting trigger\n")
trg = api.get_trigger()


print("OK",flush=True)

