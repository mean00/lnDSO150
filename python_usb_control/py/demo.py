import sys
from abc  import *
from typing import Any
import importlib

sys.path.append('../tmp_py')
sys.path.append('../tmp_py/defs')

import dso_io
import dso_protocol
import dso_api

import defs



 
n = dso_io.NetworkingIO()
if n.init() is False:
    print("Cannot connect")
    exit(1)

if n.handshake()==False:
    print("Handshake failure")
    exit(1)
print("Handshake ok")

api = dso_api.DSO_API(n)
api.set_voltage_range(defs.defines_pb2.DSO_VOLTAGE_1V)

print("OK",flush=True)

