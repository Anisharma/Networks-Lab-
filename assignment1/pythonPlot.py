import matplotlib.pyplot as plt
import numpy as np
from matplotlib import colors
import numpy as np
import pylab as pl
import scipy.stats as stats
#from matplotlib.ticker import PercentFormatter

np.random.seed(19680801)

n_bins = 5000

f = open("Result2.txt" , "r") 
myList = []

for line in f:
	lin = line[:-2]
	myList.append(float(lin))

#print(myList)
fit = stats.norm.pdf(myList, np.mean(myList), np.std(myList))  #this is a fitting indeed

pl.plot(myList,fit,'-o')

#pl.hist(myList,normed=True)      #use this to draw histogram of your data

pl.show()
#fig, axs = plt.subplots(1, 1)
#axs.set_ylabel("number of pings")
#axs.set_xlabel("latency of Ping")
#axs.hist(myList, bins=n_bins )
#plt.show()