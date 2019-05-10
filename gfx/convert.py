#
# Simple RLE packing with magic byte = 0x76
#

from PIL import Image
from struct import *
import sys

mpretty=0
stuct_name=""
def print_header(f):
    global struct_name
    f.write("#define "+str(struct_name)+"_width "+str(width)+"\n")
    f.write("#define "+str(struct_name)+"_height "+str(height)+"\n")
    f.write("const unsigned char "+str(struct_name)+"[]={\n");
def print_footer(f):
    f.write("};\n")

def printout(value):
    global mpretty 
    f.write("0x"+format(value,"02x")+",")
    mpretty+=1
    if(mpretty>15):
        mpretty=0
        f.write("\n")

if 4!=len(sys.argv):
    print("convert intput.png output.bin var_name")
    exit(1)
f= open(sys.argv[2], 'wt')
image=Image.open(sys.argv[1])
struct_name=sys.argv[3]
(width,height) = image.size
pixels = image.load
print("loaded image "+str(sys.argv[1])+" "+str(width)+"x"+str(height))
print_header(f)
arr = bytearray(width)
out = bytearray(512)
for y in range(0,height):
    x=0
    # First pack them as a byte array
    while x<width:
        arr[x]=image.getpixel((x,y))
        x+=1
    # 8 bits -> 1 bit
    mask=0x80
    xx=0
    value=0
    for x in range(0,width):
        if arr[x] >0x40:
            value+=mask
        mask>>=1
        if mask==0:
            out[xx]=value
            mask=0x80
            xx+=1
            value=0
    x=0 
    while x<xx:
        current=out[x]
        count=1
        while (x+count<xx) and (current == out[x+count]) and count<255 :
           count+=1
        if(count >3 or current==0x76):
            x+=count
            printout(0x76)
            printout(current)
            printout(count)
        else:
            printout(current)
            x+=1
#        print(str(x)+":  "+str(current)+"x"+str(count))
print_footer(f)
f.close()
print("Done generating "+str(sys.argv[2])+"\n")
