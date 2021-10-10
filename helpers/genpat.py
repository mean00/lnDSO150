#
import array as arr
#
width=192
def genArray(ar,tx):
    out="const uint16_t "+tx+"["+str(width)+"]={\n"
    for i in range(0,width):
        out+=str(ar[i])
        out+=str(",")
        if((i % 24)==23):
            out+="\n"
    out+="};\n"
    print(out)

patdef = arr.array('i')
patlightgreen = arr.array('i')
patdarkgreen = arr.array('i')
#
darkgreen=0xf<<5 # Dark green
lightgreen=0x2f<<5 # Light green
for i in range(0,width):
    patdef.append(0)
    patlightgreen.append(lightgreen)
    patdarkgreen.append(darkgreen)
    if( (i/24)*24==i):
        patdef[i]=darkgreen
#
patdef[0]=          lightgreen
patdef[width-1]=    lightgreen
patdef[1+width/2] = lightgreen

genArray(patdef,"defaultPattern")
genArray(patlightgreen,"lightGreenPattern")
genArray(patdarkgreen,"darkGreenPattern")
#

