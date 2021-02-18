from DSO150 import DSO150
import numpy as np
import cv2 as cv
import time
# Grab data from DSO
dso=DSO150()
volt=0

def getAndDraw():
    print("Asking for a capture ")
    data=dso.GetCurrentData() 
    volt=dso.GetVoltage().asFloat()
    scale=-24./volt
    #
    l=len(data)
    last=int(data[0]*scale)
    for x in range(0,l):
        y=int(data[x]*scale)
        cv.line(img,(x,y+yOffset),(x,last+yOffset),(0,255,255),1)
        last=y
    

# 
dsoWidth=240
dsoHeight=240
border=120
yOffset=int(dsoHeight/2)
# Create a black image
img = np.zeros((dsoWidth+1,dsoHeight+1+border,3), np.uint8)
# Draw grid
for x in range(0,dsoWidth+1,24):
    cv.line(img,(x,0),(x,dsoHeight-1),(0,128,0),1)
for y in range(0,dsoHeight+1,24):
    cv.line(img,(0,y),(dsoWidth-1,y),(0,128,0),1)
# Center lines
cv.line(img,(0,yOffset),(dsoWidth-1,yOffset),(0,255,0),1)
cv.line(img,(120,0),(120,dsoHeight-1),(0,255,0),1)
# Volt/div and timebase
cv.putText(img,dso.GetVoltage().name, (dsoWidth+2,20), cv.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)
cv.putText(img,dso.GetTimeBase().name, (dsoWidth+2,60), cv.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)

getAndDraw()
cv.imwrite("output.png",img)
cv.imshow("dso", img)
print("Done ")
cv.waitKey(0)
