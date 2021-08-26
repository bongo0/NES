#!/usr/bin/python3
import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

channels=[]

if(len(sys.argv) > 1):
    fnames = sys.argv[1:]

for fname in fnames:
    with open(fname) as file:
        ch=[]
        reader = csv.reader(file, delimiter=" ")
        for row in reader:
            ch.append(int(row[0]))
            
        channels.append(np.array(ch))

#offset_ch=[channels[0][0]]
#last_out=int(channels[0][0])
#for i in range(1,channels[0].size):
#    output=0
#    delta = channels[0][i]-channels[0][i-1]
#    output = delta
#    offset_ch.append( output  )
#    last_out=output


for i in range(0,len(fnames)):
    p=0#29249
    plt.plot(channels[i][p:10000+p],"-o",label= fnames[i])

#plt.plot(offset_ch,"-o",label="offset ch")
plt.legend()
plt.show()