import sys
from abc  import *
from typing import Any
import importlib

sys.path.append('../tmp_py')
sys.path.append('../tmp_py/defs')

import dso_io
import dso_protocol
import dso_api

from defs import defines_pb2



 
n = dso_io.NetworkingIO()
if n.init() is False:
    print("Cannot connect")
    exit(1)

if n.handshake()==False:
    print("Handshake failure")
    exit(1)
print("Handshake ok")

api = dso_api.DSO_API(n)

print("setting timebase\n")
api.set_time_base(defines_pb2.DSO_TIME_BASE_2MS)
print("getting timebase\n")
tb= api.get_time_base()
print("<TB:>"+str(tb))

print("setting voltage\n")
api.set_voltage_range(defines_pb2.DSO_VOLTAGE_1V)
print("getting voltage\n")
volt = api.get_voltage()
print("<VT:>"+str(volt))

print("setting trigger\n")
api.set_trigger(defines_pb2.DSO_TRIGGER_FALLING)
print("getting trigger\n")
trg = api.get_trigger()
print("<TR:>"+str(trg))



print("OK",flush=True)

