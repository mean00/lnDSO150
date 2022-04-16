#/usr/bin/python3
import sys
import xxhash;

#
#_____________
def usage():
    print("lnDsoChecksum : Add a checksum to the fw to check its integrity");
    print("\tlnDsoChecksum.py intput_bin_file checksumed_bin_file\n");
#
#_____________
def write32(f,v):
    n_byte = v.to_bytes(4,byteorder='little')
    f.write(n_byte)
#
#_____________
if(len(sys.argv)!=3):
    usage();
    exit(-1);

infile=sys.argv[1];
outfile=sys.argv[2];

print("lnDsoChecksum:  checksuming %s => %s " %(infile,outfile))


print("Reading file..")
file=open(infile,"rb")
content=file.read(-1)
file.close();

binSize=len(content)

print("%d bytes read " % (binSize))

binSize-=4*4 # Skip 4 first words = MSP / Reset / size / xxhash

payload = content[16:]

lnHash=xxhash.xxh32(payload,0x100).intdigest()

print("digest %x " % (lnHash))


result=open(outfile,"wb")
result.write(content[0:8])
write32(result,binSize)
write32(result,lnHash)
result.write(payload)
result.close()
print("Done")


