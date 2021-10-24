from DSO150 import DSO150
import csv
import time
dso=DSO150()

print("Asking for a capture ")
data=dso.GetData() 
f = open('output.csv', 'w')
writer = csv.writer(f)
writer.writerow(['t','v'])
l=len(data)
for i in range(0,l):
    ns=[i, float(data[i])]
    print(ns)
    writer.writerow(ns) 
f.close()    

