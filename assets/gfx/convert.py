#
# Simple RLE packing with magic byte = 0x76
#

from PIL import Image
from struct import *
import sys
import struct
import os

compressed=0
mpretty=0
stuct_name=""

mode_to_bpp = {'1':1, 'L':8, 'P':8, 'RGB':24, 'RGBA':32, 'CMYK':32, 'YCbCr':24, 'I':32, 'F':32}


def print_decl(inp):
    global struct_name
    f=open(inp,'wt')
    f.write("#define "+str(struct_name)+"_width "+str(width)+"\n")
    f.write("#define "+str(struct_name)+"_height "+str(height)+"\n")
    f.write("extern const unsigned char "+str(struct_name)+"[];\n");
    f.close()
def print_header(f):
    global struct_name
    #/f.write("#define "+str(struct_name)+"_width "+str(width)+"\n")
    #f.write("#define "+str(struct_name)+"_height "+str(height)+"\n")
    f.write("const unsigned char "+str(struct_name)+"[]={\n");
def print_footer(f):
    f.write("};\n")
    flat=int(width*height/8)
    f.write("// flat size = "+str(flat)+"\n")
    f.write("// compressed size = "+str(compressed)+"\n")
    print(str(flat)+"==>"+str(compressed))
def printout(value):
    global mpretty 
    f.write("0x"+format(value,"02x")+",")
    mpretty+=1
    if(mpretty>15):
        mpretty=0
        f.write("\n")

if 5!=len(sys.argv):
    print("convert intput.png output.bin var_name")
    exit(1)
f= open(sys.argv[2], 'wt')
hsfile=sys.argv[2]+".hs"
binfile=sys.argv[2]+".bin"
declfile=sys.argv[3]
image=Image.open(sys.argv[1])
mode=image.mode
bpp=mode_to_bpp[mode]
struct_name=sys.argv[4]
(width,height) = image.size
pixels = image.load
print("loaded image "+str(sys.argv[1])+" "+str(width)+"x"+str(height))
print("image is "+str(width)+" x "+str(height))
print("bpp "+str(bpp) +str(mode))

print_header(f)
arr = bytearray(width)
sz=width*height
sz=(sz+7)/8
out = bytearray(int(sz))
xx=0
for y in range(0,height):
    x=0
    # First pack them as a byte array
    while x<width:
        pix=image.getpixel((x,y))
        #print(str(pix))
        if bpp==32:
            (a,b,c,d)=pix
            pix=int((a+b+c)/3)
            pix=255-pix
        else:
            if bpp==8:
                if mode=='P':
                    pix=255*int(pix)
                    if pix > 255:
                        pix=255
                else:
                    pix=int(pix)
                    
            else:
                print("Unsupport format!") 
        #print(str(pix))
        arr[x]=pix
        x+=1
    # 8 bits -> 1 bit
    mask=0x80
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
# Save to binary file
out_bytes=bytes(out)
raw=open(binfile,'wb')
raw.write(out_bytes)
raw.close()
# compress with heatshrink
command="/usr/local/bin/heatshrink -w 8 -l 4 "+binfile+" "+hsfile
print("Compressing "+binfile+" to "+hsfile)
os.system(command)
# Output as header file
hs=open(hsfile,'rb')
while True:
  c=hs.read(1)
  if c:
    printout(int.from_bytes(c, byteorder='big'))
    compressed=compressed+1
  else:
    break
    
hs.close()

#
print_footer(f)
f.close()
print_decl(declfile)
print("Done generating "+str(sys.argv[2])+"\n")
