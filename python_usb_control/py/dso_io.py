import socket
import sys
from abc  import *
import serial
import serial.tools.list_ports;

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

    def handshake(self):
        self.write (bytearray(b'DO'))
        # Wait for DO
        data=self.read(2)
        if (data == b'OD'): #and (data[1] == b'D'):            
            return True
        else:            
            return False


#
#
#

class NetworkingIO(IO):
    def __init__(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connected = False
        pass

    def init(self):
        try:
            self.socket.connect(("localhost", 3000))
            self.connected = True
            return True
        except:
            return False

    def read(self,max):
        return self.socket.recv(max)
    
    def write(self,data:bytearray):
        return self.socket.send(data)


class UsbCDC(IO):
    def __init__(self):
        self.device = None
        self.ser = None
    def init(self):
        device=""
        self.ser=""
        for  port in serial.tools.list_ports.comports():
            if port.vid==0x1eaf and port.pid==0x24 :
                print("Found DSO as "+port.device)
                self.device=port.device
        if(self.device is None):
            print("No DSO found\n")
            return False
        self.ser = serial.Serial(self.device,115200,timeout=2)  # open serial port
        return True
     
    def read(self,max):
        return self.ser.read(max)
    
    def write(self,data:bytearray):
        return self.ser.recv(data)




