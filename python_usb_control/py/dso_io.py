import socket
import sys
from abc  import *
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




