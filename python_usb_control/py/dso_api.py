
import messaging_pb2
from defs import defines_pb2
import dso_protocol
#
#
#
class DSO_API:
    #
    #
    #

    def __init__(self, io):
        self.io = io
        self.messager = dso_protocol.Messaging(self.io)
    #
    #
    #
        
    def except_reply(self, expected):
        print("simple_send\n")
        data = self.messager.read_message()
        msg = messaging_pb2.UnionMessage().FromString(data)
        #print(str(msg)+"->"+expected)
        if msg.HasField(expected) is None:
            print("Wrong reply to query")
            return None
        return msg
    #
    #
    #        
    def wait_reply(self):     
        print("simple_send\n")   
        msg = self.except_reply("msg_r")
        if msg is None:
            print("Unexpected reply\n")
            return False
        r = msg.msg_r.s
        #print("::"+str(r))
        match r:
            case defines_pb2.OK:
                print("reply OK\n");
                return True # ok
            case _ :
                print("Call failure\n")
                return False
    #
    #
    #
    def simple_send(self,msg):
        print("simple_send\n")
        data = msg.SerializeToString()
        self.messager.send_message(data)
        return self.wait_reply()
    #
    #
    #
    def set_voltage_range(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_sv.voltage = value
        return self.simple_send(to_send)
    #
    #
    #        
    def set_time_base(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_stb.timebase = value
        return self.simple_send(to_send)     
    #
    #
    #         
    def get_time_base(self):
        to_send = messaging_pb2.UnionMessage()
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.except_reply("msg_gtb")
        if reply is None:
            print("cant get reply to set time base")
            return None
        return defines_pb2.TIMEBASE(reply.msg_gtb.timebase)
    #
    #
    #
    def get_voltage(self):
        to_send = messaging_pb2.UnionMessage()
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.except_reply("msg_gv")
        if reply is None:
            print("cant get reply to set time base")
            return None
        return defines_pb2.VOLTAGE(reply.msg_gv.voltage)     

#-- EOF --       
   