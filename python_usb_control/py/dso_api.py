
import messaging_pb2
import defs
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
        
    def wait_reply(self):
        data = self.messager.read_message()
        msg = messaging_pb2.UnionMessage().FromString(data)
        print(str(msg))
        if not (msg.HasField("msg_r") is None):
            print("Wrong reply to query")
            return False
        r = msg.msg_r.s
        match r:
            case 0:
                return True # ok
            case _ :
                print("Call failure")
                return False
    #
    #
    #
    def simple_send(self,msg):
        data = msg.SerializeToString()
        self.messager.send_message(data)
        return self.wait_reply()
    #
    #
    #
    def set_voltage_range(self, value):
        to_send = messaging_pb2.UnionMessage()
        to_send.msg_sv.volt = value
        return self.simple_send(to_send)
       
   