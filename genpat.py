#
import array as arr

patdef = arr.array('i')
patyellow = arr.array('i')
patdarkgreen = arr.array('i')
darkgreen=15*32 # Dark green
yellow=23
for i in range(0,240):
    patdef.append(0)
    patyellow.append(yellow)
    patdarkgreen.append(darkgreen)
    if( (i/24)*24==i):
        patdef[i]=darkgreen
    
out="const uint16_t defaultPattern[240]={"
for i in range(0,240):
    out+=str(patdef[i])
    out+=str(",")
print(out)
out="const uint16_t yellowPattern[240]={"
for i in range(0,240):
    out+=str(patyellow[i])
    out+=str(",")
print(out)
print("};\n")
out="const uint16_t darkGreenPattern[240]={"
for i in range(0,240):
    out+=str(patdarkgreen[i])
    out+=str(",")
print(out)
print("};\n")
