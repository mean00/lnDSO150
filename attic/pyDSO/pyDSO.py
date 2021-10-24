from PyQt5 import uic,QtWidgets,QtCore, QtGui
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from DSO150 import DSO150
import numpy as np
import cv2 as cv

import sys, time

class Ui(QtWidgets.QMainWindow):
    scene = None
    scale = 1.0
    menuVoltage = None
    menuTimeBase = None
    menuTrigger = None
    menuVoltageAg = None
    buttonSave = None
    dso = None
    data = None
    newCapture = None
    labelVoltage = None
    labelTimeBase = None
    voltage = DSO150.DsoVoltage.mV5
    timebase= DSO150.DsoTimeBase.u100
    trigger=  DSO150.DsoTrigger.RISING
    yellow=QColor(qRgb(255,255,0))
    
    def alert(self, msg):    
        msg = QMessageBox()
        msg.setText(msg)
        msg.exec_()
    
    def __init__(self):
        super(Ui,self).__init__()
        uic.loadUi("ui/dso.ui",self)
        self.gv=  self.findChild(QtWidgets.QGraphicsView, 'graphicsView')
        self.scene = QtWidgets.QGraphicsScene()
        self.gv.setScene(self.scene)
        self.drawGrid()
        self.dso=DSO150()
        self.voltage=self.dso.GetVoltage()
        self.timebase=self.dso.GetTimeBase()
        self.trigger=self.dso.GetTrigger()        
        
                
        self.menuVoltage=self.findChild(QtWidgets.QMenu, 'menuVoltage')
        self.menuTimeBase=self.findChild(QtWidgets.QMenu, 'menuTimeBase')
        self.menuTrigger=self.findChild(QtWidgets.QMenu, 'menuTrigger')
        self.newCapture=self.findChild(QtWidgets.QPushButton, 'pushButtonReq')
        self.currentCapture=self.findChild(QtWidgets.QPushButton, 'pushButtonCurrent')
        self.buttonSave=self.findChild(QtWidgets.QPushButton, 'pushButtonSave')
        
        

        self.labelVoltage=self.findChild(QtWidgets.QLabel, 'labelVoltage')
        self.labelTimeBase=self.findChild(QtWidgets.QLabel, 'labelTimeBase')

        self.labelTimeBase.setText(self.timebase.name)
        self.labelVoltage.setText(self.voltage.name)


        self.menuVoltageAg = QtWidgets.QActionGroup(self.menuVoltage)
        self.menuVoltageAg.setExclusive(True)
        for v in DSO150.DsoVoltage:
            menu = QAction( v.name, self)
            menu.setCheckable(True)
            menu.setData(v)
            if(v==self.voltage):
                menu.setChecked(True)
            ma= self.menuVoltageAg.addAction(menu)
            self.menuVoltage.addAction(ma)
            
        
        
        self.menuTimeBaseAg = QtWidgets.QActionGroup(self.menuTimeBase)
        self.menuTimeBaseAg.setExclusive(True)
        for v in DSO150.DsoTimeBase:
            menu = QAction( v.name, self)
            menu.setCheckable(True)
            if(v==self.timebase):
                menu.setChecked(True)
            menu.setData(v)
            ma= self.menuTimeBaseAg.addAction(menu)
            self.menuTimeBase.addAction(ma)
            
            
        self.menuTriggerAg = QtWidgets.QActionGroup(self.menuTrigger)
        self.menuTriggerAg.setExclusive(True)
        for v in DSO150.DsoTrigger:
            menu = QAction( v.name, self)
            menu.setCheckable(True)
            menu.setData(v)
            if(v==self.trigger):
                menu.setChecked(True)
            ma= self.menuTriggerAg.addAction(menu)
            self.menuTrigger.addAction(ma)
            
        self.menuVoltageAg.triggered.connect(self.onVoltageChange)
        self.menuTimeBaseAg.triggered.connect(self.onTimeBaseChange)
        self.menuTriggerAg.triggered.connect(self.onTriggerChange)
        self.newCapture.clicked.connect(self.onNewCapture)
        self.currentCapture.clicked.connect(self.onCurrentCapture)
        self.buttonSave.clicked.connect(self.onSave)
        self.show()
        
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
        self.data=self.dso.GetData()
        self.drawWaveForm(self.data)
    def onCurrentCapture(self):
        self.data=self.dso.GetCurrentData()
        self.drawWaveForm(self.data)
    def onTimeBaseChange(self, n):
        v=n.data()
        print("New timebase "+str(v))
        self.timebase=v
        self.dso.SetTimeBase(v)
        self.labelTimeBase.setText(v.name)
        pass
    def onTriggerChange(self, n):
        v=n.data()
        print("New trigger "+str(v))
        self.trigger=v
        self.dso.SetTrigger(v)
        pass
    def onVoltageChange(self, n):
        v=n.data()
        print("New voltage "+str(v))
        self.voltage=v
        self.dso.SetVoltage(v)
        self.labelVoltage.setText(v.name)
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
    def volt2pix(self, volt):
          y=int(volt*self.scale)+120
          return int(y)
    def drawWaveForm(self, data):
        self.drawGrid()
        volt=self.dso.GetVoltage().asFloat()
        self.scale=-24./volt
        l=len(data)
        last=self.volt2pix(data[0])
        for x in range(0,l):
            y=self.volt2pix(data[x])
            line=QLineF(x,last,x,y)
            self.scene.addLine(line,self.yellow)            
            last=y      

#
app =QtWidgets.QApplication(sys.argv)
mainWindow=Ui()
app.exec_()
