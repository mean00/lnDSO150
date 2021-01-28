

MAINCLOCK = 96000000
OVERSAMPL = [1 ]

DIVIDER = [2,4,8]
SMP = [ "1.5", "7.5","13.5","28.5","41.5","71.5","239.5"]

# Target clock (us)

TARGET = [ "0.025","0.05","0.1","0.2","0.5",1,2,3,50,20,50,100,200,500,1000]

adc= 1./MAINCLOCK
for t in TARGET:
    period=float(t)/1000. # in ms
    ratio=period/(24*adc) # 24 samp /dev
    bestDiff=1000000000000000000000000000000
    bestOffer=[0,0,0]
    # we want ratio just above divider*oversamp*(smp+12.5)
    for smp in SMP:
        for scale in DIVIDER:
           for oversamp in OVERSAMPL:
                candidate=(float(smp)+12.5)*scale*oversamp 
                #print("TGT="+str(ratio)+" r="+str(candidate))
                if candidate<ratio :
                       if(ratio-candidate)<bestDiff:
                            bestOffer=[t,smp,scale,oversamp,ratio,candidate]
                            bestDiff=ratio-candidate
    print(str(bestOffer))
print( "All done")
    
