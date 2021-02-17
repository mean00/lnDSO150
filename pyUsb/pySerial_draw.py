from DSO150 import DSO150
import numpy as np
import cv2 as cv
import time
# Grab data from DSO
dso=DSO150()


# 
dsoWidth=320
dsoHeight=240
yOffset=int(dsoHeight/2)
# Create a black image
print("Asking for a capture ")
data=dso.GetData() 
img = np.zeros((dsoWidth+1,dsoHeight+1,3), np.uint8)
# Draw gris
for x in range(0,dsoWidth+1,24):
    cv.line(img,(x,0),(x,dsoHeight-1),(0,128,0),1)
for y in range(0,dsoHeight+1,24):
    cv.line(img,(0,y),(dsoWidth-1,y),(0,128,0),1)

cv.line(img,(0,yOffset),(dsoWidth-1,yOffset),(0,255,0),1)
cv.line(img,(int(dsoWidth/2),0),(int(dsoWidth/2),dsoHeight-1),(0,255,0),1)

# 
scale=-24.
#
l=len(data)
last=int(data[0]*scale)
for x in range(0,l):
    y=int(data[x]*scale)
    cv.line(img,(x,y+yOffset),(x,last+yOffset),(255,255,255),1)
    last=y
cv.imshow("dso", img)
print("Done ")
cv.waitKey(0)
