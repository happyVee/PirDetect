#Coding = 'utf-8'

from pylab import *
import xlwt
import numpy as np
import math
import matplotlib.pyplot as plt
import os


def func():
    pass

if __name__ == '__main__':
    npath = './n/'
    ypath = './y/'
    nlist = os.listdir(npath)
    ylist = os.listdir(ypath)

    indata = np.zeros(10000)
    outdata = []

    for f in nlist:
        farr = np.loadtxt(open(npath + f,'rb'),delimiter = ',',skiprows=0)
        indata = np.vstack((indata,farr))
        outdata.append(0)

    for f in ylist:
        farr = np.loadtxt(open(ypath + f,'rb'),delimiter = ',',skiprows=0)
        indata = np.vstack((indata,farr))
        outdata.append(1)
    
    outdata = np.array(outdata)
    indata = indata[1:]
    print(len(indata))
    print(len(outdata))

    np.save('indata.npy',indata)
    np.save('outdata.npy',outdata)
    indata = []
    outdata = []

    indata = np.load('indata.npy')
    outdata = np.load('outdata.npy')

    mid = 137
    nonum = 26
    yesnum = 25
    noin = indata[0:137]
    yesin = indata[137:]
    noout = outdata[0:137]
    yesout = outdata[137:]
    print(len(noin),noin.shape)
    print(len(yesin),yesin.shape)

    np.random.seed(100)
    selarr = np.random.permutation(len(noin))
    nodevin = noin[selarr[0:nonum]]
    nodevout = noout[selarr[0:nonum]]
    notrainin = noin[selarr[nonum:]]
    notrainout = noout[selarr[nonum:]]

    np.random.seed(105)
    selarr = np.random.permutation(len(yesin))
    yesdevin = yesin[selarr[0:yesnum]]
    yesdevout = yesout[selarr[0:yesnum]]
    yestrainin = yesin[selarr[yesnum:]]
    yestrainout = yesout[selarr[yesnum:]]

    devin = np.vstack((nodevin,yesdevin))
    devout = np.append(nodevout,yesdevout)
    trainin = np.vstack((notrainin,yestrainin))
    trainout = np.append(notrainout,yestrainout)

    np.random.seed(105)
    shuffle = np.random.permutation(len(devout))
    shuffle_devin = devin[shuffle]
    shuffle_devout = devout[shuffle]
    shuffle = np.random.permutation(len(trainout))
    shuffle_trainin = trainin[shuffle]
    shuffle_trainout = trainout[shuffle]

    np.save('devin.npy',devin)
    np.save('devout.npy',devout)
    np.save('trainin.npy',trainin)
    np.save('trainout.npy',trainout)
    np.save('shuffle_devin.npy',shuffle_devin)
    np.save('shuffle_devout.npy',shuffle_devout)
    np.save('shuffle_trainin.npy',shuffle_trainin)
    np.save('shuffle_trainout.npy',shuffle_trainout)

    for k in range(devin.shape[0]):
        fmtx = devin[k]
        plotmatrix(fmtx,str(k),'./devimg/')
    
    for k in range(trainin.shape[0]):
        fmtx = trainin[k]
        plotmatrix(fmtx,str(k),'./trainimg/')