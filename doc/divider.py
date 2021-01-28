


# STM32
MAINCLOCK = 128*1000000
DIVIDER = [2,4,8]
SMP = [ "1.5", "7.5","13.5","28.5","41.5","71.5","239.5"]
OVERSAMPL = [1 ]
# GD332
#MAINCLOCK = 96*1000000
#DIVIDER = [2,4,5,6,8,10,12,20]
#SMP = [ "1.5", "7.5","13.5","28.5","41.5","71.5","239.5"]
#OVERSAMPL = [1,2,4,8,16,32,64,128,256 ]



# Target clock (us)

TARGET = [ "0.025","0.05","0.1","0.2","0.5",1,2,5,10,20,50,100,200,500,1000]

def printOffer(offer):
    speed=float(offer[0])*1000.
    target=int(offer[4])
    actual=int(offer[5])
    er=int((100*(target-actual))/target)
    print("Speed = "+str(speed)+" samp/div \tOVERSAMP="+str(offer[3])+" SCALE="+str(offer[2])+" RATE="+str(offer[1]) +"    (target = "+str(target)+"  actual="+str(actual)+",error="+str(er)+"%%)")


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
    printOffer(bestOffer)
print( "All done")
    
