
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
        
    def expect_reply(self, expected):
        #print("expect reply\n")
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
        #print("wait_reply\n")   
        msg = self.expect_reply("msg_r")
        if msg is None:
            print("Unexpected reply\n")
            return False
        r = msg.msg_r.s
        #print("::"+str(r))
        match r:
            case defines_pb2.OK:
                #print("reply OK\n");
                return True # ok
            case _ :
                print("Call failure\n")
                return False
    #
    #
    #
    def simple_send(self,msg):
        
        data = msg.SerializeToString()
        #print(">>simple_send, size= "+str(len(data))+"\n")
        self.messager.send_message(data)
        return self.wait_reply()
    #
    #
    #
    def set_voltage_range(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_sv.SetInParent()
        to_send.msg_sv.voltage = value
        return self.simple_send(to_send)
    #
    #
    #        
    def set_time_base(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_stb.SetInParent()
        to_send.msg_stb.timebase = value
        return self.simple_send(to_send)     
    #
    #
    #
    def set_trigger(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_str.SetInParent()
        to_send.msg_str.trigger = value
        return self.simple_send(to_send)             
    #
    #
    #
    def get_data(self):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_data.SetInParent()
        to_send.msg_data.dummy = 0
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.expect_reply("msg_rdata")
        if reply is None:
            print("cant get reply to set time base")
            return None
        #
        if reply.msg_rdata.s!=defines_pb2.OK:
            print("read data ko")
            return None
        # the next message are plain screen dump
        return  self.messager.read_message()

    #
    #
    #         
    def get_time_base(self):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_gtb.SetInParent()
        to_send.msg_gtb.dummy = 0
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.expect_reply("msg_stb")
        if reply is None:
            print("cant get reply to set time base")
            return None
        return defines_pb2.TIMEBASE.Name(reply.msg_stb.timebase)
    #
    #
    #
    def get_voltage_range(self):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_gv.SetInParent()
        to_send.msg_gv.dummy = 0
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.expect_reply("msg_sv")
        if reply is None:
            print("cant get reply to set time base")
            return None
        return defines_pb2.VOLTAGE.Name(reply.msg_sv.voltage)

    def get_trigger(self):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_gtr.SetInParent()
        to_send.msg_gtr.dummy = 0
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.expect_reply("msg_str")
        if reply is None:
            print("cant get reply to set trigger")
            return None
        return defines_pb2.TRIGGER.Name(reply.msg_str.trigger)

    def set_trigger_value(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_stv.SetInParent()
        to_send.msg_stv.trigger_value = value
        return self.simple_send(to_send)             

    def get_trigger_value(self):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_gtv.SetInParent()
        to_send.msg_gtr.dummy = 0
        data = to_send.SerializeToString()
        self.messager.send_message(data)
        reply = self.expect_reply("msg_stv")
        if reply is None:
            print("cant get reply to set trigger")
            return None
        return reply.msg_stv.trigger_value


#-- EOF --       
   