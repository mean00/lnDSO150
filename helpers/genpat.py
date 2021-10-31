#
import array as arr
#
width=10*20
horiz=12*20
def genArray(ar,tx):
    out="const uint16_t "+tx+"["+str(horiz)+"]={\n"
    for i in range(0,horiz):
        out+=str(ar[i])
        out+=str(",")
        if((i % 20)==19):
            out+="\n"
    out+="};\n"
    print(out)

def genHoriz(ar,tx):
    out="const uint16_t "+tx+"["+str(horiz)+"]={\n"
    for i in range(0,horiz):
        out+=str(ar[i])
        out+=str(",")
        if((i % 20)==19):
            out+="\n"
    out+="};\n"
    print(out)

pathoriz = arr.array('i')
patdef = arr.array('i')
patlightgreen = arr.array('i')
patdarkgreen = arr.array('i')
#
darkgreen=0x1f<<5 # Dark green
lightgreen=0x2f<<5 # Light green
# Horizontal
for i in range(0,horiz):
    pathoriz.append(0)
    if( (i%20)==0):
        pathoriz[i]=darkgreen
midpoint=int(horiz/2)
pathoriz[midpoint] = lightgreen
# vertical
for i in range(0,horiz):
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
genHoriz(pathoriz,"horizontal")
#

