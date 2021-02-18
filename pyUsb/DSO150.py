import serial
import serial.tools.list_ports;
from enum import Enum, unique
import struct
 
#
class DSO150:
    @unique
    class DsoArmingMode(Enum):
        SINGLE=0
        MULTI=1
        CONTINUOUS=2
    @unique
    class DsoTrigger(Enum):
      RISING=0
      FALLING=1
      BOTH=2
      RUN=3

    @unique
    class DsoCommand(Enum):
      GET=1
      SET=2
      ACK=3
      NACK=4
      EVENT=5
        
    @unique
    class DsoTarget(Enum):
      VOLTAGE=1
      TIMEBASE=2
      TRIGGER=3
      ARMINGMODE=4
      DATA=5
      TRIGGERLEVEL=6
      FIRMWARE=10

    @unique
    class DsoVoltage(Enum):
        GND=0
        mV5=1
        mV10=2
        mV20=3
        mV50=4
        mV100=5
        mV200=6
        mV500=7
        V1=8
        V2=9
        V5=10
        def asFloat(self):
            if(self.name.startswith("G")):
                return 0.
            if(self.name.startswith("mV")):
                v=self.name[2:]
                return float(v)/1000.
            return float(self.name[1:])
    
    @unique
    class DsoTimeBase(Enum):
        u5=0
        u10=1
        u25=2
        u50=3
        u100=4
        u200=5
        u500=6
        m1=7
        m2=8
        m5=9
        m10=10
        m20=11
        m50=12
        m100=13
        m200=14
        m500=15
        s1=16

    def close(self):
        self.ser.close()             # close port

    def __init__(self):
        #First search for our DSO
        device=""
        self.ser=""
        for  port in serial.tools.list_ports.comports():
            if port.vid==0x1eaf and port.pid==0x24 :
                print("Found DSO as "+port.device)
                device=port.device
        if(len(device)==0):
            print("No DSO found\n")
            exit -1
        # Then go
        self.ser = serial.Serial(device,115200,timeout=2)  # open serial port
        # Handshake
        self.ser.write(b'DSO0')     # write a string
        handshake=self.ser.read(4)
        if handshake !=b'OSD0':
            print( "Handshake failed\n")
            print( handshake)
            exit(-1)
        print("Handshake successful")
       
    def sendCommand(self,command,target,value):
        rList = [ command.value, target.value, value>>8, value &0xff]
        ba=bytearray(rList)
        self.ser.write(ba)
    #
    def Get(self,target):
        self.sendCommand(self.DsoCommand.GET,target,0)
        ret=self.ser.read(4)
        if(len(ret)==0):
            print("no reply to Get Command  to "+target.name)
            exit(-1)
        if(ret[0]!=3):
            print("Command Get to "+target.name+"failed r="+str(ret[0]))
            exit(-1)
        returnCode=ret[2]*256+ret[3] 
        return returnCode
    #
    def Set(self,target,value):
        self.sendCommand(self.DsoCommand.SET,target,value)
        ret=self.ser.read(4)
        if(len(ret)!=4):
            print("no reply to Command Set  to "+target.name)
            exit(-1)
        if(ret[0]!=3):
            print("Command Get to "+target.name+"failed r="+str(ret[0]))
            exit(-1)
    # Voltage Helper function
    def SetVoltage(self,value):
        return self.Set(self.DsoTarget.VOLTAGE,value.value)
    def GetVoltage(self):
        return self.DsoVoltage(self.Get(self.DsoTarget.VOLTAGE))

   # Timebase Helper function
    def SetTimeBase(self,value):
        return self.Set(self.DsoTarget.TIMEBASE,value.value)
    def GetTimeBase(self):
        return self.DsoTimeBase(self.Get(self.DsoTarget.TIMEBASE))
    # Trigger Helper function
    def SetTrigger(self,value):
        return self.Set(self.DsoTarget.TRIGGER,value.value)
    def GetTrigger(self):
        return self.DsoTrigger(self.Get(self.DsoTarget.TRIGGER))
    # Arming mode
    def SetArmingMode(self,value):
        return self.Set(self.DsoTarget.ARMINGMODE,value.value)
    def GetArmingMode(self):
        return self.DsoArmingMode(self.Get(self.DsoTarget.ARMINGMODE))
     # TriggerLevel 
    def SetTriggerLevel(self,value):
        value1000=(value*100.)
        thousand=int(value1000)+32768
        return self.Set(self.DsoTarget.TRIGGERLEVEL,thousand)
    def GetTriggerLevel(self):
        i= self.Get(self.DsoTarget.TRIGGERLEVEL)
        return float(i-32768)/100.;
# capture
    def GetDataInternal(self,wait):
        # Ask for a capture
        self.Set(self.DsoTarget.DATA,wait)
        loop=True
        while loop:
            ret=self.ser.read(4)
            if(len(ret)==4):
                loop=False
        # 
        if(ret[0]!= 5):
            print("Not an event! "+str(ret[3]))
            exit(-1)
        count=ret[2]*256+ret[3]
        data=[]
        for i in range(0,count):
            ret=self.ser.read(4)
            if(len(ret)!=4):
                print("Error in data at offset "+str(i) +" size="+str(len(ret)))
                exit(-1)
            #it=(ret[0]<<24)+(ret[1]<<16)+(ret[2]<<8)+(ret[3])
            it=(ret[3]<<24)+(ret[2]<<16)+(ret[1]<<8)+(ret[0])
            f=struct.unpack('f', struct.pack('I', it))[0]
            data.append(f)
        return data
    # Ask for a new capture
    def GetData(self):
        return self.GetDataInternal(0)
    # Returns already captured data
    def GetCurrentData(self):
        return self.GetDataInternal(1)

#
# EOF
