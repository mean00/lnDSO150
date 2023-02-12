import socket
import sys
from abc  import *
from typing import Any
import importlib

sys.path.append('../tmp_py')
sys.path.append('../tmp_py/defs')

import messaging_pb2
import defs


#
#
#
class IO(metaclass=ABCMeta):
    def __init(self):
        pass

    @abstractmethod
    def init(self):
        pass
    @abstractmethod
    def read(self,max):
        pass
    
    @abstractmethod
    def write(self,data:bytearray):
        pass

#
#
#

class NetworkingIO(IO):
    def __init__(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        pass

    def init(self):
        try:
            self.socket.connect(("localhost", 3000))
            return True
        except:
            return False

    def read(self,max):
        return self.socket.recv(max)
    
    def write(self,data:bytearray):
        return self.socket.send(data)


#
#
#
class automat:
    def __init__(self,ii):
        self.io = ii
        self.state=0
        self.size = 0
        self.dex = 0
        self.data = []
        self.result = False
    #
    def get_message(self):
        if self.result is False:
            return False
        return bytearray(self.data)
    #
    def run(self):
        v=self.io.read(1)[0]
        match self.state:
            case 0:
                if v==0x53:
                    self.state =1
                    return True
                else:
                    print("Bad head "+str(v),flush=True)
                    return False
            case 1:
                    self.size = v
                    self.state = 2
                    return True
            case 2:
                    self.size = self.size + (v<<8)
                    self.state = 3
                    return True
            case 3:
                    self.data.append(v)
                    self.dex = self.dex+1
                    if self.dex == self.size:
                        self.state = 4
                    return True
            case 4:
                    print("Message of len "+str(self.dex))
                    if v!=0x45:
                        print("BAD TAIL:"+str(v),flush=True)
                        self.result = False
                    else:
                        print("Got MSG ",flush=True)
                        self.result = True
                    return False

class Messaging:
    def __init__(self,io):
        self.io = io
    def send_message(self, msg : bytearray):
        n=len(msg)
        # Send "S" + size in LSB 
        header=[ 0x53, (n&0xff), (n>>8) & 0xff]
        self.io.write(bytearray(header))
        # Send body
        self.io.write(msg)
        # Send tail
        self.io.write(bytearray([0x45])) # E
    def read_message(self):
        automaton = automat(self.io)
        while True:
            if automaton.run() is False:
                r =  automaton.get_message()
                return r



 
n = NetworkingIO()
if n.init() is False:
    print("Cannot connect")
    exit(1)
n.write (bytearray(b'DO'))
# Wait for DO
data=n.read(2)
if (data == b'OD'): #and (data[1] == b'D'):
    print("Handshake ok")
else:
    print("Handshake failure "+str(data))
# Send dummy message
messager = Messaging(n)

print("Sending ping",flush=True)
person = messaging_pb2.setVoltageRange()
person.volt = defs.defines_pb2.DSO_VOLTAGE_1V #defs.DSO_VOLTAGE_10MV
msg = person.SerializeToString()
messager.send_message(bytearray(msg))
#msg=[0,1,2]

#messager.send_message(bytearray(msg))
print("Reading pong",flush=True)
reply=messager.read_message()
print(str(msg)+":"+str(reply),flush=True)
print("OK",flush=True)

