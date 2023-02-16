from PyQt5 import uic,QtWidgets,QtCore, QtGui
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
import numpy as np
import cv2 as cv

import sys, time

# DSO control module
sys.path.append('../py')

import dso_io
import dso_protocol
import dso_api

from defs import defines_pb2
#
refresh_period = 300
#
def valuenum_in_enum( enum_wrapper, valuenum):
        try:
            if enum_wrapper.Name(valuenum):
                return True
        except ValueError:
            return False

#___________________________________
def fill_menu( inst, wrapper, parent_menu,parent_parent, current, prefix):
    for v in range(0,20):
        if valuenum_in_enum(wrapper, v):
            name =  wrapper.Name(v)
            name = name.removeprefix(prefix)
            #print("::"+name+"::"+prefix)
            menu = QAction( name, inst)
            menu.setCheckable(True)
            menu.setData(v)
            if(v==current):
                print("::"+str(v)+"::"+prefix)
                menu.setChecked(True)
            ma= parent_menu.addAction(menu)
            parent_parent.addAction(ma)
            

#___________________________________


class Ui(QtWidgets.QMainWindow):
       
    def alert(self, txt):    
        msg = QMessageBox()
        msg.setText(txt)
        msg.exec_()
    # Check the value is valid in enumWWrapper
    def __init__(self):
        self.scene = None
        self.scale = 1.0
        self.menuVoltage = None
        self.menuTimeBase = None
        self.menuTrigger = None
        self.menuVoltageAg = None
        self.buttonSave = None
        self.dso_api = None
        self.data = None
        self.newCapture = None
        self.labelVoltage = None
        self.labelTimeBase = None
        self.yellow=QColor(qRgb(255,255,0))
        self.timer = None
        super(Ui,self).__init__()
        uic.loadUi("ui/dso.ui",self)
        self.gv=  self.findChild(QtWidgets.QGraphicsView, 'graphicsView')
        self.scene = QtWidgets.QGraphicsScene()
        self.gv.setScene(self.scene)
        #
        n = dso_io.UsbCDC()
        if n.init() is False:
            self.alert("Cannot connect")
            exit(1)

        if n.handshake()==False:
            self.alert("Handshake failure")
            exit(1)
        print("Handshake ok")

        self.dso_api = dso_api.DSO_API(n)

        self.drawGrid()
        self.voltage = defines_pb2.VOLTAGE.Value(self.dso_api.get_voltage_range())
        self.timebase= defines_pb2.TIMEBASE.Value(self.dso_api.get_time_base())
        self.trigger=  defines_pb2.TRIGGER.Value(self.dso_api.get_trigger())
        
                
        self.menuVoltage=self.findChild(QtWidgets.QMenu, 'menuVoltage')
        self.menuTimeBase=self.findChild(QtWidgets.QMenu, 'menuTimeBase')
        self.menuTrigger=self.findChild(QtWidgets.QMenu, 'menuTrigger')
        self.newCapture=self.findChild(QtWidgets.QPushButton, 'pushButtonReq')
        self.currentCapture=self.findChild(QtWidgets.QPushButton, 'pushButtonCurrent')
        self.buttonSave=self.findChild(QtWidgets.QPushButton, 'pushButtonSave')
        
        

        self.labelVoltage=self.findChild(QtWidgets.QLabel, 'labelVoltage')
        self.labelTimeBase=self.findChild(QtWidgets.QLabel, 'labelTimeBase')

        name =  defines_pb2.TIMEBASE.Name( self.timebase ).removeprefix('DSO_TIME_BASE_')
        self.labelTimeBase.setText(name)

        name =  defines_pb2.VOLTAGE.Name( self.voltage ).removeprefix('DSO_VOLTAGE_')
        self.labelVoltage.setText( name)

        self.timer = QTimer()

        
        self.menuVoltageAg = QtWidgets.QActionGroup(self.menuVoltage)
        self.menuVoltageAg.setExclusive(True)
        fill_menu( self, defines_pb2.VOLTAGE, self.menuVoltageAg, self.menuVoltage, self.voltage,'DSO_VOLTAGE_')
        
        
        self.menuTimeBaseAg = QtWidgets.QActionGroup(self.menuTimeBase)
        self.menuTimeBaseAg.setExclusive(True)
        fill_menu( self, defines_pb2.TIMEBASE, self.menuTimeBaseAg, self.menuTimeBase, self.timebase,"DSO_TIME_BASE_")
            
            
        self.menuTriggerAg = QtWidgets.QActionGroup(self.menuTrigger)
        self.menuTriggerAg.setExclusive(True)
        fill_menu( self, defines_pb2.TRIGGER, self.menuTriggerAg, self.menuTrigger, self.trigger,"DSO_TRIGGER_")
            
        self.menuVoltageAg.triggered.connect(self.onVoltageChange)
        self.menuTimeBaseAg.triggered.connect(self.onTimeBaseChange)
        self.menuTriggerAg.triggered.connect(self.onTriggerChange)
        self.newCapture.clicked.connect(self.onNewCapture)
        self.currentCapture.clicked.connect(self.onCurrentCapture)
        self.buttonSave.clicked.connect(self.onSave)
        self.timer.timeout.connect(self.onCurrentCapture)
        self.show()
        self.timer.start(refresh_period )
    def onSave(self):
        yOffset=120
        zoom=2
        img = np.zeros((240*zoom+1+10,320*zoom+1+2+96,3), np.uint8)
        for x in range(0,240+1,24):
            cv.line(img,(x*zoom,0),(x*zoom,240*zoom-1),(0,128,0),1)
        for y in range(0,240+1,24):
            cv.line(img,(0,y*zoom),(240*zoom-1,y*zoom),(0,128,0),1)

        cv.line(img,(0,yOffset*zoom),(240*zoom-1,yOffset*zoom),(0,255,0),1)
        cv.line(img,(120*zoom,0),(120*zoom,240*zoom-1),(0,255,0),1)
        # add legend
        cv.putText(img,"Volt:"+self.voltage.friendlyName()+"/div", (240*zoom+10,24), cv.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)
        cv.putText(img,"Time:"+self.timebase.friendlyName()+"/div", (240*zoom+10,64), cv.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)
        l=len(self.data)
        last=int(self.data[0]*self.scale)
        for x in range(0,l):
            y=int(self.data[x]*self.scale)
            cv.line(img,(x*zoom,(y+yOffset)*zoom),(x*zoom,(last+yOffset)*zoom),(0,255,255),1)
            last=y
        cv.imwrite("output.png",img)
        #cv.imshow("dso", img)

    def onNewCapture(self):
        pass
    def onCurrentCapture(self):
        self.data=self.dso_api.get_data()
        self.drawWaveForm(self.data)
    def onTimeBaseChange(self, n):
        v=n.data()
        print("New timebase "+str(v))
        self.timebase=v
        self.dso_api.set_time_base(v)
        name =  defines_pb2.TIMEBASE.Name( v ).removeprefix('DSO_TIME_BASE_')
        self.labelTimeBase.setText( name )
        pass
    def onTriggerChange(self, n):
        v=n.data()
        print("New trigger "+str(v))
        self.trigger=v
        self.dso_api.set_trigger(v)
        pass
    def onVoltageChange(self, n):
        v=n.data()
        print("New voltage "+str(v))
        self.voltage=v
        self.dso_api.set_voltage_range(v)
        #self.labelVoltage.setText(v.name)
        pass
    
    def drawGrid(self):
        black=QColor(qRgb(0,0,0))
        darkGreen=QColor(qRgb(0,128,0))
        lightGreen=QColor(qRgb(0,255,0))
        
        bgnd = QtWidgets.QGraphicsRectItem(QtCore.QRectF(0, 0, 240, 240)) 
        bgnd.setBrush( black )
        self.scene.addItem(bgnd)
        for i in range(0,240,24):
            hline=QLineF(0,i,239,i)
            self.scene.addLine(hline,darkGreen)
            vline=QLineF(i,0,i,239)
            self.scene.addLine(vline,darkGreen)
    def drawWaveForm(self, data):
        self.drawGrid()
        volt=self.dso_api.get_voltage_range()
        l=len(data)
        last=data[0]
        for x in range(0,l):
            y=data[x]
            line=QLineF(x,last,x,y)
            self.scene.addLine(line,self.yellow)            
            last=y      

#
app =QtWidgets.QApplication(sys.argv)
mainWindow=Ui()
app.exec_()
