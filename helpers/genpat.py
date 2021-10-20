#
import array as arr
#
width=200
def genArray(ar,tx):
    out="const uint16_t "+tx+"["+str(width)+"]={\n"
    for i in range(0,width):
        out+=str(ar[i])
        out+=str(",")
        if((i % 20)==19):
            out+="\n"
    out+="};\n"
    print(out)

patdef = arr.array('i')
patlightgreen = arr.array('i')
patdarkgreen = arr.array('i')
#
darkgreen=0x1f<<5 # Dark green
lightgreen=0x2f<<5 # Light green
for i in range(0,width):
    patdef.append(0)
    patlightgreen.append(lightgreen)
    patdarkgreen.append(darkgreen)
    if( (i%20)==0):
        patdef[i]=darkgreen
#
patdef[0]=          lightgreen
patdef[width-1]=    lightgreen
midpoint=int(1+width/2)
patdef[midpoint] = lightgreen

genArray(patdef,"defaultPattern")
genArray(patlightgreen,"lightGreenPattern")
genArray(patdarkgreen,"darkGreenPattern")
#

