from DSO150 import DSO150
import time
dso=DSO150()

print("Asking for a capture ")
data=dso.GetData() 
print("Got "+str(data))
time.sleep(2)
    

