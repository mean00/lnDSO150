# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: defs/defines.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x12\x64\x65\x66s/defines.proto*\x18\n\x06STATUS\x12\x06\n\x02OK\x10\x00\x12\x06\n\x02KO\x10\x01*\xf6\x01\n\x07VOLTAGE\x12\x13\n\x0f\x44SO_VOLTAGE_GND\x10\x00\x12\x13\n\x0f\x44SO_VOLTAGE_5MV\x10\x01\x12\x14\n\x10\x44SO_VOLTAGE_10MV\x10\x02\x12\x14\n\x10\x44SO_VOLTAGE_20MV\x10\x03\x12\x14\n\x10\x44SO_VOLTAGE_50MV\x10\x04\x12\x15\n\x11\x44SO_VOLTAGE_100MV\x10\x05\x12\x15\n\x11\x44SO_VOLTAGE_200MV\x10\x06\x12\x15\n\x11\x44SO_VOLTAGE_500MV\x10\x07\x12\x12\n\x0e\x44SO_VOLTAGE_1V\x10\x08\x12\x12\n\x0e\x44SO_VOLTAGE_2V\x10\t\x12\x12\n\x0e\x44SO_VOLTAGE_5V\x10\n*\xaa\x02\n\x08TIMEBASE\x12\x16\n\x12\x44SO_TIME_BASE_10US\x10\x00\x12\x16\n\x12\x44SO_TIME_BASE_20US\x10\x01\x12\x16\n\x12\x44SO_TIME_BASE_50US\x10\x02\x12\x17\n\x13\x44SO_TIME_BASE_100US\x10\x03\x12\x17\n\x13\x44SO_TIME_BASE_200US\x10\x04\x12\x17\n\x13\x44SO_TIME_BASE_500US\x10\x05\x12\x15\n\x11\x44SO_TIME_BASE_1MS\x10\x06\x12\x15\n\x11\x44SO_TIME_BASE_2MS\x10\x07\x12\x15\n\x11\x44SO_TIME_BASE_5MS\x10\x08\x12\x16\n\x12\x44SO_TIME_BASE_10MS\x10\t\x12\x16\n\x12\x44SO_TIME_BASE_20MS\x10\n\x12\x16\n\x12\x44SO_TIME_BASE_50MS\x10\x0b*e\n\x07TRIGGER\x12\x16\n\x12\x44SO_TRIGGER_RISING\x10\x00\x12\x17\n\x13\x44SO_TRIGGER_FALLING\x10\x01\x12\x14\n\x10\x44SO_TRIGGER_BOTH\x10\x02\x12\x13\n\x0f\x44SO_TRIGGER_RUN\x10\x03')

_STATUS = DESCRIPTOR.enum_types_by_name['STATUS']
STATUS = enum_type_wrapper.EnumTypeWrapper(_STATUS)
_VOLTAGE = DESCRIPTOR.enum_types_by_name['VOLTAGE']
VOLTAGE = enum_type_wrapper.EnumTypeWrapper(_VOLTAGE)
_TIMEBASE = DESCRIPTOR.enum_types_by_name['TIMEBASE']
TIMEBASE = enum_type_wrapper.EnumTypeWrapper(_TIMEBASE)
_TRIGGER = DESCRIPTOR.enum_types_by_name['TRIGGER']
TRIGGER = enum_type_wrapper.EnumTypeWrapper(_TRIGGER)
OK = 0
KO = 1
DSO_VOLTAGE_GND = 0
DSO_VOLTAGE_5MV = 1
DSO_VOLTAGE_10MV = 2
DSO_VOLTAGE_20MV = 3
DSO_VOLTAGE_50MV = 4
DSO_VOLTAGE_100MV = 5
DSO_VOLTAGE_200MV = 6
DSO_VOLTAGE_500MV = 7
DSO_VOLTAGE_1V = 8
DSO_VOLTAGE_2V = 9
DSO_VOLTAGE_5V = 10
DSO_TIME_BASE_10US = 0
DSO_TIME_BASE_20US = 1
DSO_TIME_BASE_50US = 2
DSO_TIME_BASE_100US = 3
DSO_TIME_BASE_200US = 4
DSO_TIME_BASE_500US = 5
DSO_TIME_BASE_1MS = 6
DSO_TIME_BASE_2MS = 7
DSO_TIME_BASE_5MS = 8
DSO_TIME_BASE_10MS = 9
DSO_TIME_BASE_20MS = 10
DSO_TIME_BASE_50MS = 11
DSO_TRIGGER_RISING = 0
DSO_TRIGGER_FALLING = 1
DSO_TRIGGER_BOTH = 2
DSO_TRIGGER_RUN = 3


if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _STATUS._serialized_start=22
  _STATUS._serialized_end=46
  _VOLTAGE._serialized_start=49
  _VOLTAGE._serialized_end=295
  _TIMEBASE._serialized_start=298
  _TIMEBASE._serialized_end=596
  _TRIGGER._serialized_start=598
  _TRIGGER._serialized_end=699
# @@protoc_insertion_point(module_scope)
