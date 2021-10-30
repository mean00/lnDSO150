#
# Simple RLE packing with magic byte = 0x76
#

from PIL import Image
from struct import *
import sys

compressed=0
mpretty=0
stuct_name=""
def print_external_header(f):
    global struct_name
    f.write("#define "+str(struct_name)+"_width "+str(width)+"\n")
    f.write("#define "+str(struct_name)+"_height "+str(height)+"\n")
    f.write("extern const unsigned char "+str(struct_name)+"[];\n");
def print_header(f):
    global struct_name
    f.write("const unsigned char "+str(struct_name)+"[]={\n");
def print_footer(f):
    f.write("};\n")

def printout(value):
    global mpretty 
    global compressed 
    f.write("0x"+format(value,"02x")+",")
    mpretty+=1
    compressed+=1
    if(mpretty>15):
        mpretty=0
        f.write("\n")

if 5!=len(sys.argv):
    print("convert intput.png output.bin decl.h var_name")
    exit(1)
f= open(sys.argv[2], 'wt')
image=Image.open(sys.argv[1])
struct_name=sys.argv[4]
(width,height) = image.size
pixels = image.load
print("loaded image "+str(sys.argv[1])+" "+str(width)+"x"+str(height))
print_header(f)
arr = bytearray(width*height)
out = bytearray(width*height)
for y in range(0,height):
    x=0
    # First pack them as a byte array
    while x<width:
        arr[x+y*width]=image.getpixel((x,y))
        x+=1

#  convert to 1 bit per pixel
n=width*height
mask=0x80
xx=0
value=0
for i in range(0,n):
   if arr[i] >0x0:
       value+=mask

   mask>>=1
   if mask==0:
       out[xx]=value
       mask=0x80
       xx+=1
       value=0
xx+=1
print("%d bytes to pack" % xx)
# Then RLE pack them
x=0 
while x<xx:
    current=out[x]
    count=1
    while (x+count<xx) and (current == out[x+count]) and count<254 :
       count+=1
    #print("Val=%x count=%d" % (current,count))
    if(count >3 or current==0x76):
        x+=count
        printout(0x76)
        printout(current)
        printout(count)
    else:
        printout(current)
        x+=1
print("Incoming size = %d => %d" % (width*height, compressed))
#        print(str(x)+":  "+str(current)+"x"+str(count))
print_footer(f)
f.close()
f= open(sys.argv[3], 'wt')
print_external_header(f);
f.close()
print("Done generating "+str(sys.argv[2])+"\n")
